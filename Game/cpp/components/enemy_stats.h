#pragma once

#include "src/flecs_registry.h"


struct HitPoints {
    float value;
};

struct MovementSpeed {
    float value;
};

struct MeleeDamage {
    float value;
};

struct PlayerDetectionRadius {
    float value;
};

struct WanderMoveDuration {
    float value;
};

struct WanderRestDuration {
    float value;
};


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<HitPoints>()
        .member<float>("value");

    world.component<MovementSpeed>()
        .member<float>("value");

    world.component<MeleeDamage>()
        .member<float>("value");

    world.component<PlayerDetectionRadius>()
        .member<float>("value");

    world.component<WanderMoveDuration>()
        .member<float>("value");

    world.component<WanderRestDuration>()
        .member<float>("value");
});
