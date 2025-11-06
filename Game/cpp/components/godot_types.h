#pragma once

#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include <flecs.h>

#include "../../../src/flecs_registry.h"


inline FlecsRegistry register_godot_type_components([](flecs::world& world)
{
    world.component<godot::Color>("Color") // 16 bytes
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");

    world.component<godot::Vector2>("Vector2") // 8 bytes
        .member<float>("x")
        .member<float>("y");

    world.component<godot::Vector2i>("Vector2i") // 8 bytes
        .member<int32_t>("x")
        .member<int32_t>("y");

    world.component<godot::Vector3>("Vector3") // 12 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");

    world.component<godot::Vector3i>("Vector3i") // 12 bytes
        .member<int32_t>("x")
        .member<int32_t>("y")
        .member<int32_t>("z");

    world.component<godot::Vector4>("Vector4") // 16 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z")
        .member<float>("w");

    world.component<godot::Vector4i>("Vector4i") // 16 bytes
        .member<int32_t>("x")
        .member<int32_t>("y")
        .member<int32_t>("z")
        .member<int32_t>("w");

    world.component<godot::Rect2>("Rect2") // 16 bytes
        .member<godot::Vector2>("position")
        .member<godot::Vector2>("size");

    world.component<godot::Rect2i>("Rect2i") // 16 bytes
        .member<godot::Vector2i>("position")
        .member<godot::Vector2i>("size");

    world.component<godot::Plane>("Plane") // 16 bytes
        .member<godot::Vector3>("normal")
        .member<float>("d");

    world.component<godot::Quaternion>("Quaternion") // 16 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z")
        .member<float>("w");

    world.component<godot::Basis>("Basis") // 36 bytes - acceptable
        .member<godot::Vector3>("rows", 3);

    world.component<godot::Transform2D>("Transform2D") // 24 bytes
        .member<godot::Vector2>("columns", 3);

    world.component<godot::Transform3D>("Transform3D") // 48 bytes - borderline large, but acceptable for transform components
        .member<godot::Basis>("basis")
        .member<godot::Vector3>("origin");

    world.component<godot::AABB>("AABB") // 24 bytes
        .member<godot::Vector3>("position")
        .member<godot::Vector3>("size");

    world.component<godot::Projection>("Projection") // 64 bytes - large, use sparingly
        .member<godot::Vector4>("columns", 4);
});
