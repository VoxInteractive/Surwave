#pragma once

#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "src/components/physics.h"
#include "src/components/transform.h"
#include "src/flecs_registry.h"

namespace
{
    template<
        typename ShapesT,
        typename ShapeDefinitionT,
        typename SpaceT,
        typename ServerT,
        typename TransformT,
        typename InstanceT,
        typename BodyStateT>
    inline bool try_create_physics_body(
        flecs::world& world,
        flecs::entity entity,
        const ShapesT& body_definition,
        const TransformT& transform,
        BodyStateT transform_state)
    {
        ServerT* physics_server = ServerT::get_singleton();
        const SpaceT* physics_space = world.try_get<SpaceT>();
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
        for (const ShapeDefinitionT& shape_def : body_definition.shapes)
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

        physics_server->body_set_state(body_rid, transform_state, transform);
        entity.set<InstanceT>({ body_rid });
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

    template<
        typename ShapesT,
        typename ShapeDefinitionT,
        typename SpaceT,
        typename ServerT,
        typename TransformT,
        typename InstanceT,
        typename BodyStateT>
    inline void register_physics_instantiation_system(
        flecs::world& world,
        const char* system_name,
        BodyStateT transform_state)
    {
        world.system<const ShapesT>(system_name)
            .kind(flecs::OnUpdate)
            .without<InstanceT>()
            .each([transform_state](flecs::iter& it, size_t index, const ShapesT& body_shapes)
        {
            flecs::entity entity = it.entity(index);
            const TransformT* transform = entity.try_get<TransformT>();
            TransformT initial_transform = transform ? *transform : TransformT();
            try_create_physics_body<
                ShapesT,
                ShapeDefinitionT,
                SpaceT,
                ServerT,
                TransformT,
                InstanceT>(
                    it.world(),
                    entity,
                    body_shapes,
                    initial_transform,
                    transform_state);
        });
    }

    template<typename VelocityT, typename InstanceT, typename ServerT, typename BodyStateT>
    inline void register_physics_velocity_update_system(
        flecs::world& world,
        const char* system_name,
        BodyStateT velocity_state)
    {
        world.system<const VelocityT, const InstanceT>(system_name)
            .kind(flecs::PostUpdate)
            .each([velocity_state](const VelocityT& velocity, const InstanceT& instance)
        {
            ServerT* physics_server = ServerT::get_singleton();
            if (!physics_server) { return; }
            if (!instance.body_rid.is_valid()) { return; }
            physics_server->body_set_state(instance.body_rid, velocity_state, velocity.value);
        });
    }

    template<
        typename InstanceT,
        typename PositionT,
        typename TransformComponentT,
        typename RotationT,
        typename ScaleT,
        typename VelocityT,
        typename ServerT,
        typename BodyStateT,
        typename TransformAssignmentFn,
        typename VelocityAssignmentFn>
    inline void register_physics_feedback_system(
        flecs::world& world,
        const char* system_name,
        BodyStateT transform_state,
        BodyStateT velocity_state,
        godot::Variant::Type transform_variant_type,
        godot::Variant::Type velocity_variant_type,
        TransformAssignmentFn assign_transform,
        VelocityAssignmentFn assign_velocity)
    {
        world.system<
            InstanceT,
            PositionT,
            TransformComponentT,
            RotationT*,
            ScaleT*,
            VelocityT*>(system_name)
            .kind(flecs::PreUpdate)
            .term_at(4).optional()
            .term_at(5).optional()
            .term_at(6).optional()
            .each([transform_state,
                velocity_state,
                transform_variant_type,
                velocity_variant_type,
                assign_transform,
                assign_velocity](
                    flecs::iter& it,
                    size_t index,
                    InstanceT& instance,
                    PositionT& position,
                    TransformComponentT& transform_component,
                    RotationT* rotation,
                    ScaleT* scale,
                    VelocityT* velocity)
        {
            ServerT* physics_server = ServerT::get_singleton();
            if (!physics_server) { return; }
            if (!instance.body_rid.is_valid()) { return; }

            godot::Variant transform_value = physics_server->body_get_state(
                instance.body_rid,
                transform_state);
            if (transform_value.get_type() == transform_variant_type)
            {
                assign_transform(transform_value, position, transform_component, rotation, scale);
            }

            if (velocity)
            {
                godot::Variant velocity_value = physics_server->body_get_state(
                    instance.body_rid,
                    velocity_state);
                if (velocity_value.get_type() == velocity_variant_type)
                {
                    assign_velocity(velocity_value, *velocity);
                }
            }
        });
    }
}

inline FlecsRegistry register_physics_systems([](flecs::world& world)
{
    // 2D Physics Systems
    register_physics_instantiation_system<
        PhysicsBodyShapes2D,
        PhysicsBodyShape2DDefinition,
        PhysicsSpace2D,
        godot::PhysicsServer2D,
        godot::Transform2D,
        PhysicsBodyInstance2D>(
            world,
            "Physics Body 2D Instantiation",
            godot::PhysicsServer2D::BODY_STATE_TRANSFORM);

    register_physics_sync_system<godot::Transform2D, PhysicsBodyInstance2D, godot::PhysicsServer2D>(
        world,
        "Physics Body 2D Sync",
        godot::PhysicsServer2D::BODY_STATE_TRANSFORM);

    register_physics_velocity_update_system<
        Velocity2D,
        PhysicsBodyInstance2D,
        godot::PhysicsServer2D>(
            world,
            "Physics Body 2D Velocity Update",
            godot::PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY);

    register_physics_feedback_system<
        PhysicsBodyInstance2D,
        Position2D,
        godot::Transform2D,
        Rotation2D,
        Scale2D,
        Velocity2D,
        godot::PhysicsServer2D>(
            world,
            "Physics Body 2D Feedback",
            godot::PhysicsServer2D::BODY_STATE_TRANSFORM,
            godot::PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY,
            godot::Variant::TRANSFORM2D,
            godot::Variant::VECTOR2,
            [](const godot::Variant& transform_value,
                Position2D& position,
                godot::Transform2D& transform_component,
                Rotation2D* rotation,
                Scale2D* scale)
    {
        godot::Transform2D physics_transform = transform_value;
        transform_component = physics_transform;
        position.value = physics_transform.get_origin();
        if (rotation) { rotation->value = physics_transform.get_rotation(); }
        if (scale) { scale->value = physics_transform.get_scale(); }
    },
            [](const godot::Variant& velocity_value, Velocity2D& velocity_component)
    {
        godot::Vector2 physics_velocity = velocity_value;
        velocity_component.value = physics_velocity;
    });

    register_physics_cleanup_observer<PhysicsBodyInstance2D, godot::PhysicsServer2D>(
        world,
        "Physics Body 2D Cleanup");


    // 3D Physics Systems
    register_physics_instantiation_system <
        PhysicsBodyShapes3D,
        PhysicsBodyShape3DDefinition,
        PhysicsSpace3D,
        godot::PhysicsServer3D,
        godot::Transform3D,
        PhysicsBodyInstance3D>(
            world,
            "Physics Body 3D Instantiation",
            godot::PhysicsServer3D::BODY_STATE_TRANSFORM);

    register_physics_sync_system<godot::Transform3D, PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Sync",
        godot::PhysicsServer3D::BODY_STATE_TRANSFORM);

    register_physics_velocity_update_system<
        Velocity3D,
        PhysicsBodyInstance3D,
        godot::PhysicsServer3D>(
            world,
            "Physics Body 3D Velocity Update",
            godot::PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY);

    register_physics_feedback_system<
        PhysicsBodyInstance3D,
        Position3D,
        godot::Transform3D,
        Rotation3D,
        Scale3D,
        Velocity3D,
        godot::PhysicsServer3D>(
            world,
            "Physics Body 3D Feedback",
            godot::PhysicsServer3D::BODY_STATE_TRANSFORM,
            godot::PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY,
            godot::Variant::TRANSFORM3D,
            godot::Variant::VECTOR3,
            [](const godot::Variant& transform_value,
                Position3D& position,
                godot::Transform3D& transform_component,
                Rotation3D* rotation,
                Scale3D* scale)
    {
        godot::Transform3D physics_transform = transform_value;
        transform_component = physics_transform;
        position.value = physics_transform.origin;
        if (rotation) { rotation->value = physics_transform.basis.get_euler(); }
        if (scale) { scale->value = physics_transform.basis.get_scale(); }
    },
            [](const godot::Variant& velocity_value, Velocity3D& velocity_component)
    {
        godot::Vector3 physics_velocity = velocity_value;
        velocity_component.value = physics_velocity;
    });

    register_physics_cleanup_observer<PhysicsBodyInstance3D, godot::PhysicsServer3D>(
        world,
        "Physics Body 3D Cleanup");
});
