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

#ifndef PROBLEMS_H
#define PROBLEMS_H

#include <unordered_set>
#include <utility>
#include <vector>
#include "dp.hpp"

namespace prob {
    /**
     * @brief For all possible coordinates (x, y) and for all time steps
     * 0 <= t <= T, count the paths from start to (x, y) in t steps.
     * @param T The maximum number of steps / time limit.
     * @param start The origin, from which we start the paths.
     * @param blocked The set of blocked cells, none by default.
     * @return An instance of `DP` with the counts, accessible with at(x, y, t).
     */
    dp::DP all_paths(dp::Time T, std::pair<dp::Loc, dp::Loc> start,
        std::unordered_set<dp::Blocked> const& blocked = {});

    /**
     * @brief For all possible coordinates (a, b) and for all time steps
     * 0 <= t <= T, count the paths from start to (x, y) in t steps.
     * 
     * If needed, call `flatten` on the returned DP to obtain the visit counts
     * for (x, y), accessed with `[{x, y}]`.
     * @param T The maximum number of steps / time limit.
     * @param start The origin, from which we start the paths.
     * @param end The final point of the paths.
     * @return An instance of `DP` with the counts, accessible with at(x, y, t).
     */
    dp::DP visit_all(dp::Time T, std::pair<dp::Loc, dp::Loc> start,
        std::pair<dp::Loc, dp::Loc> end);

    /**
     * @brief Generate a path from `start` to `end` according to the
     * probabilities inferred from `paths` in `T` steps.
     * 
     * Generate a random trajectory of exactly length `T` from `start` to `end`,
     * if it is possible. The `paths` DP should be the output of `all_paths`
     * with the same or larger `T` and the same `start`. We assume that the path
     * can only move to neighbouring nodes in one time step.
     * @param T The number of time steps in the trajectory.
     * @param paths The DP for computing all paths from `start`.
     * @param start The starting point of the paths.
     * @param end The endpoint of the generated trajectories.
     * @return A generated trajectory according to path counts in `paths`, so
     * the kth item is the (i, j)-coordinate at time k; or an empty trajectory
     * if the path is impossible.
     */
    std::vector<std::pair<dp::Loc, dp::Loc>> generate_path(dp::Time const& T,
        dp::DP const& paths, std::pair<dp::Loc, dp::Loc> const& end);
}
#endif
