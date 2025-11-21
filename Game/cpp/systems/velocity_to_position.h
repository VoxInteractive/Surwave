#pragma once

#include "src/flecs_registry.h"

#include "src/components/physics.h"
#include "src/components/transform.h"

inline FlecsRegistry register_velocity_to_position_system([](flecs::world& world) {
    world.system<
        Position2D,
        const Velocity2D>("Velocity to Position")
        .kind(flecs::PostUpdate)
        .multi_threaded()
        .each([](flecs::iter& it, size_t i, Position2D& position, const Velocity2D& velocity) {
        position.value += velocity.value * it.delta_time();
    });
});
