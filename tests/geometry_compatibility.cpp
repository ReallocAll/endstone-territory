#include "claim_geometry.h"

#include <stdexcept>
#include <cmath>
#include <limits>
#include <string>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error("Failed: " #condition); } while (false)

int main() {
    using namespace claim_geometry;
    // Valid command input keeps decimal coordinates; loaded runtime bounds use
    // the legacy toward-zero conversion, not floor or inclusive block counts.
    const Point raw{-10.5, 64.75, -2.5};
    CHECK(validate(raw) == raw);
    CHECK(normalize(raw) == Point(-10, 64, -2));
    CHECK(normalize(Point{10.5, -2.5, 0.5}) == Point(10, -2, 0));
    CHECK(normalize(Point{-10, 64, -2}) == Point(-10, 64, -2));
    CHECK(normalizeUpdate(Point{0.9999999, -0.9999999, 10.9999999}) == Point(1, -1, 11));
    CHECK(normalizeUpdate(raw) == Point(-10, 64, -2));
    for (const auto* text : {"-10.5", "-2.5", "0.0", "10.5", "100"})
        CHECK(block(std::stod(text)) == std::stoi(text));

    CHECK(area(0, 0, 100, 100) == 10000);
    CHECK(area(100, 100, 0, 0) == 10000);
    CHECK(area(-10.5, -10.5, -2.5, -2.5) == 64);
    CHECK(area(-1.2, -1.2, 0.2, 0.2) == 1);
    CHECK(area(0, 0, 0, 100) == 0); // Preserve legacy single-row pricing.
    CHECK(area(0.2, 0.2, 0.8, 0.8) == 0);
    CHECK(area(-30000000, -30000000, 30000000, 30000000) == 3600000000000000LL);

    // Keep the arithmetic and resource safety checks introduced in 0.4.3.
    for (double invalid : {std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN(), 30000001.0}) {
        bool rejected = false;
        try { (void)coordinate(invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        CHECK(rejected);
    }
    CHECK(!indexable({0, 0, 0}, {1000000, 0, 0}));
}
