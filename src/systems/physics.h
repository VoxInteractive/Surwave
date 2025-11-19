#pragma once

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include "src/components/physics.h"
#include "src/flecs_registry.h"

namespace
{
    template<typename TransformT, typename InstanceT, typename ServerT, typename BodyStateT>
    inline void register_physics_sync_system(
        flecs::world& world,
        const char* system_name,
        BodyStateT body_state)
    {
        world.system<const TransformT, const InstanceT>(system_name)
            .kind(flecs::PostUpdate)
            .each([body_state](const TransformT& transform, const InstanceT& instance)
        {
            ServerT* physics_server = ServerT::get_singleton();
            if (!physics_server) { return; }
            if (!instance.body_rid.is_valid()) { return; }
            physics_server->body_set_state(instance.body_rid, body_state, transform);
        });
    }

    template<typename InstanceT, typename ServerT>
    inline void register_physics_cleanup_observer(
        flecs::world& world,
        const char* observer_name)
    {
        world.observer<const InstanceT>(observer_name)
            .event(flecs::OnRemove)
            .each([](const InstanceT& instance)
        {
            ServerT* physics_server = ServerT::get_singleton();
            if (!physics_server) { return; }
            if (!instance.body_rid.is_valid()) { return; }
            physics_server->free_rid(instance.body_rid);
        });
    }
}

inline FlecsRegistry register_physics_systems([](flecs::world& world)
{
    register_physics_sync_system<godot::Transform2D, PhysicsBodyInstance2D, godot::PhysicsServer2D>(
        world,
        "Physics Body 2D Sync",
        godot::PhysicsServer2D::BODY_STATE_TRANSFORM);
    register_physics_cleanup_observer<PhysicsBodyInstance2D, godot::PhysicsServer2D>(
        world,
        "Physics Body 2D Cleanup");

    register_physics_sync_system<godot::Transform3D, PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Sync",
        godot::PhysicsServer3D::BODY_STATE_TRANSFORM);
    register_physics_cleanup_observer<PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Cleanup");
});
