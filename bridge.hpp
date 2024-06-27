/* Copyright 2024 Aleksandr Popov
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
 * @brief Bridgelets, bridges, and beads.
 * @author Aleksandr Popov
 * @date 2024
 * @copyright GNU GPLv3
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <cstddef>
#include <vector>
#include "defs.hpp"
#include "problems.hpp"

/**
 * @brief Utilities, centred on bridge computations.
 */
namespace util {
    using ::map::Meas, ::map::Traj;

    /**
     * @brief Compute the visit probabilities of a bridgelet from @p s to @p e.
     * @param s The starting point \f$(t_1, x_1, y_1)\f$ of a bridgelet.
     * @param e The endpoint \f$(t_2, x_2, y_2)\f$ of a bridgelet.
     * @param prop How to propagate (uniformly or not, with(out) diagonals).
     * @return The visit probabilities for the bridgelet from \f$(x_1, y_1)\f$
     * to \f$(x_2, y_2)\f$ in time \f$t_2 - t_1\f$.
     */
    Probs bridgelet(Meas const& s, Meas const& e, prob::Prop const& prop);

    /**
     * @brief Combine a sequence of bridgelets or partial bridges.
     *
     * We can use the formula as in the paper:
     * \f$P(v \mid W_1\cdots W_n) = 1 - \prod_{i = 1}^n (1 - P(v \mid W_i))\f$.
     * To compute this incrementally, we can observe that
     * \f[P(v \mid W_1 \cdots W_i) = 1 - (1 - P(v \mid W_1 \cdots W_{i - 1}))
     * \cdot (1 - P(v \mid W_i)).\f]
     * Also note that if \f$P(v \mid W_i) = 0\f$, the result stays unchanged.
     * Finally, we can start with \f$W_0\f$ that is zero-initialised to always
     * use the incremental formula.
     * @param pr_maps The sequence of visit probability maps.
     * @return The combined visit probability map.
     */
    Probs sequence(std::vector<Probs> const& pr_maps);

    /**
     * @brief Compute the bridge for a subtrajectory.
     * @param tr The trajectory.
     * @param s The start index for the bridge in the trajectory.
     * @param e The final index for the bridge in the trajectory.
     * @param prop How to propagate (uniformly or not, with(out) diagonals).
     * @return The visit probability map for the bridge.
     */
    Probs bridge(Traj const& tr, std::size_t s, std::size_t e,
        prob::Prop const& prop);

    /**
     * @brief Average several visit probability maps.
     *
     * This is only meaningful if they correspond to several trajectories with
     * the same start and end points and the same length.
     * @param pr_maps The collection of visit probability maps.
     * @return The averaged visit probability map.
     */
    Probs average(std::vector<Probs> const& pr_maps);

    /**
     * @brief Make a simple bead from our probability map by setting all
     * visit probabilities to 1 that are above the threshold.
     * @param pred The computed visit probability map.
     * @param thr The threshold, above which the probabilities are set to 1,
     * and below which they are set to 0.
     * @return The bead.
     */
    Probs ignore_pr(Probs const& pred, Frac const& thr = 0);

    /**
     * @brief Make a simple bead from our probability map by only keeping the
     * visit probabilities that are above the threshold.
     * @param pred The computed visit probability map.
     * @param thr The threshold, above which the probabilities are kept, and
     * below which they are set to 0.
     * @return The bead.
     */
    Probs clamp_pr(Probs const& pred, Frac const& thr = 0);

    /**
     * @brief Make a simple straight-line bead following the trajectory.
     * @param tr The trajectory with possible gaps.
     * @return The interpolated trajectory, with probability 1 in each relevant
     * cell.
     */
    Probs straight_line(Traj const& tr);
}
#endif
