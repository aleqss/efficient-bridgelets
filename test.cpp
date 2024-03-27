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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <catch2/catch_test_macros.hpp>
#pragma GCC diagnostic pop

#include "bridge.hpp"
// #include "problems.hpp"
#include "io.hpp"
#include <filesystem>
#include <fstream>
#include "debug_print.hpp"

TEST_CASE("bridgelets at origin no diag", "[bridge]") {
    ::util::Probs c1{{{0, 0}, 1}, {{1, 0}, {2, 3}}, {{2, 0}, {1, 3}},
                      {{0, 1}, {1, 3}}, {{1, 1}, {2, 3}}, {{2, 1}, 1}};
    REQUIRE(c1 == ::util::bridgelet({0, 0, 0}, {3, 2, 1}, false));

    ::util::Probs c2{{{0, 0}, 1}, {{1, 0}, {1, 5}}, {{-1, 0}, {1, 5}},
        {{0, 1}, {1, 5}}, {{0, -1}, {1, 5}}};
    REQUIRE(c2 == ::util::bridgelet({0, 0, 0}, {2, 0, 0}, false));

    ::util::Probs c3{{{0, 0}, 1}, {{1, 0}, 1}, {{2, 0}, 1}};
    REQUIRE(c3 == ::util::bridgelet({0, 0, 0}, {2, 2, 0}, false));
}

TEST_CASE("bridgelets with shift no diag", "[bridge]") {
    ::util::Probs c1{{{-1, -1}, 1}, {{0, -1}, {2, 3}}, {{1, -1}, {1, 3}},
                      {{-1, 0}, {1, 3}}, {{0, 0}, {2, 3}}, {{1, 0}, 1}};
    REQUIRE(c1 == ::util::bridgelet({1, -1, -1}, {4, 1, 0}, false));

    ::util::Probs c2{{{-1, -1}, 1}, {{-2, -1}, {3, 13}}, {{0, -1}, {3, 13}},
        {{-1, -2}, {3, 13}}, {{-1, 0}, {3, 13}}};
    REQUIRE(c2 == ::util::bridgelet({1, -1, -1}, {4, -1, -1}, false));

    ::util::Probs c3{{{2, 0}, 1}, {{3, 0}, 1}, {{4, 0}, 1}, {{5, 0}, 1}};
    REQUIRE(c3 == ::util::bridgelet({1, 2, 0}, {4, 5, 0}, false));
}

TEST_CASE("bridgelets at origin with diag", "[bridge]") {
    ::util::Probs c1{{{0, 0}, 1}, {{1, 0}, {1, 2}},
                                  {{1, 1}, {1, 2}}, {{2, 1}, 1}};
    REQUIRE(c1 == ::util::bridgelet({0, 0, 0}, {2, 2, 1}, true));

    ::util::Probs c2{{{0, 0}, 1}, {{1, 0}, {1, 9}}, {{-1, 0}, {1, 9}},
        {{0, 1}, {1, 9}}, {{0, -1}, {1, 9}}, {{1, 1}, {1, 9}},
        {{1, -1}, {1, 9}}, {{-1, 1}, {1, 9}}, {{-1, -1}, {1, 9}}};
    REQUIRE(c2 == ::util::bridgelet({0, 0, 0}, {2, 0, 0}, true));

    ::util::Probs c3{{{0, 0}, 1}, {{1, 1}, 1}, {{2, 2}, 1}};
    REQUIRE(c3 == ::util::bridgelet({0, 0, 0}, {2, 2, 2}, true));
}

TEST_CASE("bridgelets with shift with diag", "[bridge]") {
    ::util::Probs c1{{{-1, -1}, 1}, {{0, -1}, {1, 2}},
                                    {{0, 0}, {1, 2}}, {{1, 0}, 1}};
    REQUIRE(c1 == ::util::bridgelet({1, -1, -1}, {3, 1, 0}, true));

    ::util::Probs c2{{{-1, 1}, 1}, {{0, 1}, {1, 9}}, {{-2, 1}, {1, 9}},
        {{-1, 2}, {1, 9}}, {{-1, 0}, {1, 9}}, {{0, 2}, {1, 9}},
        {{0, 0}, {1, 9}}, {{-2, 2}, {1, 9}}, {{-2, 0}, {1, 9}}};
    REQUIRE(c2 == ::util::bridgelet({3, -1, 1}, {5, -1, 1}, true));

    ::util::Probs c3{{{-1, -2}, 1}, {{0, -1}, 1}, {{1, 0}, 1}};
    REQUIRE(c3 == ::util::bridgelet({2, -1, -2}, {4, 1, 0}, true));
}

TEST_CASE("sequence is correct", "[bridge]") {
    auto c1 = ::util::bridgelet({1, -1, -1}, {4, 1, 0}, false);
    auto c2 = ::util::bridgelet({4, 1, 0}, {7, 4, 0}, false);
    auto merged(c1);
    merged.insert(c2.begin(), c2.end());
    REQUIRE(merged == ::util::sequence({c1, c2}));
}

TEST_CASE("bridge is correct", "[bridge]") {
    //
}

TEST_CASE("average is correct", "[bridge]") {
    //
}

TEST_CASE("train", "") {}
TEST_CASE("covered", "") {}
TEST_CASE("query", "") {}
TEST_CASE("pred_error", "") {}

TEST_CASE("read traj", "[io]") {
    ::map::Traj cor{{0,4988,52601}, {1,4988,52601}, {3,4988,52599}, {4,4988,52599},
        {6,4990,52599}, {7,4991,52599}, {8,4991,52598}, {9,4991,52598},
        {12,4993,52596}, {15,4994,52595}, {16,4994,52594}, {17,4994,52594},
        {18,4995,52594}, {19,4995,52593}, {20,4994,52593}, {21,4994,52592},
        {22,4994,52591}, {24,4995,52591}, {26,4995,52589}, {27,4995,52589},
        {29,4995,52587}, {38,4993,52581}, {40,4995,52581}, {41,4996,52581},
        {42,4996,52581}, {45,4996,52581}, {47,4998,52580}, {52,5001,52580}};
    std::filesystem::path fname = "./movement";
    fname /= std::to_string(1);
    std::ifstream trainf(fname / std::to_string(0));
    auto traj = io::read_traj(trainf);
    REQUIRE(traj == cor);
}
