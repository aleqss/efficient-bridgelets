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

#include "problems.hpp"

#include <random>

namespace prob {
    using ::dp::DP, ::dp::Time, ::dp::Loc, ::dp::Cnt, ::dp::Blocked;
    DP all_paths(Time T, std::pair<Loc, Loc> start,
            std::unordered_set<Blocked> const& blocked) {
        DP res(std::move(T), dp::uniform_prop, std::move(start), blocked);
        return res;
    }

    DP visit_all(Time T, std::pair<Loc, Loc> start, std::pair<Loc, Loc> end) {
        DP first_visit(T, dp::uniform_prop, {0, 0}, {{0, 0, 1}});
        first_visit.set_shift(std::move(start));
        first_visit.flip_coords();
        DP rest(std::move(T), dp::uniform_prop);
        rest.flip_time();
        rest.set_shift(std::move(end));
        return first_visit * rest;
    }

    std::vector<std::pair<Loc, Loc>> generate_path(Time const& T,
            DP const& paths, std::pair<Loc, Loc> const& end) {
        auto [ci, cj] = end;
        if (paths.at(ci, cj, T) == 0)
            return {};

        std::vector<std::pair<Loc, Loc>> ret(T + 1);
        std::random_device rd;
        std::mt19937_64 helper(rd());
        std::uniform_int_distribution<unsigned int> seeder;
        gmp_randclass gen(gmp_randinit_mt);
        gen.seed(seeder(helper));
        for (Time t = T; t > 0; --t) {
            ret[t] = {ci, cj};
            Cnt total = paths.at(ci, cj, t);
            Cnt prev_counts[] = {paths.at(ci, cj, t - 1),
                paths.at(ci - 1, cj, t - 1), paths.at(ci, cj - 1, t - 1),
                paths.at(ci + 1, cj, t - 1), paths.at(ci, cj + 1, t - 1)};

            Cnt rchoice = gen.get_z_range(total);
            unsigned choice = 0;
            while (rchoice >= prev_counts[choice]) {
                rchoice -= prev_counts[choice];
                ++choice;
            }

            switch (choice) {
                case 0: break;
                case 1: --ci; break;
                case 2: --cj; break;
                case 3: ++ci; break;
                case 4: ++cj; break;
                default: break;
            }
        }
        ret[0] = {ci, cj};
        return ret;
    }
}
