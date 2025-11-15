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


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<PlayerDetectionRadius>()
        .member<float>("value");

    world.component<HitPoints>()
        .member<float>("value");

    world.component<MovementSpeed>()
        .member<float>("value");

    world.component<MeleeDamage>()
        .member<float>("value");
});
