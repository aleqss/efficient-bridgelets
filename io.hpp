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
 * @brief I/O functions.
 * @author Aleksandr Popov
 * @date 2023, 2024
 * @copyright GNU GPLv3
 */

#ifndef IO_H
#define IO_H

#include <cstdint>
#include <iosfwd>
#include <vector>
#include "defs.hpp"

namespace dp {
    class DP;
}

/**
 * @brief Collection of I/O functions.
 */
namespace io {
    /**
     * @brief Output the last layer of the `dp::DP` to a stream.
     * @param table The `dp::DP`.
     * @param T The \f$T\f$ of the DP; we output the layer at time \f$T\f$.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void dp_write(dp::DP const& table, Time const& T, Cell const& shift,
        std::ostream& outf);

    /**
     * @brief Output the flattened `dp::DP` to a stream.
     * @param table The DP.
     * @param T The \f$T\f$ of the DP; we output the DP flattened up to time
     * \f$T\f$.
     * @param shift The start point of the DP.
     * @param outf The output stream.
     */
    void flat_write(dp::DP const& table, Time const& T, Cell const& shift,
        std::ostream& outf);

    /**
     * @brief Output the probability map to a stream.
     *
     * Output format: `{x y} pr` for any cell with non-zero probability.
     * @param pr The probability map.
     * @param outf The output stream.
     */
    void probs_write(Probs const& pr, std::ostream& outf);

    /**
     * @brief Output an untimed trajectory to a stream.
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
     * @return The sequence of \f$(t, x, y)\f$ tuples.
     */
    Traj read_traj(std::istream& inf);

    /**
     * @brief Make a trajectory sparse by omitting measurements closer than
     * @p skip units in time.
     * @param dense The original trajectory.
     * @param skip The number of time steps within which we wish to not have
     * any measurements.
     * @return The sparser trajectory.
     */
    Traj sparsify(Traj const& dense, Time const& skip = 5u);

    /**
     * @brief Read a list of trajectory identifiers.
     * @param inf The input stream.
     * @return The resulting list of trajectory IDs.
     */
    std::vector<std::uint32_t> read_flist(std::istream& inf);

    /**
     * @brief Convert a cell to a string representation.
     * @param cell A cell (x, y).
     * @return The string representation, currently `{x y}`.
     */
    std::string to_string(Cell const& cell);
}

/**
 * @brief Output the @p err to a stream with appropriate precision.
 * @param outs The output stream.
 * @param err The error with `fp`, `fn`, and `total`.
 * @return outs The stream with the same properties, after outputting `fp`,
 * `fn`, and `total` in that order.
 */
std::ostream& operator<<(std::ostream& outs, io::Error const& err);
#endif
