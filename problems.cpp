/* Copyright 2022, 2024 Aleksandr Popov
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
 * @brief Compute visit counts and generate paths: implementation.
 * @author Aleksandr Popov
 * @date 2022, 2024
 * @copyright GNU GPLv3
 */

#include "problems.hpp"

#include <random>
#include <utility>

namespace prob {
    std::function<Cnt(dp::DP const&, Loc const&, Loc const&,
            Time const&)> Prop::propagate() const {
        if (diag)
            return stay ? dp::staying_diag_prop : dp::uniform_diag_prop;
        return stay ? dp::staying_prop : dp::uniform_prop;
    }

    bool Prop::dense() const {
        return diag;
    }

    dp::DP all_paths(Time T, Cell start, Prop const& prop,
            std::unordered_set<dp::Blocked> const& blocked) {
        dp::DP res(std::move(T), prop.propagate(), std::move(start), blocked,
            prop.dense());
        return res;
    }

    dp::DP visit_all(Time T, Cell start, Cell end, Prop const& prop) {
        dp::DP first_visit(T, prop.propagate(), {0, 0}, {{0, 0, 1}},
            prop.dense());
        first_visit.set_shift(std::move(start));
        first_visit.flip_coords();
        dp::DP rest(std::move(T), prop.propagate(), {0, 0}, {}, prop.dense());
        rest.flip_time();
        rest.set_shift(std::move(end));
        return first_visit * rest;
    }

    std::vector<Cell> generate_path(Time const& T, dp::DP const& paths,
            Cell const& end) {
        auto [ci, cj] = end;
        if (paths.at(ci, cj, T) == 0)
            return {};

        std::vector<Cell> ret(T + 1);
        std::random_device rd;
        std::mt19937_64 helper(rd());
        std::uniform_int_distribution<unsigned int> seeder;
        gmp_randclass gen(gmp_randinit_mt);
        gen.seed(seeder(helper));

        // Easy fix for diag: change prev_counts, add cases 5--8.
        // Maybe something more general possible for arbitrary kernels?
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
