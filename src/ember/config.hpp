#pragma once

#include <string>

#include "reflection.hpp"

#include "reflection_start.hpp"

namespace ember::config {

struct display_t {
    int width;
    int height;
};
REFLECT(display_t, (width)(height))

struct config {
    display_t display;
};
REFLECT(config, (display))

} // namespace ember::config

#include "reflection_end.hpp"
