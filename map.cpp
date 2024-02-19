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

    bool ShortcutGraph::sp_unweighted(std::vector<std::size_t>& ret) {
        std::vector<bool> visited(vs.size());
        std::vector<std::size_t> prev(vs.size());
        std::queue<std::size_t> q;
        visited[0] = true;
        prev[0] = 0;
        q.push(0u);

        while (!q.empty() && !visited[vs.size() - 1]) {
            auto cur = q.front();
            for (auto [n, w]: adj[cur]) {
                visited[n] = true;
                prev[n] = cur;
                q.push(std::move(n));
            }
            q.pop();
        }

        ret.clear();
        if (visited[vs.size() - 1]) {
            std::size_t cur{vs.size() - 1};
            ret.push_back(cur);
            while (cur != prev[cur]) {
                cur = prev[cur];
                ret.push_back(cur);
            }
        }
        ret.shrink_to_fit();
        return visited[vs.size() - 1];
    }

    bool ShortcutGraph::sp_weighted(std::vector<std::size_t>& ret) {
        std::vector<int> distance(vs.size(), std::numeric_limits<int>::max());
        std::vector<std::size_t> hops(vs.size(),
            std::numeric_limits<std::size_t>::max());
        std::vector<std::size_t> prev(vs.size());
        std::priority_queue<Edge, std::vector<Edge>,
            EdgeOrderW<std::greater>> q;
        prev[0] = 0;
        distance[0] = 0;
        hops[0] = 0;
        q.push({0, distance[0]});

        while (!q.empty()) {
            auto [cid, cw] = q.top();
            if (cw <= distance[cid]) {
                for (auto [n, w]: adj[cid]) {
                    auto alt = cw + w;
                    if (alt < distance[n]) {
                        distance[n] = alt;
                        prev[n] = cid;
                        hops[n] = hops[cid] + 1;
                        q.push({n, alt});
                    }
                    else if (alt == distance[n]) {
                        if (hops[n] > hops[cid] + 1) {
                            hops[n] = hops[cid] + 1;
                            prev[n] = cid;
                            q.push({n, alt});
                        }
                    }
                }
            }
            q.pop();
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

    ShortcutGraph::ShortcutGraph(std::vector<Vertex> const& tr,
            map::Map const& map): vs(tr) {
        for (std::size_t i = 0; i < vs.size() - 1; ++i)
            for (std::size_t j = i + 1; j < vs.size(); ++j)
                if (map.covered(vs, i, j, false))
                    adj[i].insert({j, 0});
    }

    void ShortcutGraph::add_single_hops() {
        for (std::size_t i = 0; i < vs.size() - 1; ++i)
            adj[i].insert({i + 1, 1});
        added_hops = true;
    }

    bool ShortcutGraph::shortest_path(std::vector<std::size_t>& ret) {
        return added_hops ? sp_weighted(ret) : sp_unweighted(ret);
    }

    bool Map::is_present(Meas const& s, Meas const& e) const {
        return map.count(to_bridge_id(s, e));
    }

    Probs Map::single_query(Meas const& s, Meas const& e) const {
        if (!is_present(s, e))
            return ::util::bridgelet(s, e);

        std::vector<Probs> pr_maps;
        for (auto const& [tr_id, tr_s, tr_e]: map.at(to_bridge_id(s, e)))
            pr_maps.push_back(::util::bridge(get_tr(tr_id), tr_s, tr_e));
        return ::util::average(pr_maps);
    }

    std::pair<bool, std::vector<std::size_t>> Map::find_path(
            Traj const& tr) const {
        ShortcutGraph sg(tr, *this);
        std::vector<std::size_t> path;
        bool full_cover = sg.shortest_path(path);
        if (!full_cover) {
            sg.add_single_hops();
            [[maybe_unused]] auto r = sg.shortest_path(path);
            assert(r);
        }
        return {full_cover, path};
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

    std::pair<bool, Probs> Map::query(Traj const& tr,
            bool intermediate) const {
        if (!intermediate) {
            auto s = tr.front(), e = tr.back();
            return {is_present(s, e), single_query(s, e)};
        }

        auto [cov, path] = find_path(tr);
        std::vector<Probs> res;
        for (std::size_t i = 0; i < path.size() - 1; ++i)
            res.push_back(single_query(tr[path[i]], tr[path[i + 1]]));
        return {cov, ::util::sequence(res)};
    }

    bool Map::covered(Traj const& tr, std::size_t s, std::size_t e,
            bool intermediate) const {
        if (!intermediate)
            return is_present(tr[s], tr[e]);

        ShortcutGraph gr(tr, *this);
        std::vector<std::size_t> ign;
        return gr.shortest_path(ign);
    }

    double Map::pred_error(Probs const& pred, Traj const& gr) const {
        std::unordered_set<Cell, dp::LocHash> vs;
        for (auto [t, x, y]: gr)
            vs.emplace(std::move(x), std::move(y));
        Frac res;
        for (auto const& [cell, prob]: pred) {
            if (vs.count(cell))
                res -= prob;
            else
                res += prob;
        }
        res += vs.size();
        return ::util::getd(res);
    }

    std::pair<bool, double> Map::query_error(Traj const& tr, Traj const& gr,
            bool intermediate) const {
        auto [cov, probs] = query(tr, intermediate);
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
