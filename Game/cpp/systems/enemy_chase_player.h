#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/physics.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/enemy_state.h"
#include "components/singletons.h"


inline FlecsRegistry register_enemy_chase_player_system([](flecs::world& world) {
	world.system<
		const Position2D,
		Velocity2D,
		const MovementSpeed,
		const LoseTargetRadiusSquared,
		const PlayerPosition&,
		const CharacterContactBeginDistanceSquared&>("Enemy Chase Player")
		.with<EnemyState, EnemyState::ChasingThePlayer>()
		.each([](
			flecs::iter& it,
			size_t i,
			const Position2D& position,
			Velocity2D& velocity,
			const MovementSpeed& movement_speed,
			const LoseTargetRadiusSquared& lose_target_radius_sq,
			const PlayerPosition& player_position,
			const CharacterContactBeginDistanceSquared& contact_begin_dist_sq) {

		// Condition to transition to Idle
		const float distance_to_player_sq = position.value.distance_squared_to(player_position.value);
		if (distance_to_player_sq > lose_target_radius_sq.value) {
			velocity.value = godot::Vector2(0.0f, 0.0f);
			set_state<EnemyState::Idle>(it.entity(i));
			return; // Stop processing this entity for this frame
		}

		// Enter Attacking when close enough
		if (distance_to_player_sq < contact_begin_dist_sq.value) {
			velocity.value = godot::Vector2(0.0f, 0.0f);
			set_state<EnemyState::AttackingThePlayer>(it.entity(i));
			return;
		}

		// Behavior: Chase the player
		godot::Vector2 direction = player_position.value - position.value;
		if (direction.length_squared() > 0) {
			direction = direction.normalized();
		}

		velocity.value = direction * movement_speed.value;
	});
});
