#pragma once

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/components/physics.h"
#include "src/components/transform.h"
#include "src/flecs_registry.h"

namespace
{
    inline bool try_create_physics_body_2d(
        flecs::world& world,
        flecs::entity entity,
        const PhysicsBodyShapes2D& body_definition,
        const godot::Transform2D& transform)
    {
        godot::PhysicsServer2D* physics_server = godot::PhysicsServer2D::get_singleton();
        const PhysicsSpace2D* physics_space = world.try_get<PhysicsSpace2D>();
        if (!physics_server) { return false; }
        if (!physics_space) { return false; }
        if (!physics_space->space_rid.is_valid()) { return false; }
        if (body_definition.shapes.empty()) { return false; }

        godot::RID body_rid = physics_server->body_create();
        physics_server->body_set_mode(body_rid, body_definition.body_mode);
        physics_server->body_set_space(body_rid, physics_space->space_rid);
        physics_server->body_set_collision_layer(body_rid, body_definition.collision_layer);
        physics_server->body_set_collision_mask(body_rid, body_definition.collision_mask);

        int added_shapes = 0;
        for (const PhysicsBodyShape2DDefinition& shape_def : body_definition.shapes)
        {
            if (shape_def.shape.is_null()) { continue; }
            physics_server->body_add_shape(body_rid, shape_def.shape->get_rid(), shape_def.local_transform);
            added_shapes++;
        }

        if (added_shapes == 0)
        {
            physics_server->free_rid(body_rid);
            return false;
        }

        physics_server->body_set_state(body_rid, godot::PhysicsServer2D::BODY_STATE_TRANSFORM, transform);
        entity.set<PhysicsBodyInstance2D>({ body_rid });
        return true;
    }

    inline bool try_create_physics_body_3d(
        flecs::world& world,
        flecs::entity entity,
        const PhysicsBodyShapes3D& body_definition,
        const godot::Transform3D& transform)
    {
        godot::PhysicsServer3D* physics_server = godot::PhysicsServer3D::get_singleton();
        const PhysicsSpace3D* physics_space = world.try_get<PhysicsSpace3D>();
        if (!physics_server) { return false; }
        if (!physics_space) { return false; }
        if (!physics_space->space_rid.is_valid()) { return false; }
        if (body_definition.shapes.empty()) { return false; }

        godot::RID body_rid = physics_server->body_create();
        physics_server->body_set_mode(body_rid, body_definition.body_mode);
        physics_server->body_set_space(body_rid, physics_space->space_rid);
        physics_server->body_set_collision_layer(body_rid, body_definition.collision_layer);
        physics_server->body_set_collision_mask(body_rid, body_definition.collision_mask);

        int added_shapes = 0;
        for (const PhysicsBodyShape3DDefinition& shape_def : body_definition.shapes)
        {
            if (shape_def.shape.is_null()) { continue; }
            physics_server->body_add_shape(body_rid, shape_def.shape->get_rid(), shape_def.local_transform);
            added_shapes++;
        }

        if (added_shapes == 0)
        {
            physics_server->free_rid(body_rid);
            return false;
        }

        physics_server->body_set_state(body_rid, godot::PhysicsServer3D::BODY_STATE_TRANSFORM, transform);
        entity.set<PhysicsBodyInstance3D>({ body_rid });
        return true;
    }

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

    world.system<const PhysicsBodyShapes2D>("Physics Body 2D Instantiate")
        .kind(flecs::OnUpdate)
        .without<PhysicsBodyInstance2D>()
        .each([](flecs::iter& it, size_t i, const PhysicsBodyShapes2D& body_shapes)
    {
        flecs::entity entity = it.entity(i);
        const godot::Transform2D* transform = entity.try_get<godot::Transform2D>();
        godot::Transform2D initial_transform = transform ? *transform : godot::Transform2D();
        try_create_physics_body_2d(it.world(), entity, body_shapes, initial_transform);
    });

    world.system<const Velocity2D, const PhysicsBodyInstance2D>("Physics Body 2D Apply Velocity")
        .kind(flecs::PostUpdate)
        .each([](const Velocity2D& velocity, const PhysicsBodyInstance2D& instance)
    {
        godot::PhysicsServer2D* physics_server = godot::PhysicsServer2D::get_singleton();
        if (!physics_server) { return; }
        if (!instance.body_rid.is_valid()) { return; }
        physics_server->body_set_state(instance.body_rid, godot::PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY, velocity.value);
    });

    world.system<
        PhysicsBodyInstance2D,
        Position2D,
        godot::Transform2D,
        Rotation2D*,
        Scale2D*,
        Velocity2D*>("Physics Body 2D Feedback")
        .kind(flecs::PreUpdate)
        .term_at(4).optional()
        .term_at(5).optional()
        .term_at(6).optional()
        .each([](
            flecs::iter& it,
            size_t i,
            PhysicsBodyInstance2D& instance,
            Position2D& position,
            godot::Transform2D& transform_component,
            Rotation2D* rotation,
            Scale2D* scale,
            Velocity2D* velocity)
    {
        godot::PhysicsServer2D* physics_server = godot::PhysicsServer2D::get_singleton();
        if (!physics_server) { return; }
        if (!instance.body_rid.is_valid()) { return; }

        godot::Variant transform_state = physics_server->body_get_state(
            instance.body_rid,
            godot::PhysicsServer2D::BODY_STATE_TRANSFORM);
        if (transform_state.get_type() == godot::Variant::TRANSFORM2D)
        {
            godot::Transform2D physics_transform = transform_state;
            transform_component = physics_transform;
            position.value = physics_transform.get_origin();
            if (rotation) { rotation->value = physics_transform.get_rotation(); }
            if (scale) { scale->value = physics_transform.get_scale(); }
        }

        if (velocity)
        {
            godot::Variant velocity_state = physics_server->body_get_state(
                instance.body_rid,
                godot::PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY);
            if (velocity_state.get_type() == godot::Variant::VECTOR2)
            {
                godot::Vector2 physics_velocity = velocity_state;
                velocity->value = physics_velocity;
            }
        }
    });

    register_physics_sync_system<godot::Transform3D, PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Sync",
        godot::PhysicsServer3D::BODY_STATE_TRANSFORM);
    register_physics_cleanup_observer<PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Cleanup");

    world.system<const PhysicsBodyShapes3D>("Physics Body 3D Instantiate")
        .kind(flecs::OnUpdate)
        .without<PhysicsBodyInstance3D>()
        .each([](flecs::iter& it, size_t i, const PhysicsBodyShapes3D& body_shapes)
    {
        flecs::entity entity = it.entity(i);
        const godot::Transform3D* transform = entity.try_get<godot::Transform3D>();
        godot::Transform3D initial_transform = transform ? *transform : godot::Transform3D();
        try_create_physics_body_3d(it.world(), entity, body_shapes, initial_transform);
    });
});
