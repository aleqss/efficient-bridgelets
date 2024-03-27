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

#include "debug_print.hpp"

#include <ostream>

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> const& pr) {
    os << '(' << pr.first << ", " << pr.second << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, ::map::Meas const& pr) {
    os << '(' << std::get<0>(pr) << ", " << std::get<1>(pr) << ", "
        << std::get<2>(pr) << ')';
    return os;
}

template std::ostream& operator<<(std::ostream&, ::dp::Cell const&);
template std::ostream& operator<<(std::ostream&,
    std::pair<::dp::Cell, ::dp::Cnt> const&);
template std::ostream& operator<<(std::ostream&,
    std::pair<::dp::Cell, ::util::Frac> const&);

template<typename K, typename V, class Hash>
std::ostream& operator<<(std::ostream& os,
        std::unordered_map<K, V, Hash> const& pr) {
    os << "{ ";
    for (auto const& kv: pr)
        os << kv << ' ';
    os << "}";
    return os;
}

template std::ostream& operator<<(std::ostream&,
    ::dp::Visits const&);
template std::ostream& operator<<(std::ostream&,
    ::util::Probs const&);

template<typename K>
std::ostream& operator<<(std::ostream& os, std::vector<K> const& tr) {
    os << "{ ";
    for (auto const& loc: tr)
        os << loc << ' ';
    os << "}";
    return os;
}

template std::ostream& operator<<(std::ostream&,
    std::vector<::map::Meas> const&);
