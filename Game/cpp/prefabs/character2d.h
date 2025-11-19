#pragma once

#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"

#include "src/components/transform.h"
#include "src/components/entity_rendering.h"
#include "src/components/physics.h"

inline const PhysicsBodyShapes2D& get_character2d_physics_shapes()
{
    static const PhysicsBodyShapes2D shapes = []() {
        PhysicsBodyShapes2D data;
        data.body_mode = godot::PhysicsServer2D::BODY_MODE_KINEMATIC;
        data.collision_layer = 16; // Monsters layer
        data.collision_mask = 1 | 2 | 4 | 8 | 16; // Borders, objects, players, portals, monsters

        godot::Ref<godot::CircleShape2D> collider;
        collider.instantiate();
        collider->set_radius(8.0f);
        data.shapes.push_back({ collider, godot::Transform2D() });
        return data;
    }();
    return shapes;
}


inline FlecsRegistry register_character2d_prefab([](flecs::world& world) {
    world.prefab("Character2D")
        .set_auto_override<Position2D>({ godot::Vector2(0.0f, 0.0f) })
        .set_auto_override<Rotation2D>({ 0.0f })
        .set_auto_override<Scale2D>({ godot::Vector2(1.0f, 1.0f) })
        .set_auto_override<godot::Transform2D>(godot::Transform2D())
        .set_auto_override<Velocity2D>({ godot::Vector2(0.0f, 0.0f) })
        .set_auto_override<PhysicsBodyShapes2D>(get_character2d_physics_shapes())
        .set_auto_override<RenderingColor>({ 1.0f, 1.0f, 1.0f, 1.0f })
        .set_auto_override<RenderingCustomData>({ 0.0f, 0.0f, 0.0f, 0.0f });
});
