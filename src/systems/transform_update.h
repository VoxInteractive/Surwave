#pragma once

#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

#include "../components/transform.h"
#include "flecs_registry.h"

inline FlecsRegistry register_transform_update_systems([](flecs::world& world)
{
    // This system updates the Transform2D component for entities that have Position2D, Rotation2D, and Scale2D.
    // It also ensures that the Transform2D component exists.
    world.system<const Position2D, const Rotation2D, const Scale2D, godot::Transform2D*>("Transform2D Update")
        .kind(flecs::PreStore)
        .multi_threaded()
        .write<godot::Transform2D>() // Declare that this system writes to Transform2D
        .each([](flecs::entity e, const Position2D& position, const Rotation2D& rotation, const Scale2D& scale, godot::Transform2D* transform_ptr)
    {
        if (transform_ptr) {
            *transform_ptr = {
                rotation.value,
                godot::Size2(scale.x, scale.y),
                0.0f, // skew
                godot::Vector2(position.x, position.y)
            };
        }
        else {
            e.set<godot::Transform2D>({
                rotation.value,
                godot::Size2(scale.x, scale.y),
                0.0f, // skew
                godot::Vector2(position.x, position.y)
                });
        }
    });

    // This system updates the Transform3D component for entities that have Position3D, Rotation3D, and Scale3D.
    // It also ensures that the Transform3D component exists.
    world.system<const Position3D, const Rotation3D, const Scale3D, godot::Transform3D*>("Transform3D Update")
        .kind(flecs::PreStore)
        .multi_threaded()
        .write<godot::Transform3D>() // Declare that this system writes to Transform3D
        .each([](flecs::entity e, const Position3D& position, const Rotation3D& rotation, const Scale3D& scale, godot::Transform3D* transform_ptr)
    {
        // The Quaternion approach is more efficient. While Quaternion::from_euler has a cost,
        // it avoids the multiple matrix multiplications that Basis::set_euler_scale performs internally.
        if (transform_ptr) {
            *transform_ptr = {
                godot::Basis(
                    godot::Quaternion::from_euler({ rotation.x, rotation.y, rotation.z }),
                    { scale.x, scale.y, scale.z }
                ),
                { position.x, position.y, position.z }
            };
        }
        else {
            e.set<godot::Transform3D>({
                godot::Basis(
                    godot::Quaternion::from_euler({ rotation.x, rotation.y, rotation.z }),
                    { scale.x, scale.y, scale.z }
                ),
                { position.x, position.y, position.z }
                });
        }
    });
});;
