#pragma once

#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

#include "../components/transform.h"
#include "flecs_registry.h"

inline FlecsRegistry register_transform_update_systems([](flecs::world& world)
{
    // This system updates the Transform2D component for entities that have Position2D, Rotation2D, and Scale2D.
    world.system<const Position2D, const Rotation2D, const Scale2D>("Transform2D Update")
        .kind(flecs::PreStore)
        .multi_threaded()
        .write<godot::Transform2D>() //  This will ensure that if any subsequent system reads these transform components, Flecs will insert a sync point to merge the deferred set operations first.
        .each([](flecs::entity e, const Position2D& position, const Rotation2D& rotation, const Scale2D& scale)
    {
        e.set<godot::Transform2D>({
            rotation.value,
            godot::Size2(scale.x, scale.y),
            0.0f, // skew
            godot::Vector2(position.x, position.y)
            });
    });

    // This system updates the Transform3D component for entities that have Position3D, Rotation3D, and Scale3D.
    world.system<const Position3D, const Rotation3D, const Scale3D>("Transform3D Update")
        .kind(flecs::PreStore)
        .multi_threaded()
        .write<godot::Transform3D>()
        .each([](flecs::entity e, const Position3D& position, const Rotation3D& rotation, const Scale3D& scale)
    {
        // The Quaternion approach is more efficient. While Quaternion::from_euler has a cost,
        // it avoids the multiple matrix multiplications that Basis::set_euler_scale performs internally.
        e.set<godot::Transform3D>({
            godot::Basis(
                godot::Quaternion::from_euler({ rotation.x, rotation.y, rotation.z }),
                { scale.x, scale.y, scale.z }
            ),
            { position.x, position.y, position.z }
            });
    });
});
