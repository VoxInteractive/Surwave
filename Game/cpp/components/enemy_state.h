#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"


struct EnemyState {
    struct Attacking {
        flecs::entity value; // target
    };
    struct Chasing {
        godot::Vector2 value; // target
    };
    struct Dead {};
    struct Dying {};
    struct Idle {};
    struct Wandering {
        godot::Vector2 value; // destination
    };
};

struct TimeInState {
    godot::real_t value;
};

// Helper function for state transitions.
template<typename T, typename... Args>
void set_state(const flecs::entity& entity, Args&&... args) {
    if (entity.has<EnemyState, T>()) {
        return; // Already in the requested state
    }

    if constexpr (sizeof...(args) > 0) { // Check if any arguments (e.g. a StateSubject) were provided
        entity.set<EnemyState, T>({ std::forward<Args>(args)... }); // Set the state component with the given data. This is how a state like Wandering can have its target position updated
    }
    else {
        entity.add<EnemyState, T>(); // If not, fall back to entity.add<EnemyState, T>(), which is the correct way to add a tag-like state that has no data (like Idle)
    }

    entity.set<TimeInState>({ 0.0f });
}

inline FlecsRegistry register_enemy_state_components([](flecs::world& world) {
    // State is an exclusive relationship. The target must be one of its children.
    world.component<EnemyState>()
        .add(flecs::Exclusive)
        .add(flecs::OneOf)
        .scope([&] {
        // Register all states as children of the State relationship
        world.component<EnemyState::Attacking>()
            .member<flecs::entity>("target")
            .add(flecs::Target);
        world.component<EnemyState::Chasing>()
            .member<godot::Vector2>("target")
            .add(flecs::Target);

        world.component<EnemyState::Dead>()
            .add(flecs::Target);

        world.component<EnemyState::Dying>()
            .add(flecs::Target);

        world.component<EnemyState::Idle>()
            .add(flecs::Target);

        world.component<EnemyState::Wandering>()
            .member<godot::Vector2>("destination")
            .add(flecs::Target);
    });

    world.component<TimeInState>("TimeInState")
        .member<godot::real_t>("value");
});
