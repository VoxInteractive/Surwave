#pragma once

#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

#include "../components/transform.h"
#include "flecs_registry.h"

inline FlecsRegistry register_transform_update_systems([](flecs::world& world)
{
    // This system updates the Transform2D component for entities that have Position2D, Rotation2D, and Scale2D.
    // Prerequisite: All entities matching this query must already have a godot::Transform2D component.
    world.system<const Position2D, const Rotation2D, const Scale2D, godot::Transform2D>("Transform2D Update")
        .kind(flecs::PreStore)
        .term_at(4).out() // Mark godot::Transform2D as [out]
        .each([](const Position2D& position, const Rotation2D& rotation, const Scale2D& scale, godot::Transform2D& transform)
    {
        transform = {
            rotation.value,
            godot::Size2(scale.x, scale.y),
            0.0f, // skew
            godot::Vector2(position.x, position.y)
        };
    });

    // This system updates the Transform3D component for entities that have Position3D, Rotation3D, and Scale3D.
    // Prerequisite: All entities matching this query must already have a godot::Transform3D component.
    world.system<const Position3D, const Rotation3D, const Scale3D, godot::Transform3D>("Transform3D Update")
        .kind(flecs::PreStore)
        .term_at(4).out() // Mark godot::Transform3D as [out]
        .each([](const Position3D& position, const Rotation3D& rotation, const Scale3D& scale, godot::Transform3D& transform)
    {
        // The Quaternion approach is more efficient. While Quaternion::from_euler has a cost,
        // it avoids the multiple matrix multiplications that Basis::set_euler_scale performs internally.
        transform = {
            godot::Basis(
                godot::Quaternion::from_euler({ rotation.x, rotation.y, rotation.z }),
                { scale.x, scale.y, scale.z }
            ),
            { position.x, position.y, position.z }
        };
    });
});
