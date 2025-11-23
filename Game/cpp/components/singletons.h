#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"

struct EnemyBoidForceWeights {
    float alignment_weight;
    float cohesion_weight;
    float separation_weight;
};

struct EnemyBoidMovementSettings {
    float player_attraction_weight;
    float player_engage_distance;
    float neighbor_radius;
    float separation_radius;
    float max_speed_multiplier;
    float max_force;
    float grid_cell_size;
};

struct EnemyAnimationSettings {
    float animation_interval;
    float walk_animation_range;
    float death_animation_frame_count;
    float up_direction_frame_offset;
};

struct ProjectileData {
    godot::Dictionary value;
};

struct ShockwaveData {
    godot::Dictionary value;
};


inline FlecsRegistry register_game_singleton_components([](flecs::world& world) {
    world.component<EnemyBoidForceWeights>("EnemyBoidForceWeights")
        .member<float>("alignment_weight")
        .member<float>("cohesion_weight")
        .member<float>("separation_weight")
        .add(flecs::Singleton)
        .set<EnemyBoidForceWeights>({ 0.8f, 0.45f, 1.25f });

    world.component<EnemyBoidMovementSettings>("EnemyBoidMovementSettings")
        .member<float>("player_attraction_weight")
        .member<float>("player_engage_distance")
        .member<float>("neighbor_radius")
        .member<float>("separation_radius")
        .member<float>("max_speed_multiplier")
        .member<float>("max_force")
        .member<float>("grid_cell_size")
        .add(flecs::Singleton)
        .set<EnemyBoidMovementSettings>({ 1.6f, 28.0f, 110.0f, 40.0f, 1.1f, 320.0f, 96.0f });

    world.component<EnemyAnimationSettings>("EnemyAnimationSettings")
        .member<float>("animation_interval")
        .member<float>("walk_animation_range")
        .member<float>("death_animation_frame_count")
        .member<float>("up_direction_frame_offset")
        .add(flecs::Singleton)
        .set<EnemyAnimationSettings>({ 0.25f, 5.0f, 4.0f, 6.0f });


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
