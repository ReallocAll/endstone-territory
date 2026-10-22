#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <tuple>

namespace claim_geometry {
using Point = std::tuple<double, double, double>;
inline double block(double value) {
    if (!std::isfinite(value) || value < -30000000 || value > 30000000)
        throw std::invalid_argument("坐标超出有效范围");
    return std::floor(value);
}
inline Point normalize(const Point& p) {
    return {block(std::get<0>(p)), block(std::get<1>(p)), block(std::get<2>(p))};
}
inline std::int64_t area(double x1, double z1, double x2, double z2) {
    const auto width = static_cast<std::int64_t>(std::abs(block(x2) - block(x1))) + 1;
    const auto depth = static_cast<std::int64_t>(std::abs(block(z2) - block(z1))) + 1;
    return width * depth; // At most 60000001 squared, within int64_t.
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
