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

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "dp.hpp"
#include "problems.hpp"
#include "explicit.hpp"

namespace {
    /**
     * @brief Invoke a function with arguments and time it with a wall clock.
     * Currently only supports non-void functions.
     * @param f The function to invoke.
     * @param arsg The arguments to the function.
     * @return The pair: whatever the function returns, and the time it took.
     */
    template<typename Duration = std::chrono::nanoseconds,
        typename F, typename ... Args>
    std::pair<std::invoke_result_t<F, Args...>, Duration>
            time_and_save(F&& f, Args&&... args) {
        auto const start = std::chrono::high_resolution_clock::now();
        auto res = std::forward<F>(f)(std::forward<Args>(args)...);
        auto const end = std::chrono::high_resolution_clock::now();
        return {res, std::chrono::duration_cast<Duration>(end - start)};
    }

    /**
     * @brief Output the last layer of the DP to a stream.
     * @param table The DP.
     * @param T The T of the DP; we output the layer at time T.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void dp_write(dp::DP const& table, dp::Time const& T,
            std::pair<dp::Loc, dp::Loc> const& shift, std::ostream& outf) {
        auto [is, js] = shift;
        auto sT = static_cast<dp::Loc>(T);
        outf << T << '\n';
        for (dp::Loc i = is - sT; i <= is + sT; ++i)
            for (dp::Loc j = js - sT; j <= js + sT; ++j)
                outf << table.at(i, j, T) << (j < js + sT ? ' ' : '\n');
        outf.flush();
    }

    /**
     * @brief Output the flattened DP to a stream.
     * @param table The DP.
     * @param T The T of the DP; we output the layer at time T.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void flat_write(dp::DP const& table, dp::Time const& T,
            std::pair<dp::Loc, dp::Loc> const& shift, std::ostream& outf) {
        auto fl_table = table.flatten(T);
        auto [is, js] = shift;
        auto sT = static_cast<dp::Loc>(T);
        outf << T << '\n';
        for (dp::Loc i = is - sT; i <= is + sT; ++i)
            for (dp::Loc j = js - sT; j <= js + sT; ++j)
                outf << fl_table[{i, j}] << (j < js + sT ? ' ' : '\n');
    }

    /**
     * @brief Output a trajectory to a stream.
     * @param traj The trajectory.
     * @param outf The output stream.
     */
    void traj_write(std::vector<std::pair<dp::Loc, dp::Loc>> const& traj,
            std::ostream& outf) {
        for (auto const& [i, j]: traj)
            outf << i << ' ' << j << '\n';
    }

    /**
     * @brief Check if the path counts at time T match up for the DP and the
     * explicit computation.
     * @param T The T of the explicit computation, not larger than T of DP.
     * @param a The DP.
     * @param b The explicit table.
     * @param shift The start point of both `a` and `b`.
     * @return "correct" if the counts match, "mismatch" otherwise.
     */
    std::string check_paths(dp::Time const& T, dp::DP const& a,
            xpl::Table const& b, std::pair<dp::Loc, dp::Loc> const& shift) {
        auto [is, js] = shift;
        auto sT = static_cast<dp::Loc>(T);
        bool correct = true;
        for (dp::Loc i = is - sT; i <= is + sT; ++i) {
            for (dp::Loc j = js - sT; j <= js + sT; ++j) {
                auto b_it = b.find({i, j});
                auto b_val = (b_it != b.end() ? b_it->second : 0);
                correct &= (a.at(i, j, T) == b_val);
            }
        }
        return correct ? "correct" : "mismatch";
    }

    /**
     * @brief Check if the visit counts match up for the DP and the explicit
     * computation.
     * @param T The T of both the explicit computation and the DP.
     * @param a The DP.
     * @param b The explicit table.
     * @return "correct" if the counts match, "mismatch" otherwise.
     */
    std::string check_visits(dp::Time const& T, dp::DP const& a,
            xpl::Table const& b) {
        auto af = a.flatten(T);
        return (af == b ? "correct" : "mismatch");
    }
}

int main() {
    using ms = std::chrono::milliseconds;
    using dp::operator""_loc;

    bool visits;
    do {
        char answer;
        std::cout << "Welcome! Do you want to run the exploration mode (e), or"
            << " to compute the visit counts (v)? [e/v]\n> ";
        std::cin >> answer;
        if (!std::cin || (answer != 'e' && answer != 'v')) {
            std::cout << "Please type e or v.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else {
            visits = (answer == 'v');
            break;
        }
    } while (true);

    if (visits) {
        dp::Time ta, tb;
        std::cin >> ta >> tb;
        std::stringstream vname;
        for (dp::Time t = ta; t < tb; ++t) {
            std::cout << t << ' ' << std::flush;
            auto tS = static_cast<dp::Loc>(t);
            for (dp::Loc x = 0; x <= tS; ++x) {
                auto bnd = tS - x < x ? tS - x : x;
                for (dp::Loc y = 0; y <= bnd; ++y) {
                    auto dp = prob::visit_all(t, {0_loc, 0_loc}, {x, y});
                    vname << "data/visits/v_" << t << '_' << x << '_' << y;
                    std::ofstream outi(vname.str());
                    flat_write(dp, t, {0, 0}, outi);
                    vname.str(std::string());
                    vname.clear();
                }
            }
        }
        std::cout << '\n';
        return 0;
    }

    std::cout << "Part 0: correctness checks\n\n";
    auto shift = std::make_pair<>(1_loc, 2_loc);
    dp::Time T0 = 8;
    dp::DP a(T0, dp::uniform_prop, shift);
    dp::DP b(T0, dp::uniform_prop, shift, {}, true);
    auto tbl = xpl::compute_paths(T0, shift);
    std::cout << "sparse: " << check_paths(T0, a, tbl, shift) << ", dense: "
        << check_paths(T0, b, tbl, shift) << "\n";
    std::ofstream out00("data/paths_dp_wrong");
    dp_write(a, T0, shift, out00);
    std::ofstream out01("data/paths_dp_correct");
    dp_write(b, T0, shift, out01);

    std::cout << "Part 1: timing\nWe run both the DP and the naive version, "
        << "comparing the time to compute all\npaths and to compute the paths "
        << "that visit a location.\n\n";
    dp::Time T1, T2, T3;
    do {
        std::cout << "Please input the time limit T for the DP:\n> ";
        std::cin >> T1;
        if (!std::cin) {
            std::cout << "It should be an integer between "
                << std::numeric_limits<dp::Time>::min() << " and "
                << std::numeric_limits<dp::Loc>::max() << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else
            break;
    } while (true);

    std::cout << "Computing the DP for all paths... " << std::flush;
    auto [r1, t1] = time_and_save(prob::all_paths, 10u,
        std::make_pair<>(0_loc, 0_loc), std::initializer_list<dp::Blocked>{});
    std::cout << "done." << std::endl;
    std::ofstream out1("data/paths_dp");
    dp_write(r1, 10u, {0, 0}, out1);

    std::cout << "Computing the DP for visits... " << std::flush;
    auto [r2, t2] = time_and_save(prob::visit_all, T1,
        std::make_pair<>(0_loc, 0_loc), std::make_pair<>(2_loc, 1_loc));
    std::cout << "done.\n" << std::endl;
    std::ofstream out2("data/visits_dp");
    flat_write(r2, T1, {0, 0}, out2);

    do {
        std::cout << "Please input the time limit T for the explicit "
            << "computation:\n> ";
        std::cin >> T2;
        if (!std::cin) {
            std::cout << "It should be an integer between "
                << std::numeric_limits<dp::Time>::min() << " and "
                << std::numeric_limits<dp::Loc>::max() << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else
            break;
    } while (true);

    std::cout << "Computing all paths explicitly... " << std::flush;
    auto [r3, t3] = time_and_save(xpl::compute_paths, T2,
        std::make_pair<>(0_loc, 0_loc));
    std::cout << "done." << std::endl;

    std::cout << "Computing visits explicitly... " << std::flush;
    auto [r4, t4] = time_and_save(xpl::visits, T2,
        std::make_pair<>(0_loc, 0_loc), std::make_pair<>(2_loc, 1_loc));
    std::cout << "done.\n" << std::endl;

    if (T2 <= T1) {
        std::cout << "Checking correctness for paths... " << std::flush;
        std::cout << check_paths(T2, r1, r3, {0_loc, 0_loc}) << '\n';
    }
    if (T2 == T1) {
        std::cout << "Checking correctness for visits... " << std::flush;
        std::cout << check_visits(T2, r2, r4) << "\n\n";
    }

    std::cout << "Times (ms):\n" << "Problem       DP Explicit\nPaths   "
        << std::setw(8) << std::chrono::duration_cast<ms>(t1).count() << ' '
        << std::setw(8) << std::chrono::duration_cast<ms>(t3).count()
        << "\nVisits  "
        << std::setw(8) << std::chrono::duration_cast<ms>(t2).count() << ' '
        << std::setw(8) << std::chrono::duration_cast<ms>(t4).count() << "\n\n";

    std::cout << "Part 2: obstacles\nWe run the DP with obstacles to provide "
        << "intuition about the propagation\nbehaviour in the presence of "
        << "obstacles. We run some examples by default and\nsave them. You "
        << "can input your own example as a sequence of tuples (x, y, t),\n"
        << "e.g. (1, 0, 2), (2, 0, 1), (3, 0, 2) to indicate cells that are "
        << "blocked\nstarting at time t, namely, 2, 1, and 2.\n\n";

    bool own = false;
    do {
        char answer;
        std::cout << "Would you like to run your own example? [y/n]\n> ";
        std::cin >> answer;
        if (!std::cin || (answer != 'y' && answer != 'n')) {
            std::cout << "Please type y or n.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else {
            own = (answer == 'y');
            break;
        }
    } while (true);

    std::unordered_set<dp::Blocked> wall;
    for (dp::Loc i = -10; i <= 10; ++i)
        wall.emplace(i, 3, 0);
    auto o1 = prob::all_paths(10, {0, 0}, wall);
    std::ofstream wall1("data/wall");
    dp_write(o1, 10, {0, 0}, wall1);

    for (dp::Loc i = 1; i <= 3; ++i)
        wall.erase({i, 3, 0});
    auto o2 = prob::all_paths(10, {0, 0}, wall);
    std::ofstream wall2("data/wall_gap");
    dp_write(o2, 10, {0, 0}, wall2);

    wall.clear();
    for (dp::Loc i = -1; i <= 2; ++i)
        wall.emplace(i, 3, 0);
    auto o3 = prob::all_paths(10, {0, 0}, wall);
    std::ofstream wall3("data/sm_wall");
    dp_write(o3, 10, {0, 0}, wall3);

    wall.erase({0, 3, 0});
    auto o4 = prob::all_paths(10, {0, 0}, wall);
    std::ofstream wall4("data/sm_wall_gap");
    dp_write(o4, 10, {0, 0}, wall4);

    if (own) {
        wall.clear();
        // TODO: parse own example.
    }

    dp::Cnt pc;
    std::cout << "Part 3: generation\nWe generate several trajectories from a "
        << "start to an end point using a DP. We\nneed O(T^3) time for the DP, "
        << "and O(T) time for every trajectory of length T.\n";
    do {
        std::cout << "Please input the time limit T and the number of "
            << "trajectories:\n> ";
        std::cin >> T3 >> pc;
        if (!std::cin) {
            std::cout << "The count should be non-negative; the time limit "
                << "should be an integer between\n"
                << std::numeric_limits<dp::Time>::min() << " and "
                << std::numeric_limits<dp::Loc>::max() << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else
            break;
    } while (true);

    if (pc > 0) {
        dp::Loc si, sj, ei, ej;
        do {
            std::cout << "Please input the start and end points as i1 j1 i2 "
                << "j2:\n> ";
            std::cin >> si >> sj >> ei >> ej;
            if (!std::cin) {
                std::cout << "The values should be integers between "
                    << std::numeric_limits<dp::Loc>::min() << " and "
                    << std::numeric_limits<dp::Loc>::max() << ".\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                    '\n');
            }
            else
                break;
        } while (true);

        auto paths = prob::all_paths(T3, {si, sj});
        for (dp::Cnt c = 0; c < pc; ++c) {
            auto ti = prob::generate_path(T3, paths, {ei, ej});
            std::string fname("data/traj");
            fname += c.get_str();
            std::ofstream out3(fname);
            traj_write(ti, out3);
        }
    }
    return 0;
}
