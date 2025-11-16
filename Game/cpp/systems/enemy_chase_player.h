#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_chase_player_system([](flecs::world& world) {
	world.system<Position2D, const MovementSpeed, const PlayerPosition>("Enemy Chase Player")
		.with(flecs::IsA, world.lookup("Enemy"))
		.with<EnemyState, EnemyState::Chasing>()
		.each([](flecs::iter& it, size_t i, Position2D& position, const MovementSpeed& movement_speed, const PlayerPosition& player_position) {

		godot::Vector2 direction = player_position.value - godot::Vector2(position.x, position.y);
		if (direction.length_squared() > 0) {
			direction = direction.normalized();
		}

		godot::Vector2 velocity = direction * movement_speed.value;
		position.x += velocity.x * it.delta_time();
		position.y += velocity.y * it.delta_time();
	});
});
