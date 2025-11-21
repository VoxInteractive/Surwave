#pragma once

#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"

#include "src/components/physics.h"
#include "src/components/transform.h"
#include "src/components/entity_rendering.h"


inline FlecsRegistry register_character2d_prefab([](flecs::world& world) {
    world.prefab("Character2D")
        .set_auto_override<Velocity2D>({ godot::Vector2(0.0f, 0.0f) })

        .set_auto_override<Position2D>({ godot::Vector2(0.0f, 0.0f) })
        .set_auto_override<Rotation2D>({ 0.0f })
        .set_auto_override<Scale2D>({ godot::Vector2(1.0f, 1.0f) })
        .set_auto_override<godot::Transform2D>(godot::Transform2D())

        .set_auto_override<RenderingColor>({ 1.0f, 1.0f, 1.0f, 1.0f })
        .set_auto_override<RenderingCustomData>({ 0.0f, 0.0f, 0.0f, 0.0f });
});
