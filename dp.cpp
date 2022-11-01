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

#include "dp.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace dp {
    Blocked::Blocked(Loc x, Loc y, Time s): i{std::move(x)}, j{std::move(y)},
            start{std::move(s)} {
        // Intentionally left blank.
    }

    bool Blocked::operator==(Blocked const& o) const {
        return i == o.i && j == o.j;
    }

    bool DP::test_index(Loc const& i, Loc const& j, Time const& t) const {
        auto ts = static_cast<Loc>(T);
        auto [si, sj] = shift;
        Blocked test(i, j, 0);
        auto loc = blocked.find(test);
        auto tf = flip ? T - t : t;
        Loc is = f * (i - si) + ts, js = f * (j - sj) + ts;
        return t <= T && is >= 0 && js >= 0
            && static_cast<Time>(is) <= 2 * T && static_cast<Time>(js) <= 2 * T
            && (loc == blocked.end() || tf < loc->start);
    }

    std::size_t DP::index(Loc const& i, Loc const& j, Time const& t) const {
        assert(t <= T);
        auto ts = static_cast<Loc>(T);
        auto [si, sj] = shift;
        Loc is = f * (i - si) + ts, js = f * (j - sj) + ts;
        auto tf = flip ? T - t : t;
        std::size_t ret = tf * (2 * T + 1) * (2 * T + 1);
        ret += static_cast<std::size_t>(is) * (2 * T + 1);
        ret += static_cast<std::size_t>(js);
        return ret;
    }

    Cnt& DP::at(Loc const& i, Loc const& j, Time const& t) {
        if (t > T)
            throw std::invalid_argument("t is larger than T");
        if (!test_index(i, j, t))
            throw std::out_of_range("Index not modifiable.");
        return table[index(i, j, t)];
    }

    Cnt DP::at(Loc const& i, Loc const& j, Time const& t) const {
        if (t > T)
            throw std::invalid_argument("t is larger than T");
        return test_index(i, j, t) ? table[index(i, j, t)] : 0;
    }

    DP::DP(Time max_time, std::function<Cnt(DP const&, Loc const&, Loc const&,
            Time const&)> propagate, std::pair<Loc, Loc> origin,
            std::unordered_set<Blocked> const& blocked_cells):
            T{std::move(max_time)}, table((T + 1) * (2 * T + 1) * (2 * T + 1)) {
        if (T > std::numeric_limits<Loc>::max())
            throw std::length_error("Please pick a lower value of T.");

        auto [is, js] = origin;
        for (auto const& cell: blocked_cells)
            blocked.emplace(cell.i - is, cell.j - js, cell.start);

        Loc Ts = static_cast<Loc>(T);
        for (Loc i = -Ts; i <= Ts; ++i) {
            for (Loc j = -Ts; j <= Ts; ++j) {
                auto loc = blocked.find(Blocked(i, j, 0));
                if (loc == blocked.end() || loc->start > 0)
                    at(i, j, 0) = (i == 0 && j == 0) ? 1 : 0;
            }
        }

        for (Time t = 0; t < T; ++t) {
            for (Loc i = -Ts; i <= Ts; ++i) {
                for (Loc j = -Ts; j <= Ts; ++j) {
                    auto loc = blocked.find(Blocked(i, j, 0));
                    if (loc == blocked.end() || t + 1 < loc->start)
                        at(i, j, t + 1) = propagate(*this, i, j, t);
                }
            }
        }

        set_shift(std::move(origin));
    }

    void DP::flip_time() {
        flip = !flip;
    }

    void DP::flip_coords() {
        f *= -1;
    }

    void DP::set_shift(std::pair<Loc, Loc> origin) {
        auto [i, j] = origin;
        auto l = std::numeric_limits<Loc>::min() + static_cast<Loc>(T);
        auto u = std::numeric_limits<Loc>::max() - static_cast<Loc>(T);
        if (i <= l || j <= l || i >= u || j >= u)
            throw std::out_of_range("Please shift less.");

        shift = std::move(origin);
    }

    DP DP::operator*(DP const& other) const {
        if (flip == other.flip || T != other.T)
            throw std::invalid_argument("These DPs cannot be combined.");

        DP res(*this);
        res.blocked.clear();
        auto const* r = this;
        Loc Ts = static_cast<Loc>(T);
        auto [xs, ys] = shift;
        for (Time t = 0; t <= T; ++t)
            for (Loc i = xs - Ts; i <= xs + Ts; ++i)
                for (Loc j = ys - Ts; j <= ys + Ts; ++j)
                    res.at(i, j, t) = r->at(i, j, t) * other.at(i, j, t);
        res.blocked = blocked;
        return res;
    }

    std::unordered_map<std::pair<Loc, Loc>, Cnt, LocHash> DP::flatten(Time
            const& max_time) const {
        std::unordered_map<std::pair<Loc, Loc>, Cnt, LocHash> res;
        auto const* r = this;
        auto [is, js] = shift;
        auto tmax = max_time < T ? max_time : T;
        auto ts = static_cast<Loc>(tmax);
        for (Loc i = is - ts; i <= is + ts; ++i)
            for (Loc j = js - ts; j <= js + ts; ++j)
                for (Time t = 0; t <= tmax; ++t)
                    if (r->at(i, j, t) > 0)
                        res[{i, j}] += r->at(i, j, t);
        return res;
    }

    Cnt uniform_prop(DP const& r, Loc const& i, Loc const& j, Time const& t) {
        return r.at(i, j, t) + r.at(i - 1, j, t) + r.at(i + 1, j, t)
            + r.at(i, j - 1, t) + r.at(i, j + 1, t);
    }
}
