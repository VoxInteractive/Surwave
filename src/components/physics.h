#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/shape2d.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"


struct Velocity2D
{
    godot::Vector2 value;
};

struct Velocity3D
{
    godot::Vector3 value;
};

struct PhysicsBodyShape2DDefinition
{
    godot::Ref<godot::Shape2D> shape;
    godot::Transform2D local_transform;
};

struct PhysicsBodyShapes2D
{
    std::vector<PhysicsBodyShape2DDefinition> shapes;
    godot::PhysicsServer2D::BodyMode body_mode = godot::PhysicsServer2D::BODY_MODE_RIGID;
    int64_t collision_layer = 1;
    int64_t collision_mask = 1;
};

struct PhysicsBodyInstance2D
{
    godot::RID body_rid;
};

struct PhysicsSpace2D
{
    godot::RID space_rid;

    operator godot::Variant() const { return space_rid; }
};

struct PhysicsBodyShape3DDefinition
{
    godot::Ref<godot::Shape3D> shape;
    godot::Transform3D local_transform;
};

struct PhysicsBodyShapes3D
{
    std::vector<PhysicsBodyShape3DDefinition> shapes;
    godot::PhysicsServer3D::BodyMode body_mode = godot::PhysicsServer3D::BODY_MODE_RIGID;
    int64_t collision_layer = 1;
    int64_t collision_mask = 1;
};

struct PhysicsBodyInstance3D
{
    godot::RID body_rid;
};

struct PhysicsSpace3D
{
    godot::RID space_rid;

    operator godot::Variant() const { return space_rid; }
};

inline FlecsRegistry register_physics_components([](flecs::world& world) {
    world.component<Velocity2D>("Velocity2D")
        .member<godot::Vector2>("value");

    world.component<Velocity3D>("Velocity3D")
        .member<godot::Vector3>("value");

    world.component<PhysicsBodyShapes2D>("PhysicsBodyShapes2D");
    world.component<PhysicsBodyInstance2D>("PhysicsBodyInstance2D");
    world.component<PhysicsSpace2D>("PhysicsSpace2D").add(flecs::Singleton);
    world.component<PhysicsBodyShapes3D>("PhysicsBodyShapes3D");
    world.component<PhysicsBodyInstance3D>("PhysicsBodyInstance3D");
    world.component<PhysicsSpace3D>("PhysicsSpace3D").add(flecs::Singleton);

    register_singleton_getter<PhysicsSpace2D>("PhysicsSpace2D");
    register_singleton_setter<godot::RID>("PhysicsSpace2D", [](flecs::world& world, const godot::RID& space_rid) {
        world.set<PhysicsSpace2D>({ space_rid });
    });
    register_singleton_getter<PhysicsSpace3D>("PhysicsSpace3D");
    register_singleton_setter<godot::RID>("PhysicsSpace3D", [](flecs::world& world, const godot::RID& space_rid) {
        world.set<PhysicsSpace3D>({ space_rid });
    });
});
