#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_wander_system([](flecs::world& world) {
	world.system <
		Position2D,
		const MovementSpeed,
		const PlayerDetectionRadiusSquared,
		const WanderMoveDuration,
		const TimeInState,
		const PlayerPosition&,
		const EnemyState::Wandering>("Enemy Wander")
		.with<EnemyState, EnemyState::Wandering>()
		.each([](
			flecs::iter& it,
			size_t i,
			Position2D& position,
			const MovementSpeed& movement_speed,
			const PlayerDetectionRadiusSquared& player_detection_radius_sq,
			const WanderMoveDuration& move_duration,
			const TimeInState& time,
			const PlayerPosition& player_position,
			const EnemyState::Wandering& wandering) {

		flecs::entity entity = it.entity(i);

		// Condition to transition to Chasing
		const float distance_to_player_sq = position.value.distance_squared_to(player_position.value);
		if (distance_to_player_sq <= player_detection_radius_sq.value) {
			set_state<EnemyState::ChasingThePlayer>(entity);
			return;
		}

		// Condition to transition to Idle
		if (time.value > move_duration.value) {
			set_state<EnemyState::Idle>(entity);
			return;
		}

		const godot::Vector2& random_direction = wandering.destination;
		godot::Vector2 velocity = random_direction * movement_speed.value;

		position.value += velocity * it.delta_time();
	});
});
