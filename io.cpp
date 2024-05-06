/* Copyright 2023, 2024 Aleksandr Popov
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
 * @brief I/O implementation.
 * @author Aleksandr Popov
 * @date 2023, 2024
 * @copyright GNU GPLv3
 */

#include "io.hpp"

#include <cassert>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include "dp.hpp"

namespace io {
    void dp_write(dp::DP const& table, Time const& T, Cell const& shift,
            std::ostream& outf) {
        auto [is, js] = shift;
        auto sT = static_cast<Loc>(T);
        outf << T << '\n';
        for (Loc i = is - sT; i <= is + sT; ++i)
            for (Loc j = js - sT; j <= js + sT; ++j)
                outf << table.at(i, j, T) << (j < js + sT ? ' ' : '\n');
        outf.flush();
    }

    void flat_write(dp::DP const& table, Time const& T, Cell const& shift,
            std::ostream& outf) {
        auto fl_table = table.flatten(T);
        auto [is, js] = shift;
        auto sT = static_cast<Loc>(T);
        outf << T << '\n';
        for (Loc i = is - sT; i <= is + sT; ++i)
            for (Loc j = js - sT; j <= js + sT; ++j) {
                auto nonzero = fl_table.find({i, j});
                outf << (nonzero != fl_table.end() ? nonzero->second : 0)
                     << (j < js + sT ? ' ' : '\n');
            }
    }

    void traj_write(std::vector<Cell> const& traj, std::ostream& outf) {
        for (auto const& [i, j]: traj)
            outf << i << ' ' << j << '\n';
    }

    Traj read_traj(std::istream& inf) {
        for (auto i = 0u; i < 3u; ++i)
            inf.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string infline;
        std::istringstream lproc;
        Time t;
        Loc x, y;
        Traj result;
        char comma;

        while (std::getline(inf, infline)) {
            lproc.clear();
            lproc.str(std::move(infline));
            lproc >> t >> comma >> x >> comma >> y;
            result.emplace_back(t, x, y);
        }
        return result;
    }

    Traj sparsify(Traj const& dense, Time const& skip) {
        assert(dense.size() >= 2);
        Traj ret;
        ret.push_back(dense.front());
        for (auto const& m: dense)
            if (std::get<0>(m) - std::get<0>(ret.back()) >= skip)
                ret.push_back(m);
        if (ret.back() != dense.back())
            ret.push_back(dense.back());
        return ret;
    }

    std::vector<std::uint32_t> read_flist(std::istream& inf) {
        std::vector<std::uint32_t> ret;
        std::uint32_t tmp;
        while (inf >> tmp)
            ret.push_back(std::move(tmp));
        return ret;
    }
}
