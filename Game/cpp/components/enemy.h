#pragma once

#include "flecs_registry.h"

enum class EnemyState {
    IDLE,
    RUNNING,
    DYING,
    DEAD
};

inline FlecsRegistry register_enemy_components([](flecs::world& world) {
    world.component<EnemyState>()
        .constant("IDLE", EnemyState::IDLE)
        .constant("RUNNING", EnemyState::RUNNING)
        .constant("DYING", EnemyState::DYING)
        .constant("DEAD", EnemyState::DEAD);
});
