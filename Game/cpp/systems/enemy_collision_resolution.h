#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/physics.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_collision_resolution_system([](flecs::world& world) {
    world.system<Velocity2D, const Position2D, const RadiusSquared>("Enemy Collision Resolution")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::OnValidate)
        .each([](
            flecs::entity entity,
            Velocity2D& velocity,
            const Position2D& position,
            const RadiusSquared& radius_sq) {

        // TODO: Implement enemy collision resolution logic here
    });
});
