#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"


enum class EnemyState {
    IDLE,
    WANDERING_MOVING,
    WANDERING_RESTING,
    CHASING,
    ATTACKING,
    DYING,
    DEAD
};

enum class EnemyAnimationState {
    IDLE,
    RUNNING,
    DYING,
};

struct TimeInState {
    float value;
};

struct WanderDirection {
    godot::Vector2 value;
};

inline FlecsRegistry register_enemy_state_components([](flecs::world& world) {
    world.component<EnemyState>()
        .constant("IDLE", EnemyState::IDLE)
        .constant("WANDERING_MOVING", EnemyState::WANDERING_MOVING)
        .constant("WANDERING_RESTING", EnemyState::WANDERING_RESTING)
        .constant("CHASING", EnemyState::CHASING)
        .constant("ATTACKING", EnemyState::ATTACKING)
        .constant("DYING", EnemyState::DYING)
        .constant("DEAD", EnemyState::DEAD);

    world.component<EnemyAnimationState>()
        .constant("IDLE", EnemyAnimationState::IDLE)
        .constant("RUNNING", EnemyAnimationState::RUNNING)
        .constant("DYING", EnemyAnimationState::DYING);

    world.component<TimeInState>()
        .member<float>("value");

    world.component<WanderDirection>()
        .member<godot::Vector2>("value");
});
