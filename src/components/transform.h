#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "src/flecs_registry.h"

struct Position2D {
    godot::Vector2 value;
};

struct Position3D {
    godot::Vector3 value;
};


struct Rotation2D {
    godot::real_t value;
};

struct Rotation3D {
    godot::Vector3 value; // Euler
};


struct Scale2D {
    godot::Vector2 value;
};

struct Scale3D {
    godot::Vector3 value;
};


inline FlecsRegistry register_transform_components([](flecs::world& world) {

    world.component<Position2D>("Position2D")
        .member<godot::Vector2>("value");

    world.component<Position3D>("Position3D")
        .member<godot::Vector3>("value");


    world.component<Rotation2D>("Rotation2D")
        .member<godot::real_t>("value");

    world.component<Rotation3D>("Rotation3D")
        .member<godot::Vector3>("value");


    world.component<Scale2D>("Scale2D")
        .member<godot::Vector2>("value");

    world.component<Scale3D>("Scale3D")
        .member<godot::Vector3>("value");

});
