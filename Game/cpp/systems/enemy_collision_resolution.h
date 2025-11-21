#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/physics.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/enemy_state.h"

struct EnemyCollisionEntry
{
    godot::Vector2 position;
    Velocity2D* velocity;
    godot::real_t radius_squared;
    int32_t cell_x;
    int32_t cell_y;
};

using EnemyCollisionCellMap = std::unordered_map<int64_t, std::vector<size_t>>;

inline int64_t enemy_collision_hash(int32_t cell_x, int32_t cell_y)
{
    return (static_cast<int64_t>(cell_x) << 32) ^ static_cast<uint32_t>(cell_y);
}

inline FlecsRegistry register_enemy_collision_resolution_system([](flecs::world& world) {
    world.system<>("Enemy Collision Resolution")
        .kind(flecs::OnValidate)
        .run([](flecs::iter& it) {
        flecs::world world = it.world();
        static flecs::query<Velocity2D, const Position2D, const RadiusSquared> enemy_collision_query =
            world.query_builder<Velocity2D, const Position2D, const RadiusSquared>()
            .with(flecs::IsA, world.lookup("Enemy"))
            .build();
        std::vector<EnemyCollisionEntry> entries;
        EnemyCollisionCellMap cell_to_indices;

        entries.clear();
        cell_to_indices.clear();

        enemy_collision_query.run([&](flecs::iter& query_it) {
            const int32_t count = query_it.count();
            if (count <= 0) {
                return;
            }

            entries.reserve(entries.size() + static_cast<size_t>(count));

            flecs::field<Velocity2D> velocities = query_it.field<Velocity2D>(0);
            flecs::field<const Position2D> positions = query_it.field<const Position2D>(1);
            flecs::field<const RadiusSquared> radii_sq = query_it.field<const RadiusSquared>(2);

            for (int32_t idx = 0; idx < count; ++idx) {
                EnemyCollisionEntry entry;
                entry.position = positions[idx].value;
                entry.velocity = &velocities[idx];
                entry.radius_squared = radii_sq[idx].value;
                entry.cell_x = 0;
                entry.cell_y = 0;
                entries.push_back(entry);
            }
        });

        if (entries.size() < 2U) {
            return;
        }

        // Cell size should be based on the largest radius to ensure neighbors are captured.
        const godot::real_t cell_size = godot::Math::sqrt(entries.front().radius_squared) * 2.0f;
        const godot::real_t inv_cell_size = 1.0f / cell_size;

        cell_to_indices.reserve(entries.size() * 2U);

        for (size_t idx = 0; idx < entries.size(); ++idx) {
            EnemyCollisionEntry& entry = entries[idx];
            entry.cell_x = static_cast<int32_t>(godot::Math::floor(entry.position.x * inv_cell_size));
            entry.cell_y = static_cast<int32_t>(godot::Math::floor(entry.position.y * inv_cell_size));
            const int64_t key = enemy_collision_hash(entry.cell_x, entry.cell_y);
            cell_to_indices[key].push_back(idx);
        }

        for (size_t idx = 0; idx < entries.size(); ++idx) {
            EnemyCollisionEntry& entry = entries[idx];

            for (int32_t dx = -1; dx <= 1; ++dx) {
                for (int32_t dy = -1; dy <= 1; ++dy) {
                    const int32_t neighbor_x = entry.cell_x + dx;
                    const int32_t neighbor_y = entry.cell_y + dy;
                    const int64_t key = enemy_collision_hash(neighbor_x, neighbor_y);
                    const EnemyCollisionCellMap::const_iterator cell_iter = cell_to_indices.find(key);
                    if (cell_iter == cell_to_indices.end()) {
                        continue;
                    }

                    const std::vector<size_t>& neighbor_indices = cell_iter->second;
                    const size_t neighbor_count = neighbor_indices.size();

                    for (size_t neighbor_idx = 0; neighbor_idx < neighbor_count; ++neighbor_idx) {
                        const size_t other_index = neighbor_indices[neighbor_idx];
                        if (other_index <= idx) {
                            continue;
                        }

                        EnemyCollisionEntry& other = entries[other_index];
                        const godot::Vector2 delta = other.position - entry.position;
                        const godot::real_t distance_sq = delta.length_squared();
                        const godot::real_t combined_radius = godot::Math::sqrt(entry.radius_squared) + godot::Math::sqrt(other.radius_squared);
                        const godot::real_t combined_radius_sq = combined_radius * combined_radius;
                        if (distance_sq >= combined_radius_sq) {
                            continue;
                        }

                        const godot::real_t safe_distance_sq = distance_sq > 0.0001f ? distance_sq : 0.0001f;

                        const godot::Vector2 relative_velocity = other.velocity->value - entry.velocity->value;
                        const godot::real_t approach_speed = relative_velocity.dot(delta);
                        if (approach_speed < 0.0f) {
                            const godot::real_t impulse_mag = -approach_speed / (2.0f * safe_distance_sq);
                            const godot::Vector2 impulse = delta * impulse_mag;
                            entry.velocity->value -= impulse;
                            other.velocity->value += impulse;
                        }

                        const godot::real_t overlap = combined_radius_sq - distance_sq;
                        if (overlap > 0.0f) {
                            const godot::real_t inv_len_sq = 1.0f / safe_distance_sq;
                            const godot::Vector2 separation = delta * (overlap * 0.25f * inv_len_sq);
                            entry.velocity->value -= separation;
                            other.velocity->value += separation;
                        }
                    }
                }
            }
        }
    });
});
