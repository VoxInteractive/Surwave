#pragma once

#include <limits>

#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"

inline constexpr float kEnemyDeathInvulnerableHitPoints = std::numeric_limits<float>::max();

struct HitPoints {
    float value;
};

struct HitRadius {
    float value;
};

struct MovementSpeed {
    float value;
};

struct MeleeDamage {
    float value;
};

struct AnimationFrameOffset {
    float value; // Even though this is an integer value, using float to avoid casting when used in shaders
};

struct AnimationRandomOffset {
    float value;
};

struct DeathTimer {
    float value;
};

struct HFlipTimer {
    float value;
};

struct VFlipTimer {
    float value;
};

// struct IsDying {};


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<HitPoints>("HitPoints")
        .member<float>("value");

    world.component<HitRadius>("HitRadius")
        .member<float>("value");

    world.component<MeleeDamage>("MeleeDamage")
        .member<float>("value");

    world.component<MovementSpeed>("MovementSpeed")
        .member<float>("value");

    world.component<AnimationFrameOffset>("AnimationFrameOffset")
        .member<float>("value");

    world.component<AnimationRandomOffset>("AnimationRandomOffset")
        .member<float>("value");

    world.component<DeathTimer>("DeathTimer")
        .member<float>("value");

    world.component<HFlipTimer>("HFlipTimer")
        .member<float>("value");

    world.component<VFlipTimer>("VFlipTimer")
        .member<float>("value");
});
