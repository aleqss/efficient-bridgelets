/* Copyright 2022, 2023, 2024 Aleksandr Popov
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
 * @brief Dynamic program for computing visit counts.
 * @author Aleksandr Popov
 * @date 2022--2024
 * @copyright GNU GPLv3
 */

#ifndef DP_H
#define DP_H

#include <cstddef>
#include <functional>
#include <unordered_set>
#include <vector>
#include "defs.hpp"

/**
 * @brief Everything related to the dynamic program for visit counts.
 */
namespace dp {
    /**
     * @brief The description of a blocked cell.
     */
    struct Blocked {
        /// The location of the blocked cell: x-coordinate.
        Loc i;
        /// The location of the blocked cell: y-coordinate.
        Loc j;
        /// The step from which the cell is blocked.
        Time start;

        /**
         * @brief Initialise a blocked cell.
         * @param x First dimension.
         * @param y Second dimension.
         * @param s Starting from this time, the cell is blocked.
         */
        Blocked(Loc x, Loc y, Time s);

        /**
         * @brief Compare the encoded locations of two instances (time is not
         * taken into account).
         * @param o The other instance.
         * @return `true` iff \f$(i, j) = (o.i, o.j)\f$.
         */
        bool operator==(Blocked const& o) const;
    };
}

/// Specialise `std::hash` to `dp::Blocked` for use in `std::unordered_set`.
template<> struct std::hash<::dp::Blocked> {
    /**
     * @brief Hash operator.
     * @param t Blocked cell.
     * @return A valid hash value.
     */
    size_t operator()(::dp::Blocked const& t) const noexcept {
        return ::dp::hash_helper(t.i, t.j);
    }
};

namespace dp {
    /**
     * @brief The dynamic program for computing paths with blocked cells.
     *
     * Includes access functions and simple operations: shifting, flipping
     * time, combining with another DP.
     */
    class DP {
        /// The maximum number of steps from (0, 0).
        Time const T;
        /// The dynamic program.
        std::vector<Cnt> table;
        /// The set of blocked cells, with times at which they get blocked.
        std::unordered_set<Blocked> blocked;
        /// Whether we have flipped time.
        bool flip{false};
        /// The factor in computing locations, -1 or 1, to flip directions.
        Loc f{1};
        /// The shift, i.e. starting position instead of (0, 0).
        Cell shift{0, 0};
        /// Whether the DP is stored in dense or sparse form.
        bool dense{false};

        /**
         * @brief Test if an index is within bounds, with shift and time flip,
         * handling both sparse and dense storage.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return True iff index is explicitly stored in the table, and the
         * cell is not blocked at this time.
         */
        bool test_index(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Test if an index is within bounds, with shift and time flip,
         * for sparse storage.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return True iff \f$t \le T\f$, \f$|i| + |j| \le t\f$, after shift,
         * and the cell is not blocked at this time.
         */
        bool test_index_sp(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Test if an index is within bounds, with shift and time flip,
         * for dense storage.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return True iff \f$t \le T\f$, \f$-T \le i, j \le T\f$, after
         * shift, and the cell is not blocked at this time.
         */
        bool test_index_dn(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Compute the index within the table, with shift and time flip,
         * handling both sparse and dense storage.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return The index in `DP.table` that maps to \f$(i, j, t)\f$.
         */
        std::size_t index(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Compute the index within the table, with shift and time flip,
         * for sparse storage.
         *
         * We only store entries that may be non-zero. In each time slice, we
         * only store the non-empty diamond, depending on \f$t\f$.
         * In layer \f$t\f$, there are \f$1 + 2t(t + 1)\f$ entries. The index
         * of the layer \f$t\f$ can be computed as the sum of the element
         * counts in the previous layers, yielding
         * \f$t^2 + (t - 1)t(2t - 1)/3\f$.
         *
         * To find the index within a layer, let \f$x = t - |i|\f$.
         * - For \f$i \le 0\f$, index of row \f$i\f$ is \f$x^2\f$; centre of
         * row \f$i\f$ is \f$x(x + 1)\f$.
         * - For \f$i > 0\f$, the last index in the layer is \f$2t(t + 1)\f$;
         * count backwards from it to get centre of row \f$i\f$ at
         * \f$2t(t + 1) - x(x + 1)\f$.
         *
         * Within the row, shift by \f$j\f$ to get the final index.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return The index in `DP.table` that maps to \f$(i, j, t)\f$.
         */
        std::size_t index_sp(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Compute the index within the table, with shift and time flip,
         * for dense storage.
         *
         * We store the entire hyperrectangle with dimensions
         * \f$(T + 1) * (2T + 1)^2\f$. The indexing is standard:
         * \f$t * (2T + 1)^2 + i * (2T + 1) + j\f$.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return The index in `DP.table` that maps to \f$(i, j, t)\f$.
         */
        std::size_t index_dn(Loc const& i, Loc const& j, Time const& t) const;

    public:
        /**
         * @brief Compute the number of paths in \f$W_{x, y, t}\f$ for all
         * possible \f$(x, y)\f$ and all \f$t \le T\f$, starting in @p origin,
         * \f$(0, 0)\f$ by default.
         * @param max_time The value of \f$T\f$ (allowed number of steps).
         * @param propagate The propagation function, see e.g.
         * `uniform_prop()`.
         * @param origin The starting location at time 0.
         * @param blocked_cells The set of blocked cells.
         * @param dense_st Whether to use the dense storage representation.
         */
        DP(Time max_time,
            std::function<Cnt(DP const&, Loc const&, Loc const&, Time const&)>
            propagate, Cell origin = {0, 0},
            std::unordered_set<Blocked> const& blocked_cells = {},
            bool dense_st = false);

        /**
         * @brief Return the value \f$P(i, j, t)\f$ in the DP, with 0 for
         * unreachable cells.
         * @param i First dimension, non-zero values possible from \f$-T\f$ to
         * \f$T\f$.
         * @param j Second dimension.
         * @param t The time, between \f$0\f$ and \f$T\f$.
         * @return The number of paths in \f$W_{i, j, t}\f$.
         */
        Cnt at(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Return the value \f$P(i, j, t)\f$ in the DP. Throw an
         * exception for out-of-bounds values that are not stored explicitly.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t The time, \f$0\f$ to \f$T\f$.
         * @return The number of paths in \f$W_{i, j, t}\f$.
         */
        Cnt& at(Loc const& i, Loc const& j, Time const& t);

        /**
         * @brief Flip the time, so the paths start at \f$T\f$ and end at
         * \f$0\f$.
         */
        void flip_time();

        /**
         * @brief Flip the coordinates, so a query for \f$(x, y, t)\f$ returns
         * the result in \f$(-x, -y, t)\f$ (after accounting for the shift).
         */
        void flip_coords();

        /**
         * @brief Shift the origin from \f$(0, 0)\f$ or other current one to
         * @p origin.
         * @param origin The new origin.
         */
        void set_shift(Cell origin);

        /**
         * @brief Combine two DPs by multiplying matching entries.
         * @param other The second DP, with the same `T` and opposite `flip`.
         * @return The multiplied DP, with the new shift, and no flip. It is
         * dense if and only if this DP is dense.
         */
        DP operator*(DP const& other) const;

        /**
         * @brief Flatten a DP to sum up the values at the same time stamp.
         * @param max_time Only sum up from t = 0 to @p max_time; if
         * @p max_time &ge; `T`, sum up over the entire DP.
         * @return A mapping from points (i, j) to the sum from DP over all t.
         */
        Visits flatten(Time const& max_time) const;
    };

    /**
     * @brief Uniform propagation for the DP: one path in each neighbouring
     * direction, one path for staying in the same spot.
     * @param r The instance of the DP from which we propagate.
     * @param i First dimension.
     * @param j Second dimension.
     * @param t The time from which we propagate to @p t + 1.
     * @return The count in (@p i, @p j) at @p t + 1.
     */
    Cnt uniform_prop(DP const& r, Loc const& i, Loc const& j, Time const& t);

    /**
     * @brief Uniform propagation for the DP allowing diagonal movement: one
     * path in each of eight neighbouring directions, one path for staying in
     * the same spot.
     * @param r The instance of the DP from which we propagate.
     * @param i First dimension.
     * @param j Second dimension.
     * @param t The time from which we propagate to @p t + 1.
     * @return The count in (@p i, @p j) at @p t + 1.
     */
    Cnt uniform_diag_prop(DP const& r, Loc const& i, Loc const& j,
        Time const& t);

    /**
     * @brief Uniform propagation for the DP with a preference to stay in the
     * current cell: one path in each neighbouring direction, two paths for
     * staying in the same spot.
     * @param r The instance of the DP from which we propagate.
     * @param i First dimension.
     * @param j Second dimension.
     * @param t The time from which we propagate to @p t + 1.
     * @return The count in (@p i, @p j) at @p t + 1.
     */
    Cnt staying_prop(DP const& r, Loc const& i, Loc const& j, Time const& t);

    /**
     * @brief Uniform propagation for the DP allowing diagonal movement, but
     * with a preference to stay in the current cell: one path in each of eight
     * neighbouring directions, two paths for staying in the same spot.
     * @param r The instance of the DP from which we propagate.
     * @param i First dimension.
     * @param j Second dimension.
     * @param t The time from which we propagate to @p t + 1.
     * @return The count in (@p i, @p j) at @p t + 1.
     */
    Cnt staying_diag_prop(DP const& r, Loc const& i, Loc const& j,
        Time const& t);
}
#endif
