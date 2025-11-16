#pragma once

#include "src/flecs_registry.h"

struct Target {
    flecs::entity entity;
};

struct EnemyState {
    struct Attacking {};
    struct Chasing {};
    struct Dead {};
    struct Dying {};
    struct Idle {};
    struct Wandering {};
};

struct TimeInState {
    float value;
};

// Helper function for tag-like state transitions.
template<typename T>
void set_state(const flecs::entity& entity) {
    entity.add<EnemyState, T>();
}

// Overloaded helper for state transitions that require an entity target
template<typename T>
void set_state(const flecs::entity& entity, flecs::entity target) {
    entity.add<EnemyState, T>()
        .set<Target, EnemyState>({ target });
}

inline FlecsRegistry register_enemy_state_components([](flecs::world& world) {
    // State is an exclusive relationship. The target must be one of its children.
    world.component<EnemyState>().add(flecs::Exclusive).add(flecs::OneOf);

    // Register all states as children of the State relationship
    world.component<EnemyState::Attacking>().child_of<EnemyState>();
    world.component<EnemyState::Chasing>().child_of<EnemyState>();
    world.component<EnemyState::Dead>().child_of<EnemyState>();
    world.component<EnemyState::Dying>().child_of<EnemyState>();
    world.component<EnemyState::Idle>().child_of<EnemyState>();
    world.component<EnemyState::Wandering>().child_of<EnemyState>();

    world.component<TimeInState>("TimeInState")
        .member<float>("value");
});
