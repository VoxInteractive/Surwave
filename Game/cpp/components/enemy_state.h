#pragma once

#include "src/flecs_registry.h"

#include <godot_cpp/variant/vector2.hpp>


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
    if (!entity.has<EnemyState, T>()) {
        entity.add<EnemyState, T>();
        entity.set<TimeInState>({ 0.0f });
    }
}

inline FlecsRegistry register_enemy_state_components([](flecs::world& world) {
    // State is an exclusive relationship. The target must be one of its children.
    world.component<EnemyState>()
        .add(flecs::Exclusive)
        .add(flecs::OneOf)
        .scope([&] {
        // Register all states as children of the State relationship
        world.component<EnemyState::Attacking>();
        world.component<EnemyState::Chasing>();
        world.component<EnemyState::Dead>();
        world.component<EnemyState::Dying>();
        world.component<EnemyState::Idle>();
        world.component<EnemyState::Wandering>();
    });

    world.component<TimeInState>("TimeInState")
        .member<float>("value");
});
