/* Copyright 2023, 2024 Aleksandr Popov
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Code related to the grid experiments.
 * @author Aleksandr Popov
 * @date 2023, 2024
 * @copyright GNU GPLv3
 */

#ifndef MAP_H
#define MAP_H

#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "defs.hpp"

/**
 * @brief Contains code related to the grid: data structures and queries for
 * the main usage.
 */
namespace map {
    /**
     * @brief Weighted edge.
     */
    struct Edge {
        /// An adjacent vertex index.
        std::size_t v;
        /// The weight of the edge.
        int weight;
    };

    /**
     * @brief Compare edges for equality ignoring the weight.
     */
    struct EdgeEqualV {
        /**
         * @brief Functor for edge equality comparison ignoring the weight.
         * @param a First edge.
         * @param b Second edge.
         * @return Whether `v` is the same.
         */
        constexpr bool operator()(Edge const& a, Edge const& b) const;
    };

    /**
     * @brief Compare edges based on their weight.
     * @tparam Cmp Comparator, `std::less` by default.
     */
    template <template <typename T> class Cmp = std::less>
    struct EdgeOrderW {
        /**
         * @brief Functor for edge comparison based on the weight.
         * @param a First edge.
         * @param b Second edge.
         * @return Whether weight of @p a is `Cmp` than weight of @p b.
         */
        constexpr bool operator()(Edge const& a, Edge const& b) const {
            return Cmp<int>()(a.weight, b.weight);
        }
    };
}

/// Specialise `std::hash` to `map::Edge` for use in `std::unordered_set`.
template<> struct std::hash<::map::Edge> {
    /**
     * @brief Hash operator.
     * @param e Hashed edge.
     * @return A valid hash value.
     */
    std::size_t operator()(::map::Edge const& e) const noexcept {
        return std::hash<std::size_t>{}(e.v);
    }
};

namespace map {
    // Forward declare, we use it in the shortcut graph.
    class Map;

    /**
     * @brief A simple directed graph class specialised as a shortcut graph.
     *
     * We find the shortest path from the start to the end of a trajectory
     * using only the valid shortcuts.
     */
    class ShortcutGraph {
    public:
        /// Graph vertex.
        using Vertex = ::map::Meas;

    private:
        /// The vertices of the graph are the vertices of a trajectory.
        std::vector<Vertex> const& vs;
        /// Directed adjacency list with edge weights.
        std::vector<std::unordered_set<Edge, std::hash<Edge>, EdgeEqualV>> adj;

    public:
        /**
         * @brief Create a (directed) shortcut graph for @p tr, where the
         * vertices are the points of @p tr and an edge exists iff it goes
         * forward on a trajectory and is covered by the data in @p map.
         * @param tr The trajectory to build the graph for.
         * @param map The map to check which edges are valid.
         */
        ShortcutGraph(std::vector<Vertex> const& tr, ::map::Map const& map);

        /**
         * @brief Add all edges from vertex index \f$i\f$ to \f$i + 1\f$ in the
         * graph, even if they are not covered by the map, assumed to be
         * bridgelets.
         */
        void add_single_hops();

        /**
         * @brief Return the shortest path from the start to the end of a
         * trajectory in the shortcut graph, if one exists.
         *
         * We find a weighted shortest path from the start vertex in this DAG
         * in topologically sorted order; the goal is to find the maximal
         * subtrajectories that cover the entire trajectory.
         *
         * If the single hops have been added, we allow the parts for which we
         * have no data to be filled in using bridgelets, but we minimise their
         * occurrence.
         * @param ret The shortest sequence of vertices *in reverse order*.
         * These are indices into `tr` that the graph was constructed with.
         * @param check_hops Internally used function to either minimise or
         * maximise the number of hops in the path (while always minimising the
         * weight of the path).
         * @return Whether a path from start to end exists; should always
         * return `true` if `add_single_hops()` has been called.
         */
        bool shortest_path(std::vector<std::size_t>& ret,
            std::function<bool(std::size_t, std::size_t)> check_hops = [](
                    std::size_t hj, std::size_t hi) noexcept {
                return hj > hi + 1;
            });
    };

    /**
     * @brief The data structure holding trained bridges, supports queries to
     * interpolate sparse trajectories.
     */
    class Map {
        /// Records which trajectories go from \f$(x_1, y_1)\f$ to
        /// \f$(x_2, y_2)\f$ in \f$t\f$ steps.
        std::unordered_map<BridgeID, std::forward_list<SubTraj>, TrajHash> map;
        /// The trajectories with their IDs.
        std::unordered_map<std::uint32_t, Traj> tr_reg;
        /// Whether to allow diagonal movement.
        bool diag = false;

    public:
        /**
         * @brief How to use the intermediate points.
         *
         * Possible values are `none`---only use start and end points,
         * `few`---use as few intermediate points as needed when looking up
         * saved data, `most`---use as many intermediate points as possible,
         * `all`---use all intermediate points, inserting bridgelets between
         * them if data is not present.
         */
        enum class Inter {
            none, few, most, all
        };

    private:
        /**
         * @brief Check if any trajectories go from @p s to @p e.
         * @param s Start location \f$(t_1, x_1, y_1)\f$.
         * @param e End location \f$(t_2, x_2, y_2)\f$.
         * @return Whether there are any recorded trajectories from
         * \f$(x_1, y_1)\f$ to \f$(x_2, y_2)\f$ in \f$t_2 - t_1\f$ steps.
         */
        bool is_present(Meas const& s, Meas const& e) const;

        /**
         * @brief Compute the bridge from @p s to @p e using training data.
         * @param s The start of the bridge.
         * @param e The end of the bridge.
         * @return The map of visit probabilities.
         */
        Probs single_query(Meas const& s, Meas const& e) const;

        /**
         * @brief Find the shortest covering path for a trajectory.
         * @param tr The trajectory to cover using training data.
         * @param use_points How to use intermediate points (`few` or `most`).
         * @return Whether the trajectory can be covered completely with known
         * data and the subsequence of indices into @p tr that represent the
         * best path.
         */
        std::pair<bool, std::vector<std::size_t>> find_path(
            Traj const& tr, Inter use_points) const;

    public:
        /**
         * @brief Enable diagonal movement.
         */
        void enable_diag();

        /**
         * @brief Retrieve the stored trajectory with a given index.
         * @param id The index of the trajectory.
         * @return The corresponding trajectory, previously passed to
         * `train()`.
         */
        Traj const& get_tr(std::uint32_t id) const;

        /**
         * @brief Record information about a single trajectory, also training
         * on all subtrajectories of length &ge; 3.
         * @param tr The training trajectory.
         * @param trid The ID of the trajectory (~filename).
         */
        void train(Traj tr, std::uint32_t trid);

        /**
         * @brief Process a training data set, performing `train()` for each
         * trajectory.
         * @param trs The training trajectories paired with their IDs.
         */
        void train_all(std::vector<std::pair<Traj, std::uint32_t>> const& trs);

        /**
         * @brief Compute the prediction for @p tr.
         *
         * Based on @p use_points, force all intermediate points, ignore them,
         * or use as many or as few as possible to use the training data as
         * completely as possible.
         * @param tr The query trajectory.
         * @param use_points Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether @p tr is covered by `Map.map` and the prediction.
         */
        std::pair<bool, Probs> query(Traj const& tr,
            Inter use_points = Inter::none) const;

        /**
         * @brief Check if the subtrajectory of @p tr from index @p s to index
         * @p e is covered.
         * @param tr The trajectory to check.
         * @param s The index of the first point of the subtrajectory.
         * @param e The index of the last point of the subtrajectory.
         * @param use_points Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether the trajectory is covered by `Map.map`.
         */
        bool covered(Traj const& tr, std::size_t s, std::size_t e,
            Inter use_points = Inter::none) const;

        /**
         * @brief Compute the error for the predicted visits @p pred compared
         * to the ground truth @p gr.
         *
         * We expect @p pred to have probability 1 at least at the start and
         * end of @p gr. As in the paper, assume the trajectory visits cells in
         * set \f$V\f$. Let \f$p_j\f$ be the visit probability for cell \f$j\f$
         * in the model.
         *
         * Compute \f$\sum_{j \in V} (1 - p_j) + \sum_{j \notin V} p_j\f$.
         * For this to work as intended, the @p gr trajectory has to be dense.
         * However, if we are using the intermediate points of the test
         * trajectory when querying, we get to see the ground truth, and so
         * there will be no room to apply the model between the neighbouring
         * cells. In this case, we need to skip some measurements from @p gr
         * when testing (outside of this function). If we do not use the
         * intermediate points, we may keep the full trajectory for testing.
         * @param pred The prediction as returned by e.g. `query()`.
         * @param gr The true trajectory.
         * @return The error.
         */
        Error pred_error(Probs const& pred, Traj const& gr) const;

        /**
         * @brief Compute the error for our prediction for @p tr compared to
         * the ground truth @p gr.
         *
         * If @p use_points is `none`, look up the start and the end of @p tr.
         * Otherwise, use the intermediate points of @p tr to look for
         * coverage, then compare piecewise with @p gr.
         *
         * We essentially combine `query()` and `pred_error()` without
         * returning the actual query result.
         * @param tr The trajectory to test.
         * @param gr The true trajectory (potentially denser than @p tr).
         * @param use_points Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether @p tr is covered by `Map.map` and the error.
         */
        std::pair<bool, Error> query_error(Traj const& tr, Traj const& gr,
            Inter use_points = Inter::none) const;

        /**
         * @brief Load the saved state of `Map.map` from an input stream.
         * @param inf The stream containing the `Map.map` output by `save()`.
         * @param trajs The relevant trajectories with their IDs.
         */
        void load(std::istream& inf,
            std::unordered_map<std::uint32_t, Traj> trajs);

        /**
         * @brief Save the state of `Map.map` to an output stream.
         * @param outf The clear output stream.
         */
        void save(std::ostream& outf) const;
    };
}
#endif
