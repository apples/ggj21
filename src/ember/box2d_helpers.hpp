#pragma once

#include <box2d/box2d.h>

namespace ember {

class box2d_fixed_timestep {
public:
    box2d_fixed_timestep();

    explicit box2d_fixed_timestep(float timeStep);

    box2d_fixed_timestep(float timeStep, int velocityIterations, int positionIterations);

    auto step(b2World& world, float delta) -> bool;

private:
    float timeStep;
    int velocityIterations;
    int positionIterations;
    float accum;
};

} // namespace ember
