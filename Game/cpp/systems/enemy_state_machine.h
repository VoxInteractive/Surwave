#pragma once

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_state_machine_system([](flecs::world& world) {
	world.system<const Position2D, const PlayerDetectionRadius>("Enemy State Machine")
		.with(flecs::IsA, world.lookup("Enemy"))
		.each([](flecs::iter& it, size_t i, const Position2D& position, const PlayerDetectionRadius& detection_radius) {
		const auto* player_position = it.world().try_get<PlayerPosition>();
		if (!player_position) {
			return; // PlayerPosition singleton not found.
		}

		godot::Vector2 position_vector{ position.x, position.y };
		const float distance_sq = position_vector.distance_squared_to(player_position->value);

		if (distance_sq <= (detection_radius.value * detection_radius.value)) {
			set_state<EnemyState::Chasing>(it.entity(i));
		}
		else {
			set_state<EnemyState::Wandering>(it.entity(i));
		}
	});
});
