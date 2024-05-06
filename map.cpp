/* Copyright 2023, 2024 Aleksandr Popov
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
 * @brief Implementation related to the grid.
 * @author Aleksandr Popov
 * @date 2023, 2024
 * @copyright GNU GPLv3
 */

#include "map.hpp"

#include <cassert>
#include <istream>
#include <ostream>
#include <queue>
#include <sstream>
#include "bridge.hpp"
#include "problems.hpp"

namespace map {
    constexpr bool EdgeEqualV::operator()(Edge const& a, Edge const& b) const {
        return a.v == b.v;
    }

    ShortcutGraph::ShortcutGraph(std::vector<Vertex> const& tr,
            map::Map const& map): vs(tr) {
        for (std::size_t i = 0; i < vs.size() - 1; ++i)
            for (std::size_t j = i + 1; j < vs.size(); ++j)
                if (map.covered(vs, i, j, Map::Inter::none))
                    adj[i].insert({j, 0});
    }

    void ShortcutGraph::add_single_hops() {
        for (std::size_t i = 0; i < vs.size() - 1; ++i)
            adj[i].insert({i + 1, 1});
        // Insert will silently fail if {i + 1, 0} exists in the set.
    }

    bool ShortcutGraph::shortest_path(std::vector<std::size_t>& ret,
            std::function<bool(std::size_t, std::size_t)> check_hops) {
        std::vector<int> distance(vs.size(), std::numeric_limits<int>::max());
        std::vector<std::size_t> hops(vs.size(),
            std::numeric_limits<std::size_t>::max());
        std::vector<std::size_t> prev(vs.size());

        distance[0] = 0;
        hops[0] = 0;
        prev[0] = 0;

        for (std::size_t i = 0; i < vs.size(); ++i) {
            for (auto [j, w]: adj[i]) {
                auto alt = distance[i] + w;
                if (distance[j] > alt) {
                    distance[j] = alt;
                    hops[j] = hops[i] + 1;
                    prev[j] = i;
                }
                else if (distance[j] == alt) {
                    if (check_hops(hops[j], hops[i])) {
                        hops[j] = hops[i] + 1;
                        prev[j] = i;
                    }
                }
            }
        }

        ret.clear();
        if (distance[vs.size() - 1] < std::numeric_limits<int>::max()) {
            std::size_t cur{vs.size() - 1};
            ret.push_back(cur);
            while (cur != prev[cur]) {
                cur = prev[cur];
                ret.push_back(cur);
            }
        }
        ret.shrink_to_fit();
        return ret.size() > 0;
    }

    bool Map::is_present(Meas const& s, Meas const& e) const {
        return map.count(to_bridge_id(s, e));
    }

    Probs Map::single_query(Meas const& s, Meas const& e) const {
        if (!is_present(s, e))
            return ::util::bridgelet(s, e, diag);

        std::vector<Probs> pr_maps;
        for (auto const& [tr_id, tr_s, tr_e]: map.at(to_bridge_id(s, e)))
            pr_maps.push_back(::util::bridge(get_tr(tr_id), tr_s, tr_e, diag));
        return ::util::average(pr_maps);
    }

    std::pair<bool, std::vector<std::size_t>> Map::find_path(Traj const& tr,
            Inter use_points) const {
        std::function<bool(std::size_t, std::size_t)> chk_hops;
        switch (use_points) {
        case Inter::few:
            chk_hops = [](std::size_t hj, std::size_t hi) noexcept {
                return hj > hi + 1;
            };
            break;
        case Inter::most:
            chk_hops = [](std::size_t hj, std::size_t hi) noexcept {
                return hj < hi + 1;
            };
            break;
        default:
            throw std::domain_error("No path to find when using all or no "
                "intermediate points.");
        }

        ShortcutGraph sg(tr, *this);
        std::vector<std::size_t> path;
        bool full_cover = sg.shortest_path(path, chk_hops);
        if (!full_cover) {
            sg.add_single_hops();
            [[maybe_unused]] auto r = sg.shortest_path(path, chk_hops);
            assert(r);
        }
        return {full_cover, path};
    }

    void Map::enable_diag() {
        diag = true;
    }

    Traj const& Map::get_tr(std::uint32_t id) const {
        return tr_reg.at(id);
    }

    void Map::train(Traj tr, std::uint32_t trid) {
        for (std::size_t i = 0u; i < tr.size() - 3u; ++i)
            for (std::size_t j = i + 3u; j < tr.size(); ++j)
                map[to_bridge_id(tr[i], tr[j])].emplace_front(trid, i, j);
        tr_reg.emplace(trid, std::move(tr));
    }

    void Map::train_all(
            std::vector<std::pair<Traj, std::uint32_t>> const& trs) {
        for (auto const& tr: trs)
            train(tr.first, tr.second);
    }

    std::pair<bool, Probs> Map::query(Traj const& tr, Inter use_points) const {
        switch (use_points) {
        case Inter::none: {
            auto s = tr.front(), e = tr.back();
            return {is_present(s, e), single_query(s, e)};
        }
        case Inter::few:
        case Inter::most: {
            auto [cov, path] = find_path(tr, use_points);
            std::vector<Probs> res;
            for (std::size_t i = 0; i < path.size() - 1; ++i)
                res.push_back(single_query(tr[path[i]], tr[path[i + 1]]));
            return {cov, ::util::sequence(res)};
        }
        case Inter::all: {
            bool allc = true;
            std::vector<Probs> res;
            for (std::size_t i = 0; i < tr.size() - 1; ++i) {
                allc &= is_present(tr[i], tr[i + 1]);
                res.push_back(single_query(tr[i], tr[i + 1]));
            }
            return {allc, ::util::sequence(res)};
        }
        default:
            throw std::domain_error("Unhandled value of Inter enum.");
        }
    }

    bool Map::covered(Traj const& tr, std::size_t s, std::size_t e,
            Inter use_points) const {
        switch (use_points) {
        case Inter::none:
            return is_present(tr[s], tr[e]);
        case Inter::few:
        case Inter::most: {
            ShortcutGraph gr(tr, *this);
            std::vector<std::size_t> ign;
            return gr.shortest_path(ign);
        }
        case Inter::all: {
            bool cov = true;
            for (std::size_t i = s; i < e - 1; ++i)
                cov &= is_present(tr[i], tr[i + 1]);
            return cov;
        }
        default:
            throw std::domain_error("Unhandled value of Inter enum.");
        }
    }

    double Map::pred_error(Probs const& pred, Traj const& gr) const {
        std::unordered_set<Cell, dp::LocHash> vs;
        for (auto [t, x, y]: gr)
            vs.emplace(std::move(x), std::move(y));
        Frac res = vs.size();
        for (auto const& [cell, prob]: pred) {
            if (vs.count(cell))
                res -= prob;
            else
                res += prob;
        }
        return ::util::getd(res);
    }

    std::pair<bool, double> Map::query_error(Traj const& tr, Traj const& gr,
            Inter use_points) const {
        auto [cov, probs] = query(tr, use_points);
        auto error = pred_error(probs, gr);
        return {cov, error};
    }

    void Map::load(std::istream& inf,
            std::unordered_map<std::uint32_t, Traj> trajs) {
        tr_reg = std::move(trajs);
        std::string istr;
        std::istringstream line;
        Time t;
        Loc x1, y1, x2, y2;
        std::uint32_t trid;
        std::size_t start, end;
        while (std::getline(inf, istr)) {
            line.clear();
            line.str(std::move(istr));
            line >> x1 >> y1 >> x2 >> y2 >> t;
            while (line >> trid) {
                line >> start >> end;
                map[{{x1, y1}, {x2, y2}, t}].emplace_front(trid, start, end);
            }
        }
    }

    void Map::save(std::ostream& outf) const {
        for (auto const& [k, v]: map) {
            auto const& [a, b, t] = k;
            auto const& [x1, y1] = a;
            auto const& [x2, y2] = b;
            outf << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2
                << ' ' << t << ' ';
            for (auto const& [id, s, e]: v)
                outf << id << ' ' << s << ' ' << e << ' ';
            outf << '\n';
        }
    }
}
