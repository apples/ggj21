#pragma once

#include <sol.hpp>

#include <cmath>

namespace ember::math {

inline auto imod(double x, double y) -> int {
    return x - y * std::floor(x / y);
}

} // namespace ember::math
