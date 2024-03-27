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

#include "bridge.hpp"

#include <cassert>
#include "problems.hpp"

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

    Probs ignore_pr(Probs const& pred) {
        Probs res;
        for (auto const& [cell, pr]: pred)
            res[cell] = 1;
        return res;
    }
}
