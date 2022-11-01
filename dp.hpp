/* Copyright 2022 Aleksandr Popov
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

#ifndef DP_H
#define DP_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "defs.hpp"

namespace dp {
    /**
     * The description of a blocked cell.
     */
    struct Blocked {
        /// The location of the blocked cell.
        Loc i, j;
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
         * @return `true` iff (i, j) = (o.i, o.j).
         */
        bool operator==(Blocked const& o) const;
    };
}

// Specialise std::hash to dp::Blocked for use in unordered_set.
namespace std {
    template<> struct hash<dp::Blocked> {
        std::size_t operator()(dp::Blocked const& t) const noexcept {
            return dp::hash_helper(t.i, t.j);
        }
    };
}

namespace dp {
    /**
     * The dynamic program for computing paths with blocked cells, including
     * access functions and simple operations: shifting, flipping time,
     * combining with another DP.
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
        std::pair<Loc, Loc> shift{0, 0};

        /**
         * @brief Test if an index is within bounds, with shift and time flip.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return True iff -T <= i, j <= T and 0 <= t <= T, after shift, and
         * the cell is not blocked at this time.
         */
        bool test_index(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Compute the index within the table, with shift and time flip.
         * @param i First dimension.
         * @param j Second dimension.
         * @param t Current time.
         * @return The index in table that maps to (i, j, t).
         */
        std::size_t index(Loc const& i, Loc const& j, Time const& t) const;

    public:
        /**
         * @brief Compute the number of paths in W_{x, y, t} for all possible
         * (x, y) and all t <= T, starting in (0, 0).
         * @param max_time The value of T (allowed number of steps).
         * @param propagate The propagation function, see e.g. uniform_prop.
         * @param blocked_cells The set of blocked cells.
         */
        DP(Time max_time,
            std::function<Cnt(DP const&, Loc const&, Loc const&, Time const&)>
            propagate, std::pair<Loc, Loc> origin = {0, 0},
            std::unordered_set<Blocked> const& blocked_cells = {});

        /**
         * @brief Return the value P(i, j, t) in the DP, with 0 for unreachable
         * cells.
         * @param i First dimension, with non-zero values possible from -T to T.
         * @param j Second dimension.
         * @param t The time, between 0 and T.
         * @return The number of paths in W_{i, j, t}.
         */
        Cnt at(Loc const& i, Loc const& j, Time const& t) const;

        /**
         * @brief Return the value P(i, j, t) in the DP. Throw an exception for
         * out-of-bounds values, so not in [-T, T] x [-T, T] x [0, T].
         * @param i First dimension, -T to T.
         * @param j Second dimension, -T to T.
         * @param t The time, 0 to T.
         * @return The number of paths in W_{i, j, t}.
         */
        Cnt& at(Loc const& i, Loc const& j, Time const& t);

        /**
         * @brief Flip the time, so the paths start at T and end at 0.
         */
        void flip_time();

        /**
         * @brief Flip the coordinates, so a query for (x, y, t) returns the
         * result in (-x, -y, t) (after accounting for the shift).
         */
        void flip_coords();

        /**
         * @brief Shift the origin from (0, 0) or other current one to `origin`.
         * @param origin The new origin.
         */
        void set_shift(std::pair<Loc, Loc> origin);

        /**
         * @brief Combine two DPs by multiplying matching entries.
         * @param other The second DP, with the same `T` and opposite `flip`.
         * @return The multiplied DP, with the new shift, and no flip.
         */
        DP operator*(DP const& other) const;

        /**
         * @brief Flatten a DP to sum up the values at the same time stamp.
         * @param max_time Only sum up from t = 0 to max_time; if max_time >= T,
         * some up over the entire DP.
         * @return A mapping from points (i, j) to the sum from DP over all t.
         */
        std::unordered_map<std::pair<Loc, Loc>, Cnt, LocHash> flatten(Time
            const& max_time) const;
    };

    /**
     * @brief Uniform propagation for the DP: one path in each neighbouring
     * direction, one path for staying in the same spot.
     * @param r The instance of the DP from which we propagate.
     * @param i First dimension.
     * @param j Second dimension.
     * @param t The time from which we propagate to t + 1.
     */
    Cnt uniform_prop(DP const& r, Loc const& i, Loc const& j, Time const& t);
}
#endif
