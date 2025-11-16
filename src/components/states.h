#pragma once

#include <flecs.h>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include "src/flecs_registry.h"

struct State {
    struct Attacking { flecs::entity target; };
    struct Chasing { flecs::entity target; };
    struct TravelingTo {};
    struct Dead;
    struct Dying;
    struct Idle;
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

// Overloaded helper for TravelingTo state.
template<typename T>
void set_state(const flecs::entity& entity, const T& target_position) {
    entity.set_poly<State::TravelingTo, T>(target_position);
    entity.add<State, State::TravelingTo>();
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

    // Register TravelingTo as a polymorphic component and a child of State.
    world.component<State::TravelingTo>().add<flecs::Poly>().child_of<State>();
});
