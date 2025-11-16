#pragma once

#include <flecs.h>

struct Target {
    flecs::entity entity;
};

struct State {
    struct Attacking {};
    struct Chasing {};
    struct Dead {};
    struct Dying {};
    struct Idle {};
    struct TravelingTo {};
};

struct TimeInState {
    float value;
};

// Helper function for tag-like state transitions.
template<typename T>
void set_state(const flecs::entity& entity) {
    // Ensure previous state's target data is removed if it exists
    entity.remove<Target, State>();
    entity.add<State, T>();
}

// Overloaded helper for state transitions that require an entity target
template<typename T>
void set_state(const flecs::entity& entity, flecs::entity target) {
    entity.add<State, T>()
        .set<Target, State>({ target });
}

inline FlecsRegistry register_state_components(flecs::world& world) {
    // State is an exclusive relationship. The target must be one of its children.
    world.component<State>().add(flecs::Exclusive).add(flecs::OneOf);

    // Register all states as children of the State relationship
    world.component<State::Attacking>().child_of<State>();
    world.component<State::Chasing>().child_of<State>();
    world.component<State::Dead>().child_of<State>();
    world.component<State::Dying>().child_of<State>();
    world.component<State::Idle>().child_of<State>();
    world.component<State::TravelingTo>().child_of<State>();
}
