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
 * @brief Bridgelets, bridges, and beads: implementations.
 * @author Aleksandr Popov
 * @date 2024
 * @copyright GNU GPLv3
 */

#include "bridge.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <tuple>
#include "problems.hpp"

namespace {
    /**
     * @brief The enumeration to pick between the x- and y-direction in 2D.
     */
    enum class Axis {
        x, y
    };

    /**
     * @brief Record an intersection of a line with a x- or y-cell boundary.
     */
    struct LineInt {
        /// The fraction parameter along the line segment.
        util::Frac t;
        /// Whether we cross the boundary in x- or y-direction.
        Axis dir;

        /**
         * @brief Compare two intersections based on which is closer to the
         * start.
         * @param o The other intersection.
         * @return True iff this intersection is closer to the start.
         */
        bool operator<(LineInt const& o) const {
            return t < o.t;
        }
    };

    /**
     * @brief Compute, in sorted order, the intersections of the line segment
     * from @p s to @p e with the cell boundaries, indicating if they are in x-
     * or in y-direction, so we know the sequence of visited cells.
     *
     * Let \f$\mathbf{d} = \mathbf{e} - \mathbf{s}\f$. Note that
     * \f$\mathbf{s}\f$ and \f$\mathbf{e}\f$ have integer coordinates. We need
     * to track intersections of cell boundaries occurring whenever we cross
     * \f$n + 0.5\f$ for integer \f$n\f$ in either x- or y-direction.
     *
     * We can write a line equation in parametric form as
     * \f$p_x = \lvert s_x\rvert + t \cdot \lvert d_x\rvert\f$, where \f$t\f$
     * ranges from \f$0\f$ to \f$1\f$, and similarly for y. We can iterate over
     * \f$s_x + 0.5 + n\f$, with \f$n\f$ ranging from \f$0\f$ to
     * \f$\lvert d_x\rvert - 1\f$, and similarly for y, to find the relevant
     * intersections. So, they occur at \f$t = (n + 0.5) / \lvert d_x\rvert\f$.
     * We can track them separately for x- and y-axis, then merge the results
     * in sorted order of \f$t\f$.
     * @param s The start cell for the line segment.
     * @param e The end cell for the line segment.
     * @return The sorted vector of intersections.
     */
    std::vector<LineInt> dirs(util::Cell const& s, util::Cell const& e) {
        auto const& [xs, ys] = s;
        auto const& [xe, ye] = e;

        assert(util::can_subtract(xe, xs));
        assert(util::can_subtract(ye, ys));
        auto const x = static_cast<util::Cnt>(util::absv(xe - xs));
        auto const y = static_cast<util::Cnt>(util::absv(ye - ys));

        std::vector<LineInt> deltax, deltay;
        for (util::Cnt n = 0; n < x; ++n)
            deltax.push_back({util::divide(2 * n + 1, 2 * x), Axis::x});
        for (util::Cnt n = 0; n < y; ++n)
            deltay.push_back({util::divide(2 * n + 1, 2 * y), Axis::y});

        std::vector<LineInt> res;
        res.reserve(deltax.size() + deltay.size());
        std::merge(deltax.begin(), deltax.end(), deltay.begin(), deltay.end(),
            std::back_inserter(res));
        return res;
    }
}

namespace util {
    Probs bridgelet(Meas const& s, Meas const& e, bool diag) {
        auto const& [t1, x1, y1] = s;
        auto const& [t2, x2, y2] = e;
        assert(t2 > t1);
        return normalise(prob::visit_all(t2 - t1, {x1, y1},
            {x2, y2}, diag).flatten(t2 - t1), {x1, y1});
    }

    Probs sequence(std::vector<Probs> const& pr_maps) {
        Probs res;
        // Probability unchanged where pr_map is 0.
        for (auto const& pr_map: pr_maps)
            for (auto const& [cell, prob]: pr_map)
                res[cell] = 1 - (1 - res[cell]) * (1 - prob);
        return res;
    }

    Probs bridge(Traj const& tr, std::size_t s, std::size_t e, bool diag) {
        assert(e > s && e < tr.size());
        std::vector<Probs> seq;
        for (auto i = s; i < e; ++i)
            seq.emplace_back(bridgelet(tr[i], tr[i + 1], diag));
        return sequence(seq);
    }

    Probs average(std::vector<Probs> const& pr_maps) {
        if (pr_maps.size() == 1)
            return pr_maps[0];

        Probs res;
        for (auto const& pr_map: pr_maps)
            for (auto const& [cell, pr]: pr_map)
                res[cell] += pr;

        auto total = pr_maps.size();
        for (auto& elem: res)
            elem.second /= total;
        return res;
    }

    Probs ignore_pr(Probs const& pred, Frac const& thr) {
        Probs res;
        for (auto const& [cell, pr]: pred)
            if (pr >= thr)
                res[cell] = 1;
        return res;
    }

    Probs straight_line(Traj const& tr) {
        Probs res;
        if (tr.size() < 2)
            return res;
        res[::map::meas_to_cell(tr.front())] = 1;
        for (std::size_t i = 0; i < tr.size() - 1; ++i) {
            Cell cur = ::map::meas_to_cell(tr[i]);
            Cell const end = ::map::meas_to_cell(tr[i + 1]);
            auto dx = sign_of(end.first - cur.first);
            auto dy = sign_of(end.second - cur.second);
            auto seq = dirs(cur, end);
            for (auto const& nxt: seq) {
                if (nxt.dir == Axis::x)
                    cur.first += dx;
                else
                    cur.second += dy;
                res[cur] = 1;
            }
        }
        return res;
    }
}
