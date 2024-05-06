/* Copyright 2022, 2024 Aleksandr Popov
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
 * @brief Explicit computation of visit counts (inefficient).
 * @author Aleksandr Popov
 * @date 2022, 2024
 * @copyright GNU GPLv3
 */

#ifndef EXPLICIT_H
#define EXPLICIT_H

#include <unordered_map>
#include "defs.hpp"

/**
 * @brief Explicit computation of visit counts.
 */
namespace xpl {
    /// Map from locations to counts.
    using Table = std::unordered_map<Cell, Cnt, LocHash>;

    /**
     * @brief For all possible coordinates \f$(x, y)\f$, count the paths from
     * @p shift to \f$(x, y)\f$ in @p T steps.
     *
     * Unlike `dp::DP`, no information about intermediate time steps is
     * available.
     * @warning This runs in \f$O(5^T)\f$ time, use `dp::DP` instead.
     * @param T The maximum number of steps / time limit.
     * @param shift The origin, from which we start the paths.
     * @return An instance of `::Table` with the counts, each associated with a
     * location \f$(x, y)\f$.
     */
    Table compute_paths(Time const& T, Cell const& shift);

    /**
     * @brief For all possible coordinates \f$(x, y)\f$, count the paths from
     * @p shift to @p end in @p T steps that visit \f$(x, y)\f$.
     * @warning This runs in \f$O(5^T)\f$ time, use `dp::DP` instead.
     * @param T The maximum number of steps / time limit.
     * @param shift The origin, from which we start the paths.
     * @param end The path destination.
     * @return An instance of `::Table` with the counts, each associated with a
     * location \f$(x, y)\f$.
     */
    Table visits(Time const& T, Cell const& shift, Cell const& end);
}
#endif
