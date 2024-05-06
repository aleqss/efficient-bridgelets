/* Copyright 2022, 2023, 2024 Aleksandr Popov
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

#ifndef DEFS_H
#define DEFS_H

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

/* In case you want to use different numeric types, this header and its
 * implementation are hopefully the only files that need to be rewritten.
 * For example, for `Frac` that is `double`, `getd` becomes a pass-through
 * function, and divide should return `res.get_d();` instead.
 * Also, adapt `decode` and `max_num` in explicit.cpp.
 */
namespace dp {
    /// Count in a cell.
    using Cnt = mpz_class;
    /// Time steps.
    using Time = std::uint32_t;
    /// Discrete x/y coordinate.
    using Loc = std::int32_t;
    /// Discrete location.
    using Cell = std::pair<Loc, Loc>;

    /**
     * @brief Hash for a coordinate pair.
     * @param a The x-coordinate.
     * @param b The y-coordinate.
     * @return The hash value.
     */
    std::size_t hash_helper(Loc a, Loc b) noexcept;

    /**
     * Hasher for std::unordered_map of coordinate pairs.
     */
    struct LocHash {
        std::size_t operator()(Cell const& p) const noexcept;
    };

    /// Flat DS storing visit counts to specific cells.
    using Visits = std::unordered_map<Cell, Cnt, LocHash>;

    /**
     * Literal of type Loc.
     */
    Loc operator ""_loc(unsigned long long l);
}

namespace util {
    using ::dp::Loc, ::dp::Cell, ::dp::LocHash, ::dp::Visits, ::dp::Cnt;
    using ::dp::operator""_loc;
    /// Fraction of `Cnt`s.
    using Frac = mpq_class;
    /// Flat DS storing visit probabilities (exactly with GMP).
    using Probs = std::unordered_map<Cell, Frac, LocHash>;

    /**
     * @brief Normalise a `Visits` map to the value in the cell `s`.
     * @param v A valid visit count map, like the one returned by
     * `dp::DP::flatten`.
     * @param s The start cell whose value should be normalised to 1.
     * @return The normalised map (using fractions currently).
     */
    Probs normalise(Visits const& v, Cell const& s);

    /**
     * @brief For GMP integers, we need a different implementation for
     * division, going through `mpq_class`.
     * @param num The numerator.
     * @param den The denominator.
     * @return `num` / `den`.
     */
    Frac divide(Cnt const& num, Cnt const& den);

    /**
     * @brief For GMP fractions, we need to call a special functions to
     * approximately convert them to `double`.
     * @param fr The fraction.
     * @return The closest representation of `fr`.
     */
    double getd(Frac const& fr);

    /**
     * @brief Return the sign of the value.
     * @param val The possibly negative value.
     * @return -1 for a negative value, 1 for a non-negative value.
     */
    inline Loc sign_of(Loc const& val) {
        return val < 0 ? -1_loc : 1_loc;
    }

    /**
     * @brief Compute the absolute value.
     * @param val The possibly negative value.
     * @return |val| without overflow handling.
     */
    inline Loc absv(Loc const& val) {
        return val < 0 ? -val : val;
    }

    /**
     * @brief Check that a - b does not overflow.
     * @param a The minuend.
     * @param b The subtrahend.
     * @return False iff the difference overflows.
     */
    [[maybe_unused]] bool can_subtract(Loc const& a, Loc const& b);
}

namespace map {
    using ::dp::Loc, ::dp::Time, ::dp::Cell, ::dp::Visits, ::util::Frac,
        ::util::Probs;
    /// Measurement: discrete location with a timestamp.
    using Meas = std::tuple<Time, Loc, Loc>;
    /// Trajectory: sequence of measurements.
    using Traj = std::vector<Meas>;
    /// Identifier for bridges: start, end, and duration.
    using BridgeID = std::tuple<Cell, Cell, Time>;
    /// Identifier for subtrajectories: trajectory ID, start, and end indices.
    using SubTraj = std::tuple<std::uint32_t, std::size_t, std::size_t>;

    /**
     * Hasher for std::undordered_map of bridge identifiers.
     */
    struct TrajHash {
        std::size_t operator()(BridgeID const& t) const noexcept;
    };

    /**
     * @brief Get a bridge ID when going from `a` to `b`.
     * @param a The start of the bridge.
     * @param b The end of the bridge.
     * @return The bridge ID for going from `a` to `b` in correct time.
     */
    BridgeID to_bridge_id(Meas const& a, Meas const& b);

    /**
     * @brief Convert a measurement (with time) to a cell (ignoring time).
     * @param The measurement (t, x, y).
     * @return The cell (x, y).
     */
    Cell meas_to_cell(Meas const& m);
}
#endif
