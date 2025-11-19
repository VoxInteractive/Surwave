#pragma once

#include <godot_cpp/variant/dictionary.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"

struct StageData
{
    godot::Dictionary value;

    // This will make it implicitly convertible to a godot::Dictionary, which is a type GDScript understands.
    // When get_singleton_component returns the StageData object, this conversion will be used automatically to create a godot::Variant.
    operator godot::Variant() const {
        return value;
    }
};

inline FlecsRegistry register_stage_components([](flecs::world& world) {
    world.component<StageData>("StageData").add(flecs::Singleton);

    register_singleton_setter<godot::Dictionary>("StageData", [](flecs::world& world, const godot::Dictionary& stage_data) {
        world.set<StageData>({ stage_data });
    });

    register_singleton_getter<StageData>("StageData");
});
