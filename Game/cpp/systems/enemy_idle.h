#pragma once

#include <godot_cpp/variant/utility_functions.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/enemy_attributes.h"
#include "components/enemy_state.h"

inline FlecsRegistry register_enemy_idle_system([](flecs::world& world) {
    world.system<const Position2D, const PlayerDetectionRadiusSquared, const WanderRestDuration, const TimeInState, const PlayerPosition&>("Enemy Idle")
        .with<EnemyState::Idle>()
        .each([](flecs::entity entity, const Position2D& pos, const PlayerDetectionRadiusSquared& detect_sq, const WanderRestDuration& rest_dur, const TimeInState& time, const PlayerPosition& player_pos) {

        // Condition to transition to Chasing
        const float distance_to_player_sq = pos.value.distance_squared_to(player_pos.value);
        if (distance_to_player_sq <= detect_sq.value) {
            set_state<EnemyState::ChasingThePlayer>(entity);
            return;
        }

        // Condition to transition to Wandering
        if (time.value > rest_dur.value) {
            float random_angle = godot::UtilityFunctions::randf_range(0.0f, Math_TAU);
            godot::Vector2 random_direction = godot::Vector2(cos(random_angle), sin(random_angle));
            set_state<EnemyState::Wandering>(entity, random_direction);
        }
    });
});