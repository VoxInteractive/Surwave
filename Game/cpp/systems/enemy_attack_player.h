#pragma once

#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/transform.h"

#include "components/singletons.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_attack_player_system([](flecs::world& world) {
    world.system<
        const Position2D,
        const PlayerPosition&,
        const CharacterContactEndDistanceSquared&>("Enemy Attack Player")
        .with<EnemyState, EnemyState::AttackingThePlayer>()
        .each([](
            flecs::entity entity,
            const Position2D& position,
            const PlayerPosition& player_position,
            const CharacterContactEndDistanceSquared& contact_end_dist_sq) {

        // Exit Attacking if we are too far from the player
        const godot::real_t distance_to_player_sq = position.value.distance_squared_to(player_position.value);
        if (distance_to_player_sq > contact_end_dist_sq.value) {
            set_state<EnemyState::ChasingThePlayer>(entity);
            return;
        }

        // No behavior inside AttackingThePlayer yet.
    });
});
