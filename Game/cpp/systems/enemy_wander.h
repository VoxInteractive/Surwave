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
		const PlayerPosition&>("Enemy Wander")
		.with<EnemyState, EnemyState::Wandering>()
		.each([](
			flecs::iter& it,
			size_t i,
			Position2D& position,
			const MovementSpeed& movement_speed,
			const PlayerDetectionRadiusSquared& player_detection_radius_sq,
			const WanderMoveDuration& move_duration,
			const TimeInState& time,
			const PlayerPosition& player_position) {

		flecs::entity entity = it.entity(i);
		const EnemyState::Wandering& wandering = entity.get<EnemyState, EnemyState::Wandering>();

		// Ensure a direction exists; if missing, set and use it immediately.
		godot::Vector2 move_direction = wandering.direction;
		if (move_direction.length_squared() == 0.0f) {
			float choice = godot::UtilityFunctions::randf();
			if (choice < (1.0f / 3.0f)) {
				// 1/3: head towards the world origin (0,0).
				const godot::Vector2 to_origin = -position.value;
				if (to_origin.length_squared() < 1000.0f) {
					// If we're close to the origin, fall back to a random direction.
					float random_angle = godot::UtilityFunctions::randf_range(0.0f, Math_TAU);
					move_direction = godot::Vector2(cos(random_angle), sin(random_angle));
				}
				else {
					move_direction = to_origin.normalized();
				}
			}
			else if (choice < (2.0f / 3.0f)) {
				// 1/3: head toward the current player position.
				const godot::Vector2 to_player = player_position.value - position.value;
				move_direction = to_player.normalized();
			}
			else {
				// 1/3: pick a random direction.
				const float random_angle = godot::UtilityFunctions::randf_range(0.0f, Math_TAU);
				move_direction = godot::Vector2(cos(random_angle), sin(random_angle));
			}

			set_state<EnemyState::Wandering>(entity, move_direction);
		}

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

		godot::Vector2 velocity = move_direction * movement_speed.value;

		position.value += velocity * it.delta_time();
	});
});
