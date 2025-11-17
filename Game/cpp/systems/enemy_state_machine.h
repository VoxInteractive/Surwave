#pragma once

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_state_machine_system([](flecs::world& world) {
	world.system<const Position2D, const PlayerDetectionRadiusSquared, const PlayerPosition&>("Enemy State Machine")
		.with(flecs::IsA, world.lookup("Enemy"))
		.each([](flecs::iter& it, size_t i, const Position2D& position, const PlayerDetectionRadiusSquared& detection_radius_sq, const PlayerPosition& player_position) {

		flecs::entity entity = it.entity(i);
		entity.set<TimeInState>({ entity.get<TimeInState>().value + it.delta_time() });

		const float distance_to_player_sq = godot::Vector2(position.x, position.y).distance_squared_to(player_position.value);
		// Hysteresis: Use a larger radius to lose the target than to acquire it.
		const float lose_target_radius = detection_radius_sq.value * 1.2f;

		if (entity.has<EnemyState::Chasing>() && distance_to_player_sq > lose_target_radius) {
			set_state<EnemyState::Idle>(entity);
		}
		else if (distance_to_player_sq <= detection_radius_sq.value) {
			set_state<EnemyState::Chasing>(entity, player_position.value);
		}
	});
});
