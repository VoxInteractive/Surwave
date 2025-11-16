#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_wander_system([](flecs::world& world) {
	world.system<Position2D, const MovementSpeed>("Enemy Wander")
		.with(flecs::IsA, world.lookup("Enemy"))
		.with<EnemyState, EnemyState::Wandering>()
		.each([](flecs::iter& it, size_t i, Position2D& position, const MovementSpeed& movement_speed) {

		float random_angle = godot::UtilityFunctions::randf_range(0.0f, Math_TAU);
		godot::Vector2 random_direction = godot::Vector2(cos(random_angle), sin(random_angle));
		godot::Vector2 velocity = random_direction * movement_speed.value;
		position.x += velocity.x * it.delta_time();
		position.y += velocity.y * it.delta_time();
	});
});
