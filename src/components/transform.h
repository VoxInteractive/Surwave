#pragma once

#include "src/flecs_registry.h"

struct Position2D {
    float x, y;
};

struct Position3D {
    float x, y, z;
};


struct Rotation2D {
    float value;
};

struct Rotation3D {
    float x, y, z; // Euler
};


struct Scale2D {
    float x, y;
};

struct Scale3D {
    float x, y, z;
};


inline FlecsRegistry register_transform_components([](flecs::world& world) {

    world.component<Position2D>("Position2D")
        .member<float>("x")
        .member<float>("y");

    world.component<Position3D>("Position3D")
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");


    world.component<Rotation2D>("Rotation2D")
        .member<float>("value");

    world.component<Rotation3D>("Rotation3D")
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");


    world.component<Scale2D>("Scale2D")
        .member<float>("x")
        .member<float>("y");

    world.component<Scale3D>("Scale3D")
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");

});
