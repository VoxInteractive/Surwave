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
    godot::real_t walk_animation_range;
    godot::real_t death_animation_frame_count;
    godot::real_t up_direction_frame_offset;
    godot::real_t horizontal_flip_cooldown;
    godot::real_t vertical_flip_cooldown;
    godot::real_t nominal_movement_speed;
    godot::real_t animation_offset_fraction_range;
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
        .set<EnemyBoidForceWeights>({
            godot::real_t(0.8),   // alignment_weight
            godot::real_t(0.45),  // cohesion_weight
            godot::real_t(0.8)   // separation_weight
            });

    world.component<EnemyBoidMovementSettings>("EnemyBoidMovementSettings")
        .member<godot::real_t>("player_attraction_weight")
        .member<godot::real_t>("player_engage_distance")
        .member<godot::real_t>("neighbor_radius")
        .member<godot::real_t>("separation_radius")
        .member<godot::real_t>("max_speed_multiplier")
        .member<godot::real_t>("max_force")
        .member<godot::real_t>("grid_cell_size")
        .add(flecs::Singleton)
        .set<EnemyBoidMovementSettings>({
            godot::real_t(1.2),   // player_attraction_weight
            godot::real_t(28.0),  // player_engage_distance
            godot::real_t(110.0), // neighbor_radius
            godot::real_t(40.0),  // separation_radius
            godot::real_t(1.1),   // max_speed_multiplier
            godot::real_t(220.0), // max_force
            godot::real_t(96.0)   // grid_cell_size
            });

    world.component<EnemyAnimationSettings>("EnemyAnimationSettings")
        .member<godot::real_t>("animation_interval")
        .member<godot::real_t>("walk_animation_range")
        .member<godot::real_t>("death_animation_frame_count")
        .member<godot::real_t>("up_direction_frame_offset")
        .member<godot::real_t>("horizontal_flip_cooldown")
        .member<godot::real_t>("vertical_flip_cooldown")
        .member<godot::real_t>("nominal_movement_speed")
        .member<godot::real_t>("animation_offset_fraction_range")
        .add(flecs::Singleton)
        .set<EnemyAnimationSettings>({
            godot::real_t(0.25), // animation_interval
            godot::real_t(5.0),  // walk_animation_range
            godot::real_t(4.0),  // death_animation_frame_count
            godot::real_t(6.0),  // up_direction_frame_offset
            godot::real_t(0.5),  // horizontal_flip_cooldown
            godot::real_t(0.5),  // vertical_flip_cooldown
            godot::real_t(8.0),  // nominal_movement_speed
            godot::real_t(0.2)   // animation_offset_fraction_range
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
