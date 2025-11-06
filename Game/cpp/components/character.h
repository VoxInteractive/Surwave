#pragma once

#include <godot_cpp/variant/vector3.hpp>

#include <flecs.h>

#include "../../../src/flecs_registry.h"

struct LocalTransform
{
    godot::Vector3 position;
};

struct CharacterGroundingData
{
    float height_offset = 0.0f;
    float grounding_sharpness = 0.0f;
};


inline FlecsRegistry register_character_components([](flecs::world& world)
{
    world.component<LocalTransform>()
        .member<godot::Vector3>("position");

    world.component<CharacterGroundingData>()
        .member<float>("height_offset")
        .member<float>("grounding_sharpness");
});
