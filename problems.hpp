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
 * @brief Compute visit counts and generate paths (use the DP).
 * @author Aleksandr Popov
 * @date 2022, 2024
 * @copyright GNU GPLv3
 */

#ifndef PROBLEMS_H
#define PROBLEMS_H

#include <functional>
#include <unordered_set>
#include <vector>
#include "defs.hpp"
#include "dp.hpp"

/**
 * @brief Problems: how we use the DP.
 */
namespace prob {
    /**
     * @brief Encapsulate the choice of propagation function and dense / sparse
     * storage.
     */
    struct Prop {
        /// Whether diagonal movement should be allowed.
        bool diag = false;
        /// Whether we prefer staying in current location over uniform.
        bool stay = false;

        /**
         * @brief Return the propagation function based on `diag` and `stay`.
         * @return One of the propagation functions defined in `::dp`.
         */
        std::function<Cnt(dp::DP const&, Loc const&, Loc const&,
            Time const&)> propagate() const;

        /**
         * @brief Whether we need to use dense storage instead of the more
         * memory-efficient sparse storage.
         * @return True when we allow diagonal movement.
         */
        bool dense() const;
    };

    /**
     * @brief For all possible coordinates \f$(x, y)\f$ and for all time steps
     * \f$0 \le t \le T\f$, count the paths from @p start to \f$(x, y)\f$ in
     * \f$t\f$ steps.
     * @param T The maximum number of steps / time limit.
     * @param start The origin, from which we start the paths.
     * @param prop How to propagate.
     * @param blocked The set of blocked cells, none by default.
     * @return An instance of `dp::DP` with the counts, accessible with
     * `at(x, y, t)`.
     */
    dp::DP all_paths(Time T, Cell start, Prop const& prop,
        std::unordered_set<dp::Blocked> const& blocked = {});

    /**
     * @brief For all possible coordinates \f$(x, y)\f$ and for all time steps
     * \f$0 \le t \le T\f$, count the paths from @p start to @p end through
     * \f$(x, y)\f$ in \f$t\f$ steps.
     *
     * If needed, call `dp::DP::flatten()` on the returned DP to obtain the
     * visit counts for \f$(x, y)\f$, accessed with `[{x, y}]`.
     * @param T The maximum number of steps / time limit.
     * @param start The origin, from which we start the paths.
     * @param end The final point of the paths.
     * @param prop How to propagate.
     * @return An instance of `dp::DP` with the counts, accessible with
     * `at(x, y, t)`.
     */
    dp::DP visit_all(Time T, Cell start, Cell end, Prop const& prop);

    /**
     * @brief Generate a path from the origin of the DP to @p end according to
     * the probabilities inferred from @p paths in @p T steps.
     *
     * Generate a random trajectory of exactly length @p T from origin to
     * @p end, if it is possible. The @p paths DP should be the output of
     * `all_paths()` with the same or larger @p T.
     * @remark We assume that the path can only move to neighbouring nodes in
     * one time step, so this does not yet work when diagonal movement is
     * allowed.
     * @param T The number of time steps in the trajectory.
     * @param paths The DP for computing all paths.
     * @param end The endpoint of the generated trajectories.
     * @return A generated trajectory according to path counts in @p paths, so
     * the \f$k\f$-th item is the \f$(i, j)\f$-coordinate at time \f$k\f$; or
     * an empty trajectory if the path is impossible.
     */
    std::vector<Cell> generate_path(Time const& T, dp::DP const& paths,
        Cell const& end);
}
#endif
