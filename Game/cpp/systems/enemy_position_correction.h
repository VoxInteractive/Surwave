#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/transform.h"

#include "components/enemy_state.h"
#include "components/singletons.h"


inline FlecsRegistry register_enemy_position_buffer_update_system([](flecs::world& world) {
    flecs::query<const Position2D> enemy_positions_query =
        world.query_builder<const Position2D>()
        .with<EnemyState, EnemyState::ChasingThePlayer>().or_()
        .with<EnemyState, EnemyState::Wandering>()
        .build();

    world.system<EnemySpatialHashGrid&, const EnemySeparationDistanceSquared&>("Enemy Position Buffer Update")
        .kind(flecs::OnValidate)
        .each([enemy_positions_query](EnemySpatialHashGrid& grid, const EnemySeparationDistanceSquared& separation_distance_sq) {
        const std::size_t enemy_count = static_cast<std::size_t>(enemy_positions_query.count());
        grid.begin_build(separation_distance_sq.value);
        grid.reserve(enemy_count);

        enemy_positions_query.each([&grid](flecs::entity enemy_entity, const Position2D& position) {
            grid.insert(enemy_entity, position.value);
        });
    });
});

inline FlecsRegistry register_enemy_position_correction_system([](flecs::world& world) {
    world.system<
        Position2D,
        const EnemySeparationDistanceSquared&,
        const EnemySpatialHashGrid&>("Enemy Position Correction")
        .with<EnemyState, EnemyState::ChasingThePlayer>().or_()
        .with<EnemyState, EnemyState::Wandering>()
        .kind(flecs::PostUpdate)
        .each([](
            flecs::iter& it,
            size_t i,
            Position2D& position,
            const EnemySeparationDistanceSquared& separation_dist_sq,
            const EnemySpatialHashGrid& enemy_grid) {

        godot::Vector2 separation_force;
        int neighbours = 0;
        const godot::real_t search_radius = enemy_grid.cell_size;
        const godot::Vector2 current_position = position.value;

        enemy_grid.for_each_neighbor(current_position, search_radius, [&](flecs::entity_t other_entity_id, const godot::Vector2& other_position) {
            if (other_entity_id == it.entity(i).id()) {
                return;
            }

            const godot::Vector2 distance_to_other = other_position - current_position;
            if (distance_to_other.length_squared() < separation_dist_sq.value) {
                separation_force -= distance_to_other;
                neighbours++;
            }
        });

        if (neighbours > 0) {
            position.value = current_position + separation_force * 0.1f; // Apply a small nudge
        }
    });
});
