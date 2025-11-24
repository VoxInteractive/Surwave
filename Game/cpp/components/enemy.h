#pragma once

#include <limits>

#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"

inline constexpr godot::real_t kEnemyDeathInvulnerableHitPoints = std::numeric_limits<godot::real_t>::max();

struct HitPoints {
    godot::real_t value;
};

struct HitRadius {
    godot::real_t value;
};

struct MovementSpeed {
    godot::real_t value;
};

struct MeleeDamage {
    godot::real_t value;
};

struct AnimationFrameOffset {
    godot::real_t value;
};

struct DeathTimer {
    godot::real_t value;
};

struct HitReactionTimer {
    godot::real_t value;
};

struct HFlipTimer {
    godot::real_t value;
};

struct VFlipTimer {
    godot::real_t value;
};

struct ProjectileHitTimeout {
    godot::real_t value;
};

struct ShockwaveHitTimeout {
    godot::real_t value;
};

// struct IsDying {};


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<HitPoints>("HitPoints")
        .member<godot::real_t>("value");

    world.component<HitRadius>("HitRadius")
        .member<godot::real_t>("value");

    world.component<MeleeDamage>("MeleeDamage")
        .member<godot::real_t>("value");

    world.component<MovementSpeed>("MovementSpeed")
        .member<godot::real_t>("value");

    world.component<AnimationFrameOffset>("AnimationFrameOffset")
        .member<godot::real_t>("value");

    world.component<DeathTimer>("DeathTimer")
        .member<godot::real_t>("value");

    world.component<HitReactionTimer>("HitReactionTimer")
        .member<godot::real_t>("value");

    world.component<HFlipTimer>("HFlipTimer")
        .member<godot::real_t>("value");

    world.component<VFlipTimer>("VFlipTimer")
        .member<godot::real_t>("value");

    world.component<ProjectileHitTimeout>("ProjectileHitTimeout")
        .member<godot::real_t>("value");

    world.component<ShockwaveHitTimeout>("ShockwaveHitTimeout")
        .member<godot::real_t>("value");
});
