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

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

#include "bridge.hpp"
#include "dp.hpp"
#include "explicit.hpp"
#include "io.hpp"
#include "map.hpp"
#include "problems.hpp"

namespace {
    /**
     * @brief Invoke a function with arguments and time it with a wall clock.
     * Currently only supports non-void functions.
     * @param f The function to invoke.
     * @param args The arguments to the function.
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
     * @brief Read one or more flags with checking.
     * @param prompt The prompt (without newline at the end).
     * @param wrong The message displayed if the input did not pass validation.
     * @param check The function that takes all the read parameters and
     * validates them (returns true if validation passes).
     * @param vars The flags to be read.
     */
    template <typename Test, typename ... Ts>
    void read_flag(char const* prompt, char const* wrong, Test check,
            Ts& ... vars) {
        do {
            std::cout << prompt << "\n> ";
            (std::cin >> ... >> vars);
            if (!std::cin || !check(vars...)) {
                std::cout << wrong << '\n';
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                    '\n');
            }
            else
                break;
        } while (true);
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
    bool check_paths(dp::Time const& T, dp::DP const& a,
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
        return correct;
    }

    /**
     * @brief Check if the visit counts match up for the DP and the explicit
     * computation.
     * @param T The T of both the explicit computation and the DP.
     * @param a The DP.
     * @param b The explicit table.
     * @return "correct" if the counts match, "mismatch" otherwise.
     */
    bool check_visits(dp::Time const& T, dp::DP const& a,
            xpl::Table const& b) {
        return a.flatten(T) == b;
    }

    /**
     * @brief Run the correctness checks on DP computation.
     */
    void dp_correct() {
        using dp::operator""_loc;
        std::cout << "Part 0: correctness checks\n\n";
        auto shift = std::make_pair(1_loc, 2_loc);
        dp::Time T0 = 8;
        dp::DP a(T0, dp::uniform_prop, shift);
        dp::DP b(T0, dp::uniform_prop, shift, {}, true);
        auto tbl = xpl::compute_paths(T0, shift);
        std::cout << "sparse: "
            << (check_paths(T0, a, tbl, shift) ? "correct" : "mismatch")
            << ", dense: "
            << (check_paths(T0, b, tbl, shift) ? "correct" : "mismatch")
            << "\n";
        std::ofstream out00("./data/paths_dp_wrong");
        io::dp_write(a, T0, shift, out00);
        std::ofstream out01("./data/paths_dp_correct");
        io::dp_write(b, T0, shift, out01);
    }

    /**
     * @brief Time the DP computation compared to the explicit version.
     */
    void dp_time() {
        using ms = std::chrono::milliseconds;
        using dp::operator""_loc;
        std::cout << "Part 1: timing\nWe run the DP and the naive version, "
            "comparing the time to compute all\npaths and to compute the paths"
            " that visit a location.\n\n";
        dp::Time T1, T2;
        std::ostringstream wr_time;
        wr_time << "It should be an integer between "
            << std::numeric_limits<dp::Time>::min() << " and "
            << std::numeric_limits<dp::Loc>::max() << '.';
        read_flag("Please input the time limit T for the DP:",
            wr_time.str().c_str(), [](dp::Time const& t) {
                return t <= std::numeric_limits<dp::Loc>::max();
            }, T1);

        std::cout << "Computing the DP for all paths... " << std::flush;
        auto [r1, t1] = time_and_save(prob::all_paths, 10u,
            std::make_pair(0_loc, 0_loc),
            std::initializer_list<dp::Blocked>{}, false);
        std::cout << "done." << std::endl;
        std::ofstream out1("./data/paths_dp");
        io::dp_write(r1, 10u, {0, 0}, out1);

        std::cout << "Computing the DP for visits... " << std::flush;
        auto [r2, t2] = time_and_save(prob::visit_all, T1,
            std::make_pair(0_loc, 0_loc), std::make_pair(2_loc, 1_loc), false);
        std::cout << "done.\n" << std::endl;
        std::ofstream out2("./data/visits_dp");
        io::flat_write(r2, T1, {0, 0}, out2);

        read_flag("Please input the time limit T for the explicit computation:",
            wr_time.str().c_str(), [](dp::Time const& t) {
                return t <= std::numeric_limits<dp::Loc>::max();
            }, T2);

        std::cout << "Computing all paths explicitly... " << std::flush;
        auto [r3, t3] = time_and_save(xpl::compute_paths, T2,
            std::make_pair(0_loc, 0_loc));
        std::cout << "done." << std::endl;

        std::cout << "Computing visits explicitly... " << std::flush;
        auto [r4, t4] = time_and_save(xpl::visits, T2,
            std::make_pair(0_loc, 0_loc), std::make_pair(2_loc, 1_loc));
        std::cout << "done.\n" << std::endl;

        if (T2 <= T1) {
            std::cout << "Checking correctness for paths... " << std::flush;
            std::cout << (check_paths(T2, r1, r3, {0_loc, 0_loc}) ?
                "correct" : "mismatch") << '\n';
        }
        if (T2 == T1) {
            std::cout << "Checking correctness for visits... " << std::flush;
            std::cout << (check_visits(T2, r2, r4) ? "correct" : "mismatch")
                << "\n\n";
        }

        std::cout << "Times (ms):\n" << "Problem       DP Explicit\nPaths   "
            << std::setw(8) << std::chrono::duration_cast<ms>(t1).count() << ' '
            << std::setw(8) << std::chrono::duration_cast<ms>(t3).count()
            << "\nVisits  "
            << std::setw(8) << std::chrono::duration_cast<ms>(t2).count() << ' '
            << std::setw(8) << std::chrono::duration_cast<ms>(t4).count()
            << "\n\n";
    }

    /**
     * @brief Use the DP with some hand-constructed obstacles.
     */
    void dp_obstacles() {
        std::cout << "Part 2: obstacles\nWe run the DP with obstacles to "
            "provide intuition about the propagation\nbehaviour in the "
            "presence of obstacles. We run some examples by default and\nsave "
            "them. You can input your own example as a sequence of tuples "
            "(x, y, t),\ne.g. (1, 0, 2), (2, 0, 1), (3, 0, 2) to indicate "
            "cells that are blocked\nstarting at time t, namely, 2, 1, 2.\n\n";

        char own_ch;
        read_flag("Would you like to run your own example? [y/n]",
            "Please type y or n.", [](char const& c) {
                return c == 'y' || c == 'n';
            }, own_ch);
        bool own = (own_ch == 'y');

        std::unordered_set<dp::Blocked> wall;
        for (dp::Loc i = -10; i <= 10; ++i)
            wall.emplace(i, 3, 0);
        auto o1 = prob::all_paths(10, {0, 0}, wall);
        std::ofstream wall1("./data/wall");
        io::dp_write(o1, 10, {0, 0}, wall1);

        for (dp::Loc i = 1; i <= 3; ++i)
            wall.erase({i, 3, 0});
        auto o2 = prob::all_paths(10, {0, 0}, wall);
        std::ofstream wall2("./data/wall_gap");
        io::dp_write(o2, 10, {0, 0}, wall2);

        wall.clear();
        for (dp::Loc i = -1; i <= 2; ++i)
            wall.emplace(i, 3, 0);
        auto o3 = prob::all_paths(10, {0, 0}, wall);
        std::ofstream wall3("./data/sm_wall");
        io::dp_write(o3, 10, {0, 0}, wall3);

        wall.erase({0, 3, 0});
        auto o4 = prob::all_paths(10, {0, 0}, wall);
        std::ofstream wall4("./data/sm_wall_gap");
        io::dp_write(o4, 10, {0, 0}, wall4);

        if (own) {
            wall.clear();
            // TODO: parse own example.
        }
    }

    /**
     * @brief Generate some paths.
     */
    void dp_gen_paths() {
        dp::Time T3;
        dp::Cnt pc;
        std::cout << "Part 3: generation\nWe generate several trajectories "
            "from a start to an end point using a DP. We\nneed O(T^3) time for"
            " the DP, and O(T) time for every trajectory of length T.\n";

        std::ostringstream wr_path;
        wr_path << "The count should be non-negative; the time limit should be"
            " an integer between\n" << std::numeric_limits<dp::Time>::min()
            << " and " << std::numeric_limits<dp::Loc>::max() << '.';

        read_flag("Please input the time limit T and the number of "
            "trajectories:", wr_path.str().c_str(),
            [](dp::Time const& t, dp::Cnt const& c){
                return c >= 0 && t <= std::numeric_limits<dp::Loc>::max();
            }, T3, pc);

        if (pc > 0) {
            dp::Loc si, sj, ei, ej;
            wr_path.str("");
            wr_path.clear();
            wr_path << "The values should be integers between "
                << std::numeric_limits<dp::Loc>::min() << " and "
                << std::numeric_limits<dp::Loc>::max() << '.';
            read_flag("Please input the start and end points as i1 j1 i2 j2:",
                wr_path.str().c_str(), []([[maybe_unused]] dp::Loc const& s1,
                        [[maybe_unused]] dp::Loc const& s2,
                        [[maybe_unused]] dp::Loc const& e1,
                        [[maybe_unused]] dp::Loc const& e2) {
                    return true;
                }, si, sj, ei, ej);

            auto paths = prob::all_paths(T3, {si, sj});
            for (dp::Cnt c = 0; c < pc; ++c) {
                auto ti = prob::generate_path(T3, paths, {ei, ej});
                std::string fname("./data/traj");
                fname += c.get_str();
                std::ofstream out3(fname);
                io::traj_write(ti, out3);
            }
        }
    }

    /**
     * @brief Run the exploration mode, showcasing the bridgelets.
     */
    void explore() {
        std::cout << "Exploration mode.\n\n";
        dp_correct();
        dp_time();
        dp_obstacles();
        dp_gen_paths();
    }

    /**
     * @brief Batch-compute the visit counts for all possible locations.
     */
    void batch_visits() {
        std::cout << "Visits computation.\n\n";
        using dp::operator""_loc;
        dp::Time ta, tb;

        std::ostringstream wr_vis;
        wr_vis << "Both times should be non-negative integers below "
            << std::numeric_limits<dp::Loc>::max()
            << ",\nand t1 should be smaller than t2.";
        read_flag("Input the two times t1 t2 to compute all visits in [t1, t2)"
            " time steps.", wr_vis.str().c_str(),
            [](dp::Time const& t1, dp::Time const& t2) {
                return t1 < t2 && t2 <= std::numeric_limits<dp::Loc>::max();
            }, ta, tb);

        std::ostringstream vname;
        for (dp::Time t = ta; t < tb; ++t) {
            std::cout << t << ' ' << std::flush;
            auto tS = static_cast<dp::Loc>(t);
            for (dp::Loc x = 0; x <= tS; ++x) {
                auto bnd = tS - x < x ? tS - x : x;
                for (dp::Loc y = 0; y <= bnd; ++y) {
                    auto dp = prob::visit_all(t, {0_loc, 0_loc}, {x, y});
                    vname << "data/visits/v_" << t << '_' << x << '_' << y;
                    std::ofstream outi(vname.str());
                    io::flat_write(dp, t, {0, 0}, outi);
                    vname.str("");
                    vname.clear();
                }
            }
        }
        std::cout << '\n';
    }

    /**
     * @brief Compute the median of the vector elements.
     * @param vec The vector.
     * @param comp The less-than function for the elements of type `T`.
     * @param ext The extractor that gets the value to use for the median
     * computation from an element of type `T`.
     * @return The median of the elements of `vec`, extracted using `ext`,
     * where the ordering is defined by `comp`.
     */
    template <typename T, typename Comp, typename Ext>
    double median(std::vector<T>& vec, Comp comp, Ext ext) {
        assert(vec.size() > 0);
        auto mid = vec.begin() + std::distance(vec.begin(), vec.end()) / 2;
        std::nth_element(vec.begin(), mid, vec.end(), comp);
        auto ret = static_cast<double>(ext(*mid));
        if (vec.size() % 2 == 0) {
            ret += ext(*std::max_element(vec.begin(), mid, comp));
            ret /= 2.0;
        }
        return ret;
    }

    /**
     * @brief Compute the median of the vector elements.
     * @param vec The vector with numerical elements.
     * @return The median of the vector.
     */
    template<typename T>
    double median(std::vector<T>& vec) {
        return median(vec, [](T const& a, T const& b) { return a < b; },
            [](T const& t) { return t; });
    }

    map::Traj select_points(map::Traj const& tr, map::Map::Inter use_points) {
        if (tr.size() < 2)
            return tr;
        switch (use_points) {
        case map::Map::Inter::none:
            return {tr.front(), tr.back()};
        case map::Map::Inter::few:
        case map::Map::Inter::most:
            return {}; // Figure this out.
        case map::Map::Inter::all:
            return tr;
        default:
            throw std::domain_error("Unhandled value of Inter enum");
        }
    }

    /**
     * @brief Run the experiment with bridges on the OpenPFLOW data.
     */
    void use_map() {
        std::cout << "Registering the training trajectories.\n\n";
        short mode;
        read_flag("Which movement mode would you like to analyse: "
            "walking (1), driving (2), taking\na train (3), or cycling (4)?",
            "Please type 1, 2, 3, or 4.", [](short m) {
                return m >= 1 && m <= 4;
            }, mode);

        char sparse_ch;
        read_flag("Should the trajectories be made sparse? [y/n]",
            "Please type y or n.", [](char s) {
                return s == 'y' || s == 'n';
            }, sparse_ch);
        bool sparse = (sparse_ch == 'y');

        char inter_ch;
        read_flag("Do you want to use the intermediate points during training,"
            " and if so, should\nall be used, or maximum or minimum amount "
            "needed? [n(o)/a(ll)/f(ew)/m(ost)]", "Please type n, a, f, or m.",
            [](char i) {
                return i == 'n' || i == 'a' || i == 'f' || i == 'm';
            }, inter_ch);
        auto use_points = (inter_ch == 'n' ? map::Map::Inter::none :
            (inter_ch == 'a' ? map::Map::Inter::all : (inter_ch == 'f' ?
                map::Map::Inter::few : map::Map::Inter::most)));

        char diag_ch;
        read_flag("Should diagonal movement be allowed? [y/n]",
            "Please type y or n.", [](char d) {
                return d == 'y' || d == 'n';
            }, diag_ch);
        bool diag = (diag_ch == 'y');

        std::filesystem::path fname = "./movement";
        fname /= std::to_string(mode);
        std::ifstream trainlist(fname / "train.txt");
        auto trainfiles = io::read_flist(trainlist);
        map::Map reg;
        if (diag)
            reg.enable_diag();
        for (auto const& train_id: trainfiles) {
            std::ifstream trainf(fname / std::to_string(train_id));
            if (sparse)
                reg.train(io::sparsify(io::read_traj(trainf)), train_id);
            else
                reg.train(io::read_traj(trainf), train_id);
        }

        using PBD = std::pair<bool, double>;
        std::cout << "Testing trajectories.\n\n";
        std::ofstream stats(fname / "stats.txt");
        std::ifstream testlist(fname / "test.txt");
        std::vector<PBD> err, err_learned, err_naive;
        auto testfiles = io::read_flist(testlist);
        for (auto const& test_id: testfiles) {
            std::ifstream testf(fname / std::to_string(test_id));
            auto ground = io::read_traj(testf);
            auto [c, probs] = reg.query(sparse ? io::sparsify(ground) : ground,
                use_points);
            auto e = reg.pred_error(probs, ground);
            auto e_learned = reg.pred_error(util::ignore_pr(probs), ground);

            auto naive = select_points(sparse ? io::sparsify(ground) : ground,
                use_points);
            auto e_naive = reg.pred_error(util::bridge(naive, 0,
                naive.size() - 1, diag), ground);
            // auto [c, e] = reg.query_error(
            //     sparse ? io::sparsify(ground) : ground, ground, use_points);
            stats << test_id << ' ' << c << ' ' << e << ' ' << e_learned << ' '
                << e_naive << '\n';
            err.emplace_back(c, e);
            err_learned.emplace_back(c, e_learned);
            err_naive.emplace_back(c, e_naive);
        }
        std::vector<decltype(err)> it{err, err_learned, err_naive};
        for (auto& errors: it) {
            std::vector<double> cov, uncov;
            for (auto const& [c, e]: errors) {
                if (c)
                    cov.push_back(e);
                else
                    uncov.push_back(e);
            }
            auto med = median(errors, [](PBD const& a, PBD const& b) {
                    return a.second < b.second; },
                [](PBD const& p) {return p.second;});
            auto med_cov = median(cov);
            auto med_uncov = median(uncov);
            std::cout << "Median errors:\nCovered: " << med_cov << "\nUncovered: "
                << med_uncov << "\nTotal: " << med << '\n';
        }
    }
}

/**
 * @brief Drive the program capabilities appropriately.
 */
int main() {
    do {
        char answer;
        read_flag("Welcome! Do you want to run the exploration mode (e), to "
            "compute the visit counts (v), or to run the training and testing "
            "with the map (m)? [e/v/m]", "Please type e, v, or m.",
            [](char a) {
                return a == 'e' || a == 'v' || a == 'm';
            }, answer);

        switch (answer) {
        case 'e':
            explore();
            break;
        case 'v':
            batch_visits();
            break;
        case 'm':
            use_map();
            break;
        default:
            assert(false);
            break;
        }

        char qyn;
        std::cout << "Would you like to run anything else? [y/n]\n> ";
        std::cin >> qyn;
        if (!std::cin || qyn != 'y') {
            std::cout << "Quitting.\n";
            break;
        }
    } while (true);
    return 0;
}
