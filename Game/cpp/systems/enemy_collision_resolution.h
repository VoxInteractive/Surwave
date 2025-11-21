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
    world.system<Velocity2D, const Position2D, const RadiusSquared>("Enemy Collision Resolution")
        .kind(flecs::OnValidate)
        .with(flecs::IsA, world.lookup("Enemy"))
        .run([](flecs::iter& it) {
        const int32_t count = it.count();
        if (count < 2) { return; }

        flecs::field<Velocity2D> velocities = it.field<Velocity2D>(0);
        flecs::field<const Position2D> positions = it.field<const Position2D>(1);
        flecs::field<const RadiusSquared> radii_sq = it.field<const RadiusSquared>(2);

        std::vector<EnemyCollisionEntry> entries;
        entries.reserve(static_cast<size_t>(count));
        for (int32_t i = 0; i < count; ++i) {
            EnemyCollisionEntry e; e.position = positions[i].value; e.velocity = &velocities[i]; e.radius_squared = radii_sq[i].value; e.cell_x = 0; e.cell_y = 0; entries.push_back(e);
        }
        EnemyCollisionCellMap cell_to_indices; cell_to_indices.reserve(entries.size() * 2U);

        const godot::real_t cell_size = godot::Math::sqrt(entries.front().radius_squared) * 2.0f;
        const godot::real_t inv_cell_size = 1.0f / cell_size;

        for (size_t idx = 0; idx < entries.size(); ++idx) {
            EnemyCollisionEntry& e = entries[idx];
            e.cell_x = static_cast<int32_t>(godot::Math::floor(e.position.x * inv_cell_size));
            e.cell_y = static_cast<int32_t>(godot::Math::floor(e.position.y * inv_cell_size));
            const int64_t key = enemy_collision_hash(e.cell_x, e.cell_y);
            cell_to_indices[key].push_back(idx);
        }

        for (size_t idx = 0; idx < entries.size(); ++idx) {
            EnemyCollisionEntry& e = entries[idx];
            for (int32_t dx = -1; dx <= 1; ++dx) {
                for (int32_t dy = -1; dy <= 1; ++dy) {
                    const int32_t nx = e.cell_x + dx; const int32_t ny = e.cell_y + dy; const int64_t key = enemy_collision_hash(nx, ny);
                    const EnemyCollisionCellMap::const_iterator cell_it = cell_to_indices.find(key);
                    if (cell_it == cell_to_indices.end()) { continue; }
                    const std::vector<size_t>& neighbor_indices = cell_it->second; const size_t neighbor_count = neighbor_indices.size();
                    for (size_t n = 0; n < neighbor_count; ++n) {
                        const size_t other_index = neighbor_indices[n]; if (other_index <= idx) { continue; }
                        EnemyCollisionEntry& other = entries[other_index];
                        const godot::Vector2 delta = other.position - e.position; const godot::real_t distance_sq = delta.length_squared();
                        const godot::real_t combined_radius = godot::Math::sqrt(e.radius_squared) + godot::Math::sqrt(other.radius_squared);
                        const godot::real_t combined_radius_sq = combined_radius * combined_radius; if (distance_sq >= combined_radius_sq) { continue; }
                        const godot::real_t safe_distance_sq = distance_sq > 0.0001f ? distance_sq : 0.0001f;
                        const godot::Vector2 relative_velocity = other.velocity->value - e.velocity->value; const godot::real_t approach_speed = relative_velocity.dot(delta);
                        if (approach_speed < 0.0f) { const godot::real_t impulse_mag = -approach_speed / (2.0f * safe_distance_sq); const godot::Vector2 impulse = delta * impulse_mag; e.velocity->value -= impulse; other.velocity->value += impulse; }
                        const godot::real_t overlap = combined_radius_sq - distance_sq; if (overlap > 0.0f) { const godot::real_t inv_len_sq = 1.0f / safe_distance_sq; const godot::Vector2 separation = delta * (overlap * 0.25f * inv_len_sq); e.velocity->value -= separation; other.velocity->value += separation; }
                    }
                }
            }
        }
    });
});
