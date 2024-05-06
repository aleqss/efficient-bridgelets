/* Copyright 2022, 2023, 2024 Aleksandr Popov
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
 * @brief Basic definitions and data types implemented.
 * @author Aleksandr Popov
 * @date 2022--2024
 * @copyright GNU GPLv3
 */

#include "defs.hpp"

#include <cassert>
#include <limits>
#include <type_traits>

namespace dtypes {
    std::size_t hash_helper(Loc a, Loc b) noexcept {
        auto i = static_cast<std::make_unsigned_t<Loc>>(a);
        auto j = static_cast<decltype(i)>(b);
        static_assert(sizeof(std::size_t) >= 2 * sizeof(decltype(i)));
        return static_cast<std::size_t>(i) << sizeof(i) * 8 | j;
    }

    std::size_t LocHash::operator()(Cell const& p) const noexcept {
        return hash_helper(p.first, p.second);
    }

    Loc operator ""_loc(unsigned long long l) {
        return static_cast<Loc>(l);
    }
}

namespace util {
    Probs normalise(Visits const& v, Cell const& s) {
        Probs res;
        Cnt total = v.at(s);
        for (auto const& [cell, cnt]: v)
            res[cell] = divide(cnt, total);
        return res;
    }

    Frac divide(Cnt const& num, Cnt const& den) {
        Frac res(num, den);
        res.canonicalize();
        return res;
    }

    double getd(Frac const& fr) {
        return fr.get_d();
    }

    bool can_subtract(Loc const& a, Loc const& b) {
        auto mxr = std::numeric_limits<Loc>::max();
        return sign_of(a) == sign_of(b)
            || (a >= -mxr && b >= -mxr && mxr - absv(a) >= absv(b));
    }
}

namespace map {
    std::size_t TrajHash::operator()(BridgeID const& t) const noexcept {
        ::dp::LocHash lhasher;
        auto [first, last, time] = t;
        // https://stackoverflow.com/a/1646913/126995
        std::size_t ret{17};
        ret = 31 * ret + lhasher(first);
        ret = 31 * ret + lhasher(last);
        ret = 31 * ret + time;
        return ret;
    }

    BridgeID to_bridge_id(Meas const& a, Meas const& b) {
        auto const& [t1, x1, y1] = a;
        auto const& [t2, x2, y2] = b;
        assert(t2 > t1);
        return {{x1, y1}, {x2, y2}, t2 - t1};
    }

    Cell meas_to_cell(Meas const& m) {
        auto const& [t, x, y] = m;
        return {x, y};
    }
}
