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

#ifndef EXPLICIT_H
#define EXPLICIT_H

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "defs.hpp"

namespace xpl {
    using ::dp::Cnt, ::dp::Loc, ::dp::Time, ::dp::LocHash;
    using Table = std::unordered_map<std::pair<Loc, Loc>, Cnt, LocHash>;
    using PList = std::unordered_set<std::pair<Loc, Loc>, LocHash>;

    /**
     * @brief For all possible coordinates (x, y), count the paths from shift to
     * (x, y) in T steps.
     *
     * Unlike `DP`, no information about intermediate time steps is available.
     * Note: this runs in O(5^T) time, use the DP instead.
     * @param T The maximum number of steps / time limit.
     * @param shift The origin, from which we start the paths.
     * @return An instance of `Table` with the counts, each associated with a
     * location (x, y).
     */
    Table compute_paths(Time const& T, std::pair<Loc, Loc> const& shift);

    /**
     * @brief For all possible coordinates (x, y), count the paths from shift to
     * end in T steps that visit (x, y).
     *
     * Note: this runs in O(5^T) time, use the DP instead.
     * @param T The maximum number of steps / time limit.
     * @param shift The origin, from which we start the paths.
     * @param end The path destination.
     * @return An instance of `Table` with the counts, each associated with a
     * location (x, y).
     */
    Table visits(Time const& T, std::pair<Loc, Loc> const& shift,
        std::pair<Loc, Loc> const& end);
}
#endif
