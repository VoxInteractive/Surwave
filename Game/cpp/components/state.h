#pragma once

#include <flecs.h>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include "src/flecs_registry.h"

struct State {
    struct Attacking { flecs::entity target; };
    struct Chasing { flecs::entity target; };
    struct Dead {};
    struct Dying {};
    struct Idle {};
};

// Helper function for tag-like state transitions.
template<typename T>
void set_state(const flecs::entity& entity) {
    entity.add<State, T>();
}

// Overloaded helper for state transitions that require a target.
template<typename T>
void set_state(const flecs::entity& entity, flecs::entity target) {
    entity.set<State, T>({ target });
}

inline FlecsRegistry register_state_entities([](flecs::world& world) {
    // State is an exclusive relationship. The target must be one of its children.
    world.component<State>().add(flecs::Exclusive).add(flecs::OneOf);


    world.component<State::Attacking>()
        .member<flecs::entity>("target")
        .child_of<State>();

    world.component<State::Chasing>()
        .member<flecs::entity>("target")
        .child_of<State>();

    world.component<State::Dead>().child_of<State>();

    world.component<State::Dying>().child_of<State>();

    world.component<State::Idle>().child_of<State>();
});
