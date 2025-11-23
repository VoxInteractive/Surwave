#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"

struct EnemyBoidForceWeights {
    godot::real_t alignment_weight;
    godot::real_t cohesion_weight;
    godot::real_t separation_weight;
};

struct EnemyBoidMovementSettings {
    godot::real_t player_attraction_weight;
    godot::real_t player_engage_distance;
    godot::real_t neighbor_radius;
    godot::real_t separation_radius;
    godot::real_t max_speed_multiplier;
    godot::real_t max_force;
    godot::real_t grid_cell_size;
};

struct EnemyAnimationSettings {
    godot::real_t animation_interval;
};

struct ProjectileData {
    godot::Dictionary value;
};

struct ShockwaveData {
    godot::Dictionary value;
};


inline FlecsRegistry register_game_singleton_components([](flecs::world& world) {
    world.component<EnemyBoidForceWeights>("EnemyBoidForceWeights")
        .member<godot::real_t>("alignment_weight")
        .member<godot::real_t>("cohesion_weight")
        .member<godot::real_t>("separation_weight")
        .add(flecs::Singleton)
        .set<EnemyBoidForceWeights>({ 0.8f, 0.45f, 1.25f });

    world.component<EnemyBoidMovementSettings>("EnemyBoidMovementSettings")
        .member<godot::real_t>("player_attraction_weight")
        .member<godot::real_t>("player_engage_distance")
        .member<godot::real_t>("neighbor_radius")
        .member<godot::real_t>("separation_radius")
        .member<godot::real_t>("max_speed_multiplier")
        .member<godot::real_t>("max_force")
        .member<godot::real_t>("grid_cell_size")
        .add(flecs::Singleton)
        .set<EnemyBoidMovementSettings>({ 1.6f, 28.0f, 110.0f, 40.0f, 1.1f, 320.0f, 96.0f });

    world.component<EnemyAnimationSettings>("EnemyAnimationSettings")
        .member<godot::real_t>("animation_interval")
        .add(flecs::Singleton)
        .set<EnemyAnimationSettings>({ 0.25f });


    world.component<ProjectileData>("ProjectileData")
        .add(flecs::Singleton);

    register_singleton_setter<godot::Dictionary>("ProjectileData", [](flecs::world& world, const godot::Dictionary& projectile_data) {
        world.set<ProjectileData>({ projectile_data });
    });


    world.component<ShockwaveData>("ShockwaveData")
        .add(flecs::Singleton);

    register_singleton_setter<godot::Dictionary>("ShockwaveData", [](flecs::world& world, const godot::Dictionary& shockwave_data) {
        world.set<ShockwaveData>({ shockwave_data });
    });
});
