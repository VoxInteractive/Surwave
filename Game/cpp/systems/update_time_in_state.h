#pragma once

#include "src/flecs_registry.h"

#include "components/enemy_state.h"

inline FlecsRegistry register_update_time_in_state_system([](flecs::world& world) {
    world.system<TimeInState>("Update Time In State")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::PreUpdate)
        .multi_threaded()
        .each([](flecs::iter& it, size_t i, TimeInState& time) {
        time.value += it.delta_time();
    });
});
