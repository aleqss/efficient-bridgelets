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

#ifndef IO_H
#define IO_H

#include <iosfwd>
#include <vector>
#include "defs.hpp"

namespace dp {
    class DP;
}

namespace io {
    using ::dp::Loc, ::dp::Time, ::dp::Cell, ::map::Traj;

    /**
     * @brief Output the last layer of the DP to a stream.
     * @param table The DP.
     * @param T The T of the DP; we output the layer at time T.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void dp_write(dp::DP const& table, Time const& T, Cell const& shift,
        std::ostream& outf);

    /**
     * @brief Output the flattened DP to a stream.
     * @param table The DP.
     * @param T The T of the DP; we output the layer at time T.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void flat_write(dp::DP const& table, Time const& T, Cell const& shift,
        std::ostream& outf);

    /**
     * @brief Output a trajectory to a stream.
     * @param traj The trajectory.
     * @param outf The output stream.
     */
    void traj_write(std::vector<Cell> const& traj, std::ostream& outf);

    /**
     * @brief Read a trajectory from a stream.
     *
     * The trajectory is expected in CSV format, one point per line, starting
     * with line four, as t,x,y.
     * @param inf The input stream.
     * @return The sequence of (t, x, y) tuples.
     */
    Traj read_traj(std::istream& inf);
}
#endif


/*
std::array cnts{803456, 485103, 336692, 295115};
std::stringstream fname;

for (auto mode = 1; mode <= 4; ++mode) {
    for (auto tri = 0; tri < cnts[mode - 1]; ++tri) {
        fname << "movement/" << mode << "/" << tri;
        std::ifstream intraj(fname.str());
        io::read_traj(intraj);

        flat_write(dp, t, {0, 0}, outi);
        fname.str(std::string());
        fname.clear();
    }
}


std::unordered_map<std::tuple<std::pair<Loc, Loc>, std::pair<Loc, Loc>, Time>, std::list<std::tuple<std::uint32_t, std::size_t, std::size_t>>>
*/
