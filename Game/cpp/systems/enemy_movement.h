#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/player.h"
#include "src/components/physics.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/singletons.h"
#include "utilities/enemy_spatial_hash.h"

namespace enemy_movement {

    struct BoidAccessor {
        Position2D* position;
        Velocity2D* velocity;
        const MovementSpeed* movement_speed;
    };

    inline godot::Vector2 steer_towards(const godot::Vector2& desired_direction, const godot::Vector2& current_velocity, godot::real_t max_speed) {
        if (desired_direction.length_squared() == 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        const godot::Vector2 desired_velocity = desired_direction.normalized() * max_speed;
        return desired_velocity - current_velocity;
    }

    inline godot::Vector2 limit_vector(const godot::Vector2& value, godot::real_t max_length) {
        if (max_length <= 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        const godot::real_t current_length_sq = value.length_squared();
        const godot::real_t max_length_sq = max_length * max_length;
        if (current_length_sq <= max_length_sq) {
            return value;
        }

        if (current_length_sq == 0.0f) {
            return godot::Vector2(0.0f, 0.0f);
        }

        return value.normalized() * max_length;
    }

} // namespace enemy_movement

inline FlecsRegistry register_enemy_movement_system([](flecs::world& world) {
    world.system<Position2D, Velocity2D, const MovementSpeed, const DeathTimer>("Enemy Movement")
        .with(flecs::IsA, world.lookup("Enemy"))
        .run([](flecs::iter& it) {
        flecs::world stage_world = it.world();
        const PlayerPosition* player_position = stage_world.try_get<PlayerPosition>();
        const EnemyBoidForceWeights* force_weights = stage_world.try_get<EnemyBoidForceWeights>();
        const EnemyBoidMovementSettings* movement_settings = stage_world.try_get<EnemyBoidMovementSettings>();

        if (player_position == nullptr || force_weights == nullptr || movement_settings == nullptr) {
            return;
        }

        std::vector<enemy_movement::BoidAccessor> boids;
        boids.reserve(128U);

        godot::real_t delta_time = 0.0f;
        bool delta_time_initialized = false;

        while (it.next()) {
            if (!delta_time_initialized) {
                delta_time = it.delta_time();
                delta_time_initialized = true;
            }

            flecs::field<Position2D> positions = it.field<Position2D>(0);
            flecs::field<Velocity2D> velocities = it.field<Velocity2D>(1);
            flecs::field<const MovementSpeed> movement_speeds = it.field<const MovementSpeed>(2);
            flecs::field<const DeathTimer> death_timers = it.field<const DeathTimer>(3);

            const std::int32_t row_count = static_cast<std::int32_t>(it.count());
            for (std::int32_t row_index = 0; row_index < row_count; ++row_index) {
                if (death_timers[static_cast<std::size_t>(row_index)].value > 0.0f) {
                    continue;
                }
                enemy_movement::BoidAccessor accessor{
                    &positions[static_cast<std::size_t>(row_index)],
                    &velocities[static_cast<std::size_t>(row_index)],
                    &movement_speeds[static_cast<std::size_t>(row_index)]
                };
                boids.push_back(accessor);
            }
        }

        const std::int32_t enemy_count = static_cast<std::int32_t>(boids.size());
        if (enemy_count == 0) {
            return;
        }

        const godot::real_t clamped_cell_size = godot::Math::max(movement_settings->grid_cell_size, 1.0f);
        const godot::real_t neighbor_radius_sq = movement_settings->neighbor_radius * movement_settings->neighbor_radius;
        const godot::real_t separation_radius_sq = movement_settings->separation_radius * movement_settings->separation_radius;
        const godot::real_t max_force_sq = movement_settings->max_force * movement_settings->max_force;
        const godot::real_t player_engage_radius_sq = movement_settings->player_engage_distance * movement_settings->player_engage_distance;

        const godot::real_t normalized_span = movement_settings->neighbor_radius / clamped_cell_size;
        const std::int32_t cell_span = static_cast<std::int32_t>(godot::Math::ceil(normalized_span));

        std::vector<enemy_spatial_hash::GridCellKey> entity_cells;
        enemy_spatial_hash::SpatialHash spatial_hash;
        enemy_spatial_hash::populate_spatial_hash(
            enemy_count,
            clamped_cell_size,
            [&boids](std::int32_t entity_index) {
            const std::size_t boid_index = static_cast<std::size_t>(entity_index);
            return boids[boid_index].position->value;
        },
            entity_cells,
            spatial_hash);

        for (std::int32_t entity_index = 0; entity_index < enemy_count; ++entity_index) {
            const godot::Vector2 position_value = boids[entity_index].position->value;
            const godot::Vector2 current_velocity = boids[entity_index].velocity->value;
            const godot::real_t max_speed = godot::Math::max(boids[entity_index].movement_speed->value * movement_settings->max_speed_multiplier, 1.0f);

            godot::Vector2 alignment_sum = godot::Vector2(0.0f, 0.0f);
            godot::Vector2 cohesion_sum = godot::Vector2(0.0f, 0.0f);
            godot::Vector2 separation_sum = godot::Vector2(0.0f, 0.0f);
            std::int32_t neighbor_count = 0;
            std::int32_t separation_count = 0;

            const enemy_spatial_hash::GridCellKey origin_cell = entity_cells[static_cast<std::size_t>(entity_index)];

            for (std::int32_t offset_x = -cell_span; offset_x <= cell_span; ++offset_x) {
                for (std::int32_t offset_y = -cell_span; offset_y <= cell_span; ++offset_y) {
                    const enemy_spatial_hash::GridCellKey neighbor_cell{ origin_cell.x + offset_x, origin_cell.y + offset_y };
                    const enemy_spatial_hash::SpatialHash::const_iterator found = spatial_hash.find(neighbor_cell);
                    if (found == spatial_hash.end()) {
                        continue;
                    }

                    const std::vector<std::int32_t>& occupants = found->second;
                    for (std::size_t occupant_index = 0; occupant_index < occupants.size(); ++occupant_index) {
                        const std::int32_t other_index = occupants[occupant_index];
                        if (other_index == entity_index) {
                            continue;
                        }

                        const godot::Vector2 offset = boids[other_index].position->value - position_value;
                        const godot::real_t distance_squared = offset.length_squared();
                        if (distance_squared > neighbor_radius_sq || distance_squared == 0.0f) {
                            continue;
                        }

                        neighbor_count += 1;
                        cohesion_sum += boids[other_index].position->value;
                        alignment_sum += boids[other_index].velocity->value;

                        if (distance_squared < separation_radius_sq) {
                            separation_sum -= offset / distance_squared;
                            separation_count += 1;
                        }
                    }
                }
            }

            godot::Vector2 alignment_force = godot::Vector2(0.0f, 0.0f);
            godot::Vector2 cohesion_force = godot::Vector2(0.0f, 0.0f);
            if (neighbor_count > 0) {
                const godot::Vector2 average_velocity = alignment_sum / static_cast<godot::real_t>(neighbor_count);
                alignment_force = enemy_movement::steer_towards(average_velocity, current_velocity, max_speed);

                const godot::Vector2 average_position = cohesion_sum / static_cast<godot::real_t>(neighbor_count);
                const godot::Vector2 direction_to_center = average_position - position_value;
                cohesion_force = enemy_movement::steer_towards(direction_to_center, current_velocity, max_speed);
            }

            godot::Vector2 separation_force = godot::Vector2(0.0f, 0.0f);
            if (separation_count > 0) {
                const godot::Vector2 average_push = separation_sum / static_cast<godot::real_t>(separation_count);
                separation_force = enemy_movement::steer_towards(average_push, current_velocity, max_speed);
            }

            const godot::Vector2 player_offset = player_position->value - position_value;
            godot::Vector2 player_force = enemy_movement::steer_towards(player_offset, current_velocity, max_speed);
            const godot::real_t distance_to_player_sq = player_offset.length_squared();
            if (distance_to_player_sq < player_engage_radius_sq && player_engage_radius_sq > 0.0f) {
                const godot::real_t distance_to_player = godot::Math::sqrt(distance_to_player_sq);
                const godot::real_t normalized_distance = distance_to_player / movement_settings->player_engage_distance;
                const godot::real_t slowdown_factor = godot::Math::clamp(normalized_distance, 0.2f, 1.0f);
                player_force *= slowdown_factor;
            }

            godot::Vector2 acceleration = alignment_force * force_weights->alignment_weight;
            acceleration += cohesion_force * force_weights->cohesion_weight;
            acceleration += separation_force * force_weights->separation_weight;
            acceleration += player_force * movement_settings->player_attraction_weight;

            if (acceleration.length_squared() > max_force_sq && max_force_sq > 0.0f) {
                acceleration = enemy_movement::limit_vector(acceleration, movement_settings->max_force);
            }

            godot::Vector2 new_velocity = current_velocity + acceleration * delta_time;
            if (new_velocity.length_squared() > max_speed * max_speed) {
                new_velocity = enemy_movement::limit_vector(new_velocity, max_speed);
            }

            boids[entity_index].velocity->value = new_velocity;
        }
    });
});
