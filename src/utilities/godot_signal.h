#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "src/flecs_registry.h"

struct GodotSignal {
    godot::StringName name;
    godot::Dictionary data;
};

inline FlecsRegistry register_godot_signal_component([](flecs::world& world)
{
    world.component<GodotSignal>("GodotSignal");
});

// Helper function to emit Godot signals from Flecs systems safely
inline void emit_godot_signal(const flecs::world& world, flecs::entity source_entity, const godot::StringName& name, const godot::Dictionary& data = godot::Dictionary()) {
    GodotSignal signal{ name, data };
    // Defer the emission to ensure it happens at a safe synchronization point (main thread usually)
    world.defer([source_entity, signal]() {
        source_entity.world().event<GodotSignal>()
            .entity(source_entity)
            .ctx(signal)
            .emit();
    });
}
