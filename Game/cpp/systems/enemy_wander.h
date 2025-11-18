#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_wander_system([](flecs::world& world) {
	world.system<Position2D, const MovementSpeed, const EnemyState::Wandering>("Enemy Wander")
		.with(flecs::IsA, world.lookup("Enemy"))
		.each([](flecs::iter& it, size_t i, Position2D& position, const MovementSpeed& movement_speed, const EnemyState::Wandering& wandering) {

		const godot::Vector2& random_direction = wandering.destination;
		godot::Vector2 velocity = random_direction * movement_speed.value;

		position.value += velocity * it.delta_time();
	});
});
