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

namespace map {
    /// Weighted edge.
    struct Edge {
        /// An adjacent vertex index.
        std::size_t v;
        /// The weight of the edge.
        int weight;
    };

    /// Compare edges for equality ignoring the weight.
    struct EdgeEqualV {
        constexpr bool operator()(Edge const& a, Edge const& b) const;
    };

    /// Compare edges based on their weight.
    template <template <typename T> class Cmp = std::less>
    struct EdgeOrderW {
        constexpr bool operator()(Edge const& a, Edge const& b) const {
            return Cmp<int>()(a.weight, b.weight);
        }
    };
}

// Specialise std::hash to map::Edge for use in unordered_set.
template<> struct std::hash<::map::Edge> {
    std::size_t operator()(::map::Edge const& e) const noexcept {
        return std::hash<std::size_t>{}(e.v);
    }
};

namespace map {
    // Forward declare, we use it in the shortcut graph.
    class Map;

    /**
     * A simple directed graph class specialised as a shortcut graph, to find
     * the shortest path from the start to the end of a trajectory using only
     * the valid shortcuts.
     */
    class ShortcutGraph {
    public:
        using Vertex = ::map::Meas;

    private:
        /// The vertices of the graph are the vertices of a trajectory.
        std::vector<Vertex> const& vs;
        /// Directed adjacency list with edge weights.
        std::vector<std::unordered_set<Edge, std::hash<Edge>, EdgeEqualV>> adj;
        /// Whether we have added single hops.
        bool added_hops = false;

        /**
         * @brief Return the unweighted shortest path from the start to the end
         * of a trajectory in the shortcut graph, if one exists.
         *
         * This function uses BFS from the start vertex; the goal is to find
         * the maximal subtrajectories that cover the entire trajectory. If the
         * hops were added that are only covered with bridgelets, you should
         * use the weighted version instead.
         * @param ret The shortest sequence of vertices *in reverse order*.
         * These are indices into `tr` that the graph was constructed with.
         * @return Whether a path from start to end exists; should always
         * return `true` if `add_single_hops()` has been called.
         */
        bool sp_unweighted(std::vector<std::size_t>& ret);

        /**
         * @brief Return the weighted shortest path from the start to the end
         * of a trajectory in the shortcut graph, if one exists.
         *
         * This function uses Dijkstra's algorithm from the start vertex; the
         * goal is to find the minimal-weight path with fewest hops. If the
         * single hops have been added, we allow the parts for which we have no
         * data to be filled in using bridgelets, they contribute to weight.
         * @param ret The shortest sequence of vertices *in reverse order*.
         * These are indices into `tr` that the graph was constructed with.
         * @return Whether a path from start to end exists; should always
         * return `true` if `add_single_hops()` has been called.
         */
        bool sp_weighted(std::vector<std::size_t>& ret);

    public:
        /**
         * @brief Create a (directed) shortcut graph for `tr`, where the
         * vertices are the points of `tr` and an edge exists iff it goes
         * forward on a trajectory and is covered by the data in `map`.
         * @param tr The trajectory to build the graph for.
         * @param map The map to check which edges are valid.
         */
        ShortcutGraph(std::vector<Vertex> const& tr, ::map::Map const& map);

        /**
         * @brief Add all edges from vertex index `i` to `i + 1` in the graph,
         * even if they are not covered by the map, assumed to be bridgelets.
         */
        void add_single_hops();

        /**
         * @brief Return the shortest path from the start to the end of a
         * trajectory in the shortcut graph, if one exists.
         *
         * This function uses either BFS or Dijkstra's algorithm from the start
         * vertex; the goal is to find the maximal subtrajectories that cover
         * the entire trajectory. If the single hops have been added, we allow
         * the parts for which we have no data to be filled in using bridgelets
         * and minimise their occurrence.
         * @param ret The shortest sequence of vertices *in reverse order*.
         * These are indices into `tr` that the graph was constructed with.
         * @return Whether a path from start to end exists; should always
         * return `true` if `add_single_hops()` has been called.
         */
        bool shortest_path(std::vector<std::size_t>& ret);
    };

    class Map {
        /// Records which trajectories go from (x1, y1) to (x2, y2) in t steps.
        std::unordered_map<BridgeID, std::forward_list<SubTraj>, TrajHash> map;
        /// The trajectories with their IDs.
        std::unordered_map<std::uint32_t, Traj> tr_reg;
        /// Whether to allow diagonal movement.
        bool diag = false;

    public:
        enum class Inter {
            none, few, most, all
        };

    private:
        /**
         * @brief Check if any trajectories go from `s` to `e`.
         * @param s Start location (t1, x1, y1).
         * @param e End location (t2, x2, y2).
         * @return Whether there are any recorded trajectories from (x1, y1) to
         * (x2, y2) in t2 - t1 steps.
         */
        bool is_present(Meas const& s, Meas const& e) const;

        /**
         * @brief Compute the bridge from `s` to `e` using training data.
         * @param s The start of the bridge.
         * @param e The end of the bridge.
         * @return The map of visit probabilities.
         */
        Probs single_query(Meas const& s, Meas const& e) const;

        /**
         * @brief Find the shortest covering path for a trajectory.
         * @param tr The trajectory to cover using training data.
         * @return Whether the trajectory can be covered completely with known
         * data and the subsequence of indices into `tr` that represent the
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
         * @return The corresponding trajectory, previously passed to `train`.
         */
        Traj const& get_tr(std::uint32_t id) const;

        /**
         * @brief Record information about a single trajectory, also training
         * on all subtrajectories of length >= 3.
         * @param tr The training trajectory.
         * @param trid The ID of the trajectory (~filename).
         */
        void train(Traj tr, std::uint32_t trid);

        /**
         * @brief Process a training data set, performing `train` for each
         * trajectory.
         * @param trs The training trajectories paired with their IDs.
         */
        void train_all(std::vector<std::pair<Traj, std::uint32_t>> const& trs);

        /**
         * @brief Compute the prediction for `tr`.
         *
         * If `intermediate` is false, look up the start and the end of `tr`,
         * Otherwise, use the intermediate points of `tr` to look for coverage.
         * @param tr The query trajectory.
         * @param intermediate Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether `tr` is covered by `map` and the prediction.
         */
        // std::pair<bool, Probs> query(Traj const& tr,
        //     bool intermediate = false) const;
        std::pair<bool, Probs> query(Traj const& tr,
            Inter use_points = Inter::none) const;

        /**
         * @brief Check if the subtrajectory of `tr` from index `s` to index
         * `e` is covered.
         * @param tr The trajectory to check.
         * @param s The index of the first point of the subtrajectory.
         * @param e The index of the last point of the subtrajectory.
         * @param intermediate Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether the trajectory is covered by `map`.
         */
        // bool covered(Traj const& tr, std::size_t s, std::size_t e,
        //     bool intermediate = false) const;
        bool covered(Traj const& tr, std::size_t s, std::size_t e,
            Inter use_points = Inter::none) const;

        /**
         * @brief Compute the error for the predicted visits `pred` compared to
         * the ground truth `gr`.
         *
         * We expect pred to have probability 1 at least at the start and end
         * of `gr`. As in the paper, assume the trajectory visits cells in set
         * \f$V\f$. Let \f$p_j\f$ be the visit probability for cell \f$j\f$ in
         * the model.
         * Compute \f$\sum_{j \in V} (1 - p_j) + \sum_{j \notin V} p_j\f$.
         * For this to work as intended, the `gr` trajectory needs to be dense.
         * However, if we are using the intermediate points of the test
         * trajectory when querying, we get to see the ground truth, and so
         * there will be no room to apply the model between the neighbouring
         * cells. In this case, we need to skip some measurements from `gr`
         * when testing (outside of this function). If we do not use the
         * intermediate points, we may keep the full trajectory for testing.
         * @param pred The prediction as returned by e.g. `query`.
         * @param gr The true trajectory.
         * @return The error (see `hop_error` for more info).
         */
        double pred_error(Probs const& pred, Traj const& gr) const;

        /**
         * @brief Compute the error for our prediction for `tr` compared to the
         * ground truth `gr`.
         *
         * If `intermediate` is false, look up the start and the end of `tr`,
         * Otherwise, use the intermediate points of `tr` to look for coverage,
         * then compare piecewise with `gr`. We essentially combine `query` and
         * `error` without returning the actual query result.
         * @param tr The trajectory to test.
         * @param gr The true trajectory (potentially denser than `tr`).
         * @param intermediate Whether to use the intermediate points on the
         * subtrajectory to try to cover the trajectory in pieces.
         * @return Whether `tr` is covered by `map` and the error.
         */
        std::pair<bool, double> query_error(Traj const& tr, Traj const& gr,
            Inter use_points = Inter::none) const;
            // bool intermediate = false) const;

        /**
         * @brief Load the saved state of `map` from an input stream.
         * @param inf The stream containing the `map` as output by `save`.
         * @param trajs The relevant trajectories with their IDs.
         */
        void load(std::istream& inf,
            std::unordered_map<std::uint32_t, Traj> trajs);

        /**
         * @brief Save the state of `map` to an output stream.
         * @param outf The clear output stream.
         */
        void save(std::ostream& outf) const;
    };
}
#endif
