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

#ifndef DEFS_H
#define DEFS_H

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <type_traits>
#include <utility>

namespace dp {
	using Cnt = mpz_class;
    using Time = std::uint32_t;
    using Loc = std::int32_t;

    /**
     * @brief Hash for a coordinate pair.
     * @param a The x-coordinate.
     * @param b The y-coordinate.
     * @return The hash value.
     */
    std::size_t hash_helper(Loc a, Loc b) noexcept;

    /**
     * Hasher for unordered_map of pairs.
     */
    struct LocHash {
        std::size_t operator()(std::pair<Loc, Loc> const& p) const noexcept;
    };

    Loc operator"" _loc(unsigned long long l);
}
#endif
