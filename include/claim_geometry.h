#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>

namespace claim_geometry {
using Point = std::tuple<double, double, double>;
inline double coordinate(double value) {
    if (!std::isfinite(value) || value < -30000000 || value > 30000000)
        throw std::invalid_argument("坐标超出有效范围");
    return value;
}
inline double block(double value) {
    // Legacy loaded claims and pricing truncate coordinates towards zero.
    return std::trunc(coordinate(value));
}
inline Point validate(const Point& p) {
    return {coordinate(std::get<0>(p)), coordinate(std::get<1>(p)), coordinate(std::get<2>(p))};
}
inline Point normalize(const Point& p) {
    return {block(std::get<0>(p)), block(std::get<1>(p)), block(std::get<2>(p))};
}
inline Point normalizeUpdate(const Point& p) {
    // resizeTerritory persists std::to_string (six decimal places). Match the
    // legacy reload after that rounding, including values next to integers.
    const auto stored = [](double value) {
        return block(std::stod(std::to_string(coordinate(value))));
    };
    return {stored(std::get<0>(p)), stored(std::get<1>(p)), stored(std::get<2>(p))};
}
inline std::int64_t area(double x1, double z1, double x2, double z2) {
    const auto width = static_cast<std::int64_t>(std::abs(block(x2) - block(x1)));
    const auto depth = static_cast<std::int64_t>(std::abs(block(z2) - block(z1)));
    return width * depth; // Legacy endpoint-difference pricing, with safe wide arithmetic.
}
inline bool indexable(const Point& a, const Point& b) {
    const auto x1 = block(std::get<0>(a)), x2 = block(std::get<0>(b));
    const auto z1 = block(std::get<2>(a)), z2 = block(std::get<2>(b));
    const auto cellsX = std::abs(std::floor(x1 / 128) - std::floor(x2 / 128)) + 1;
    const auto cellsZ = std::abs(std::floor(z1 / 128) - std::floor(z2 / 128)) + 1;
    return std::abs(x2 - x1) <= 65535 && std::abs(z2 - z1) <= 65535
        && cellsX * cellsZ <= 4096;
}
}
