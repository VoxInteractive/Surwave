#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_state.h"
#include "components/enemy_stats.h"


inline FlecsRegistry register_enemy_movement_system([](flecs::world& world) {
	world.system<
		Position2D,
		EnemyState,
		TimeInState,
		WanderDirection,
		const MovementSpeed,
		const PlayerPosition,
		const PlayerDetectionRadius,
		const WanderMoveDuration,
		const WanderRestDuration
	>("Enemy Movement")
		.with(flecs::IsA, world.lookup("Enemy"))
		.each([](
			flecs::iter& it,
			size_t i,
			Position2D& position,
			EnemyState& current_state,
			TimeInState& time_in_state,
			WanderDirection& wander_direction,
			const MovementSpeed& movement_speed,
			const PlayerPosition& player_position,
			const PlayerDetectionRadius& detection_radius,
			const WanderMoveDuration& move_duration,
			const WanderRestDuration& rest_duration
			) {
		flecs::entity e = it.entity(i);

		e.set<TimeInState>({ time_in_state.value + it.delta_time() });

		godot::Vector2 current_position(position.x, position.y);
		float distance_to_player = player_position.value.distance_to(current_position);

		if (distance_to_player <= detection_radius.value)
		{
			e.set<EnemyState>({ EnemyState::CHASING }).set<TimeInState>({ 0.0f });
		}


		if (current_state == EnemyState::CHASING)
		{
			if (distance_to_player > detection_radius.value)
			{
				e.set<EnemyState>({ EnemyState::WANDERING_RESTING }).set<TimeInState>({ 0.0f });
			}
			else
			{
				godot::Vector2 direction = player_position.value - current_position;
				if (direction.length_squared() > 0) {
					direction = direction.normalized();
				}

				godot::Vector2 velocity = direction * movement_speed.value;
				position.x += velocity.x * it.delta_time();
				position.y += velocity.y * it.delta_time();
			}
		}
		else if (current_state == EnemyState::WANDERING_RESTING)
		{
			if (time_in_state.value >= rest_duration.value)
			{
				e.set<EnemyState>({ EnemyState::WANDERING_MOVING }).set<TimeInState>({ 0.0f });
				wander_direction.value = godot::Vector2(
					(float)rand() / RAND_MAX * 2.0f - 1.0f,
					(float)rand() / RAND_MAX * 2.0f - 1.0f
				).normalized();
			}
		}
		else if (current_state == EnemyState::WANDERING_MOVING)
		{
			if (time_in_state.value >= move_duration.value)
			{
				e.set<EnemyState>({ EnemyState::WANDERING_RESTING }).set<TimeInState>({ 0.0f });
			}
			else
			{
				godot::Vector2 velocity = wander_direction.value * movement_speed.value;
				position.x += velocity.x * it.delta_time();
				position.y += velocity.y * it.delta_time();
			}
		}
	});
});
