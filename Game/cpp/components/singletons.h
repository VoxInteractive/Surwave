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
        .set<EnemyBoidForceWeights>({
            0.8f,   // alignment_weight
            0.45f,  // cohesion_weight
            1.25f   // separation_weight
            });

    world.component<EnemyBoidMovementSettings>("EnemyBoidMovementSettings")
        .member<float>("player_attraction_weight")
        .member<float>("player_engage_distance")
        .member<float>("neighbor_radius")
        .member<float>("separation_radius")
        .member<float>("max_speed_multiplier")
        .member<float>("max_force")
        .member<float>("grid_cell_size")
        .add(flecs::Singleton)
        .set<EnemyBoidMovementSettings>({
            1.6f,   // player_attraction_weight
            28.0f,  // player_engage_distance
            110.0f, // neighbor_radius
            40.0f,  // separation_radius
            1.1f,   // max_speed_multiplier
            320.0f, // max_force
            96.0f   // grid_cell_size
            });

    world.component<EnemyAnimationSettings>("EnemyAnimationSettings")
        .member<float>("animation_interval")
        .member<float>("walk_animation_range")
        .member<float>("death_animation_frame_count")
        .member<float>("up_direction_frame_offset")
        .add(flecs::Singleton)
        .set<EnemyAnimationSettings>({
            0.25f, // animation_interval
            5.0f,  // walk_animation_range
            4.0f,  // death_animation_frame_count
            6.0f   // up_direction_frame_offset
            });


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
