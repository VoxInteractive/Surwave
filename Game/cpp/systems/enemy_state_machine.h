#pragma once

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_state_machine_system([](flecs::world& world) {
	world.system<
		const Position2D,
		const PlayerDetectionRadiusSquared,
		const LoseTargetRadiusSquared,
		const PlayerPosition&
	>("Enemy State Machine")
		.with(flecs::IsA, world.lookup("Enemy"))
		.kind(flecs::PostLoad)
		.rate(2) // Run every other frame
		.each([](
			flecs::iter& it,
			size_t i,
			const Position2D& position,
			const PlayerDetectionRadiusSquared& player_detection_radius_sq,
			const LoseTargetRadiusSquared& lose_target_radius_sq,
			const PlayerPosition& player_position
			) {

		flecs::entity entity = it.entity(i);
		entity.set<TimeInState>({ entity.get<TimeInState>().value + it.delta_time() });

		const float distance_to_player_sq = position.value.distance_squared_to(player_position.value);

		if (distance_to_player_sq <= player_detection_radius_sq.value) {
			set_state<EnemyState::ChasingThePlayer>(entity);
		}
		else if (distance_to_player_sq >= lose_target_radius_sq.value) {
			float random_angle = godot::UtilityFunctions::randf_range(0.0f, Math_TAU);
			godot::Vector2 random_direction = godot::Vector2(cos(random_angle), sin(random_angle));

			set_state<EnemyState::Wandering>(entity, random_direction);
		}
	});
});
