#pragma once

#include "src/flecs_registry.h"


enum class EnemyState {
    IDLE,
    WANDERING,
    CHASING,
    ATTACKING,
    DYING,
    DEAD
};

enum class EnemyAnimationState {
    RUNNING,
    DYING,
};


inline FlecsRegistry register_enemy_state_components([](flecs::world& world) {
    world.component<EnemyState>()
        .constant("IDLE", EnemyState::IDLE)
        .constant("WANDERING", EnemyState::WANDERING)
        .constant("CHASING", EnemyState::CHASING)
        .constant("ATTACKING", EnemyState::ATTACKING)
        .constant("DYING", EnemyState::DYING)
        .constant("DEAD", EnemyState::DEAD);

    world.component<EnemyAnimationState>()
        .constant("RUNNING", EnemyAnimationState::RUNNING)
        .constant("DYING", EnemyAnimationState::DYING);
});
