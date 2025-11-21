#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "src/flecs_registry.h"
#include "src/components/physics.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_collision_resolution_system([](flecs::world& world) {
    const flecs::entity enemy_prefab = world.lookup("Enemy");
    if (!enemy_prefab) {
        godot::UtilityFunctions::push_error("Enemy Collision Resolution: prefab 'Enemy' not found; system not registered");
        return;
    }

    const auto* radius_squared_component = enemy_prefab.try_get<RadiusSquared>();
    if (!radius_squared_component) {
        godot::UtilityFunctions::push_error("Enemy Collision Resolution: prefab 'Enemy' missing RadiusSquared component; collision resolution disabled");
        return;
    }
    const godot::real_t radius_squared = radius_squared_component->value;

    world.system<Velocity2D, const Position2D>("Enemy Collision Resolution")
        .with(flecs::IsA, enemy_prefab)
        .kind(flecs::OnValidate)
        .run([&world, enemy_prefab, radius_squared](flecs::iter& it) {
        // TODO: Implement enemy collision resolution logic here.
    });
});
