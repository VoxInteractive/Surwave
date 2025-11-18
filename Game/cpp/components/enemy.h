#pragma once

#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"


struct HitPoints {
    godot::real_t value;
};

struct MovementSpeed {
    godot::real_t value;
};

struct MeleeDamage {
    godot::real_t value;
};

struct PlayerDetectionRadiusSquared {
    godot::real_t value;
};

struct LoseTargetRadiusSquared {
    godot::real_t value;
};

struct WanderMoveDuration {
    godot::real_t value;
};

struct RestDuration {
    godot::real_t value;
};


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<HitPoints>("HitPoints")
        .member<godot::real_t>("value");

    world.component<MovementSpeed>("MovementSpeed")
        .member<godot::real_t>("value");

    world.component<MeleeDamage>("MeleeDamage")
        .member<godot::real_t>("value");

    world.component<PlayerDetectionRadiusSquared>("PlayerDetectionRadiusSquared")
        .member<godot::real_t>("value");

    world.component<LoseTargetRadiusSquared>("LoseTargetRadiusSquared")
        .member<godot::real_t>("value");

    world.component<WanderMoveDuration>("WanderMoveDuration")
        .member<godot::real_t>("value");

    world.component<RestDuration>("RestDuration")
        .member<godot::real_t>("value");
});
