#pragma once

#include <string>

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "src/components/physics.h"
#include "src/components/transform.h"
#include "src/flecs_registry.h"

using godot::Array;
using godot::Dictionary;
using godot::UtilityFunctions;
using godot::Variant;

namespace
{
    template<typename ShapesT, typename ShapeDefinitionT, typename SpaceT, typename ServerT, typename TransformT, typename InstanceT, typename BodyStateT>
    inline bool create_physics_body(
        flecs::entity& instance,
        const ShapesT& body_definition,
        const SpaceT* physics_space,
        ServerT* physics_server,
        const TransformT& transform,
        BodyStateT transform_state,
        const char* invalid_shape_warning)
    {
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
            if (shape_def.shape.is_null())
            {
                UtilityFunctions::push_warning(invalid_shape_warning);
                continue;
            }
            physics_server->body_add_shape(body_rid, shape_def.shape->get_rid(), shape_def.local_transform);
            added_shapes++;
        }

        if (added_shapes == 0)
        {
            physics_server->free_rid(body_rid);
            return false;
        }

        physics_server->body_set_state(body_rid, transform_state, transform);
        instance.set<InstanceT>({ body_rid });
        return true;
    }
}

inline FlecsRegistry register_prefab_instantiation_system([](flecs::world& world)
{
    world.system<>("Prefab Instantiation")
        .kind(0) // On-demand
        .write<Position2D>()
        .write<Rotation2D>()
        .write<Scale2D>()
        .write<Position3D>()
        .write<Rotation3D>()
        .write<Scale3D>()
        .write<PhysicsBodyInstance2D>()
        .write<PhysicsBodyInstance3D>()
        .run([&](flecs::iter& it)
    {
        const Dictionary* parameters = static_cast<const Dictionary*>(it.param());
        if (!parameters || parameters->is_empty()) {
            UtilityFunctions::push_error("Prefab Instantiation: system called without parameters. At least 'prefab' needs to be specified.");
            return;
        }

        if (!parameters->has("prefab")) {
            UtilityFunctions::push_error("Prefab Instantiation: 'prefab' parameter needs to be given with the name of the prefab to instantiate.");
            return;
        }
        if ((*parameters)["prefab"].get_type() != Variant::STRING)
        {
            UtilityFunctions::push_error("Prefab Instantiation: 'prefab' parameter must be a String.");
            return;
        }
        godot::String prefab_name = (*parameters)["prefab"];
        std::string prefab_name_str = prefab_name.utf8().get_data();
        flecs::entity prefab = world.lookup(prefab_name_str.c_str());
        if (!prefab.is_valid())
        {
            UtilityFunctions::push_error(godot::String("Prefab Instantiation: prefab '") + prefab_name + "' not found in Flecs world");
            return;
        }

        int count = 1;
        if (parameters->has("count"))
        {
            if ((*parameters)["count"].get_type() != Variant::INT)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'count' parameter must be an Integer.");
                return;
            }
            count = (*parameters)["count"];
        }

        bool has_transforms = false;
        bool transforms_are_2d = false;
        Array transforms_array;
        if (parameters->has("transforms"))
        {
            if ((*parameters)["transforms"].get_type() != Variant::ARRAY)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' parameter must be an Array of Transform2D or Transform3D.");
                return;
            }

            transforms_array = (*parameters)["transforms"];
            if (transforms_array.is_empty()) {
                UtilityFunctions::push_error("Prefab Instantiation: Empty 'transforms' Array given.");
                return;
            }

            if ((int)transforms_array.size() != count)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' array size must be equal to 'count'.");
                return;
            }

            // Determine whether array contains Transform2D or Transform3D values
            Variant first = transforms_array[0];
            if (first.get_type() == Variant::TRANSFORM2D) {
                transforms_are_2d = true;
            }
            else if (first.get_type() == Variant::TRANSFORM3D) {
                transforms_are_2d = false;
            }
            else {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' Array must contain Transform2D or Transform3D elements.");
                return;
            }

            // Ensure all elements are of the same transform type
            for (int transform_idx = 1; transform_idx < (int)transforms_array.size(); ++transform_idx) {
                Variant v = transforms_array[transform_idx];
                if (transforms_are_2d && v.get_type() != Variant::TRANSFORM2D) {
                    UtilityFunctions::push_error("Prefab Instantiation: all elements in 'transforms' must be Transform2D when the first element is a Transform2D.");
                    return;
                }
                if (!transforms_are_2d && v.get_type() != Variant::TRANSFORM3D) {
                    UtilityFunctions::push_error("Prefab Instantiation: all elements in 'transforms' must be Transform3D when the first element is a Transform3D.");
                    return;
                }
            }

            has_transforms = true;
        }

        godot::PhysicsServer2D* physics_server_2d = godot::PhysicsServer2D::get_singleton();
        const PhysicsSpace2D* physics_space_2d = physics_server_2d ? it.world().try_get<PhysicsSpace2D>() : nullptr;
        bool physics_2d_ready = physics_server_2d && physics_space_2d && physics_space_2d->space_rid.is_valid();
        bool warned_missing_physics_2d = false;

        godot::PhysicsServer3D* physics_server_3d = godot::PhysicsServer3D::get_singleton();
        const PhysicsSpace3D* physics_space_3d = physics_server_3d ? it.world().try_get<PhysicsSpace3D>() : nullptr;
        bool physics_3d_ready = physics_server_3d && physics_space_3d && physics_space_3d->space_rid.is_valid();
        bool warned_missing_physics_3d = false;

        for (int instance_idx = 0; instance_idx < count; ++instance_idx)
        {
            flecs::entity instance = world.entity().is_a(prefab);
            bool has_spawn_transform_2d = false;
            godot::Transform2D spawn_transform_2d;
            bool has_spawn_transform_3d = false;
            godot::Transform3D spawn_transform_3d;
            if (has_transforms) {
                if (transforms_are_2d) {
                    godot::Transform2D transform = transforms_array[instance_idx];
                    godot::Vector2 position = transform.get_origin();
                    godot::real_t rotation = transform.get_rotation();
                    godot::Vector2 scale = transform.get_scale();

                    instance.set<Position2D>({ position });
                    instance.set<Rotation2D>({ rotation });
                    instance.set<Scale2D>({ scale });
                    instance.set<godot::Transform2D>(transform);
                    spawn_transform_2d = transform;
                    has_spawn_transform_2d = true;
                }
                else {
                    godot::Transform3D transform = transforms_array[instance_idx];
                    godot::Vector3 position = transform.get_origin();
                    godot::Vector3 rotation = transform.get_basis().get_euler();
                    godot::Vector3 scale = transform.get_basis().get_scale();

                    instance.set<Position3D>({ position });
                    instance.set<Rotation3D>({ rotation });
                    instance.set<Scale3D>({ scale });
                    instance.set<godot::Transform3D>(transform);
                    spawn_transform_3d = transform;
                    has_spawn_transform_3d = true;
                }
            }

            if (!physics_2d_ready)
            {
                physics_space_2d = physics_server_2d ? it.world().try_get<PhysicsSpace2D>() : nullptr;
                physics_2d_ready = physics_server_2d && physics_space_2d && physics_space_2d->space_rid.is_valid();
            }

            const PhysicsBodyShapes2D* body_shapes = instance.try_get<PhysicsBodyShapes2D>();
            if (body_shapes && !body_shapes->shapes.empty())
            {
                if (!physics_2d_ready)
                {
                    if (!warned_missing_physics_2d)
                    {
                        UtilityFunctions::push_warning("Prefab Instantiation: PhysicsBodyShapes2D present but PhysicsServer2D space is unavailable.");
                        warned_missing_physics_2d = true;
                    }
                    continue;
                }

                const godot::Transform2D* transform_component = nullptr;
                if (has_spawn_transform_2d)
                {
                    transform_component = &spawn_transform_2d;
                }
                else
                {
                    transform_component = instance.try_get<godot::Transform2D>();
                }

                godot::Transform2D final_transform = transform_component ? *transform_component : godot::Transform2D();

                if (!create_physics_body<
                    PhysicsBodyShapes2D,
                    PhysicsBodyShape2DDefinition,
                    PhysicsSpace2D,
                    godot::PhysicsServer2D,
                    godot::Transform2D,
                    PhysicsBodyInstance2D,
                    godot::PhysicsServer2D::BodyState>(
                        instance,
                        *body_shapes,
                        physics_space_2d,
                        physics_server_2d,
                        final_transform,
                        godot::PhysicsServer2D::BODY_STATE_TRANSFORM,
                        "Prefab Instantiation: PhysicsBodyShapes2D contains an invalid Shape2D reference."))
                {
                    UtilityFunctions::push_warning(godot::String("Prefab Instantiation: Failed to create PhysicsServer2D body for prefab '") + prefab_name + "'.");
                }
            }

            if (!physics_3d_ready)
            {
                physics_space_3d = physics_server_3d ? it.world().try_get<PhysicsSpace3D>() : nullptr;
                physics_3d_ready = physics_server_3d && physics_space_3d && physics_space_3d->space_rid.is_valid();
            }

            const PhysicsBodyShapes3D* body_shapes_3d = instance.try_get<PhysicsBodyShapes3D>();
            if (body_shapes_3d && !body_shapes_3d->shapes.empty())
            {
                if (!physics_3d_ready)
                {
                    if (!warned_missing_physics_3d)
                    {
                        UtilityFunctions::push_warning("Prefab Instantiation: PhysicsBodyShapes3D present but PhysicsServer3D space is unavailable.");
                        warned_missing_physics_3d = true;
                    }
                    continue;
                }

                const godot::Transform3D* transform_component = nullptr;
                if (has_spawn_transform_3d)
                {
                    transform_component = &spawn_transform_3d;
                }
                else
                {
                    transform_component = instance.try_get<godot::Transform3D>();
                }

                godot::Transform3D final_transform = transform_component ? *transform_component : godot::Transform3D();

                if (!create_physics_body<
                    PhysicsBodyShapes3D,
                    PhysicsBodyShape3DDefinition,
                    PhysicsSpace3D,
                    godot::PhysicsServer3D,
                    godot::Transform3D,
                    PhysicsBodyInstance3D,
                    godot::PhysicsServer3D::BodyState>(
                        instance,
                        *body_shapes_3d,
                        physics_space_3d,
                        physics_server_3d,
                        final_transform,
                        godot::PhysicsServer3D::BODY_STATE_TRANSFORM,
                        "Prefab Instantiation: PhysicsBodyShapes3D contains an invalid Shape3D reference."))
                {
                    UtilityFunctions::push_warning(godot::String("Prefab Instantiation: Failed to create PhysicsServer3D body for prefab '") + prefab_name + "'.");
                }
            }
        }

        // UtilityFunctions::print(godot::String("Prefab Instantiation: spawned ") + godot::String::num_int64(count) + " instances of '" + prefab_name + "'");
    });
});
