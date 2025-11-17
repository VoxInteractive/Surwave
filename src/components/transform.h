#pragma once

#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"

struct Position2D {
    godot::real_t x, y;
};

struct Position3D {
    godot::real_t x, y, z;
};


struct Rotation2D {
    godot::real_t value;
};

struct Rotation3D {
    godot::real_t x, y, z; // Euler
};


struct Scale2D {
    godot::real_t x, y;
};

struct Scale3D {
    godot::real_t x, y, z;
};


inline FlecsRegistry register_transform_components([](flecs::world& world) {

    world.component<Position2D>("Position2D")
        .member<godot::real_t>("x")
        .member<godot::real_t>("y");

    world.component<Position3D>("Position3D")
        .member<godot::real_t>("x")
        .member<godot::real_t>("y")
        .member<godot::real_t>("z");


    world.component<Rotation2D>("Rotation2D")
        .member<godot::real_t>("value");

    world.component<Rotation3D>("Rotation3D")
        .member<godot::real_t>("x")
        .member<godot::real_t>("y")
        .member<godot::real_t>("z");


    world.component<Scale2D>("Scale2D")
        .member<godot::real_t>("x")
        .member<godot::real_t>("y");

    world.component<Scale3D>("Scale3D")
        .member<godot::real_t>("x")
        .member<godot::real_t>("y")
        .member<godot::real_t>("z");

});
