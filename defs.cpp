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

#include "defs.hpp"

namespace dp {
    std::size_t hash_helper(Loc a, Loc b) noexcept {
        auto i = static_cast<std::make_unsigned_t<Loc>>(a);
        auto j = static_cast<decltype(i)>(b);
        static_assert(sizeof(std::size_t) >= 2 * sizeof(decltype(i)));
        return static_cast<std::size_t>(i) << sizeof(i) * 8 | j;
    }

    std::size_t LocHash::operator()(std::pair<Loc, Loc> const&
            p) const noexcept {
        return hash_helper(p.first, p.second);
    }

    Loc operator"" _loc(unsigned long long l) {
        return static_cast<Loc>(l);
    }
}
