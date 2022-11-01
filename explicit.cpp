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

#include "explicit.hpp"

namespace {
    using ::xpl::Loc, ::xpl::PList, ::xpl::Cnt, ::xpl::Time;
    std::pair<Loc, Loc> decode(Cnt move, PList& res) {
        res.clear();
        Loc x = 0, y = 0;
        res.emplace(x, y);
        auto current = std::move(move);
        while (current > 0) {
            auto dir = static_cast<Cnt>(current % 5).get_ui();
            switch (dir) {
                case 0: break;
                case 1: ++x; break;
                case 2: ++y; break;
                case 3: --x; break;
                case 4: --y; break;
                default: break;
            }
            current /= 5;
            res.emplace(x, y);
        }
        return {x, y};
    }

    Cnt max_num(Time const& T) {
        Cnt res;
        mpz_ui_pow_ui(res.get_mpz_t(), 5, T);
        res--;
        return res;
    }
}

namespace xpl {
    Table compute_paths(Time const& T, std::pair<Loc, Loc> const& shift) {
        std::unordered_map<std::pair<Loc, Loc>, Cnt, dp::LocHash> table;
        auto max_cnt = max_num(T);
        auto [is, js] = shift;
        PList visited;
        for (Cnt cntr = 0; cntr <= max_cnt; ++cntr) {
            auto [i, j] = decode(cntr, visited);
            table[{i + is, j + js}]++;
        }
        return table;
    }

    Table visits(Time const& T, std::pair<Loc, Loc> const& shift,
            std::pair<Loc, Loc> const& end) {
        Table table;
        auto max_cnt = max_num(T);
        auto [is, js] = shift;
        PList visited;
        for (Cnt cntr = 0; cntr <= max_cnt; ++cntr) {
            auto [i, j] = decode(cntr, visited);
            if (end.first == is + i && end.second == js + j)
                for (auto const& [x, y]: visited)
                    table[{is + x, js + y}]++;
        }
        return table;
    }
}
