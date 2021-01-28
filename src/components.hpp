#pragma once

#include "ember/component_common.hpp"
#include "ember/entities.hpp"
#include "ember/net_id.hpp"
#include "ember/ez3d.hpp"

#include <sushi/sushi.hpp>
#include <box2d/box2d.h>

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <memory>

#include "ember/reflection.hpp"

#define EMBER_REFLECTION_ENABLE_REGISTRY
#include "ember/reflection_start.hpp"

namespace component {

/** World transform */
struct transform : sushi::transform {};
REFLECT(transform, (pos)(rot)(scl))

struct sprite {
    std::string texture;
    glm::vec2 size = {1, 1};
    std::vector<int> frames;
    float time = 0;
};
REFLECT(sprite, (texture)(size)(frames)(time))

struct rigid_body {
    b2Body* body;
};
REFLECT(rigid_body, (body))

} // namespace component

#include "ember/reflection_end.hpp"
