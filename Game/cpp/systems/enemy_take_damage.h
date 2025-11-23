#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/transform.h"

#include "components/enemy.h"
#include "components/singletons.h"
#include "systems/enemy_spatial_hash.h"

namespace enemy_take_damage_detail {

    struct DamageTargetAccessor {
        Position2D* position;
        HitPoints* hit_points;
    };

    inline godot::Array get_projectile_positions(const ProjectileData* projectile_data) {
        if (projectile_data == nullptr) {
            return godot::Array();
        }

        const godot::StringName positions_key = "projectile_positions";

        if (!projectile_data->value.has(positions_key)) {
            return godot::Array();
        }

        const godot::Variant positions_variant = projectile_data->value.get(positions_key, godot::Variant());
        if (positions_variant.get_type() != godot::Variant::ARRAY) {
            return godot::Array();
        }

        return positions_variant;
    }

} // namespace enemy_take_damage_detail

inline FlecsRegistry register_enemy_take_damage_system([](flecs::world& world) {
    world.system<Position2D, HitPoints>("Enemy Take Damage")
        .with(flecs::IsA, world.lookup("Enemy"))
        .run([](flecs::iter& it) {
        flecs::world stage_world = it.world();
        const ProjectileData* projectile_data = stage_world.try_get<ProjectileData>();
        const EnemyBoidMovementSettings* movement_settings = stage_world.try_get<EnemyBoidMovementSettings>();
        if (projectile_data == nullptr || movement_settings == nullptr) {
            return;
        }

        const godot::Array projectile_positions = enemy_take_damage_detail::get_projectile_positions(projectile_data);
        const std::int32_t projectile_count = static_cast<std::int32_t>(projectile_positions.size());
        if (projectile_count == 0) {
            return;
        }

        std::vector<enemy_take_damage_detail::DamageTargetAccessor> targets;
        targets.reserve(128U);

        while (it.next()) {
            flecs::field<Position2D> positions = it.field<Position2D>(0);
            flecs::field<HitPoints> hit_points = it.field<HitPoints>(1);
            const std::int32_t row_count = static_cast<std::int32_t>(it.count());
            for (std::int32_t row_index = 0; row_index < row_count; ++row_index) {
                enemy_take_damage_detail::DamageTargetAccessor accessor{
                    &positions[static_cast<std::size_t>(row_index)],
                    &hit_points[static_cast<std::size_t>(row_index)]
                };
                targets.push_back(accessor);
            }
        }

        const std::int32_t enemy_count = static_cast<std::int32_t>(targets.size());
        if (enemy_count == 0) {
            return;
        }

        const godot::real_t hit_radius = godot::Math::max(movement_settings->separation_radius, 1.0f);
        const godot::real_t hit_radius_sq = hit_radius * hit_radius;
        const godot::real_t clamped_cell_size = godot::Math::max(movement_settings->grid_cell_size, 1.0f);
        const godot::real_t normalized_span = hit_radius / clamped_cell_size;
        const std::int32_t cell_span = static_cast<std::int32_t>(godot::Math::ceil(normalized_span));

        std::vector<enemy_spatial_hash::GridCellKey> entity_cells;
        enemy_spatial_hash::SpatialHash spatial_hash;
        enemy_spatial_hash::populate_spatial_hash(
            enemy_count,
            clamped_cell_size,
            [&targets](std::int32_t entity_index) {
            const std::size_t target_index = static_cast<std::size_t>(entity_index);
            return targets[target_index].position->value;
        },
            entity_cells,
            spatial_hash);

        std::vector<float> accumulated_damage(static_cast<std::size_t>(enemy_count), 0.0f);

        for (std::int32_t projectile_index = 0; projectile_index < projectile_count; ++projectile_index) {
            const godot::Variant projectile_variant = projectile_positions[projectile_index];
            if (projectile_variant.get_type() != godot::Variant::VECTOR2) {
                continue;
            }

            const godot::Vector2 projectile_position = projectile_variant;
            const enemy_spatial_hash::GridCellKey projectile_cell = enemy_spatial_hash::make_key(projectile_position, clamped_cell_size);

            for (std::int32_t offset_x = -cell_span; offset_x <= cell_span; ++offset_x) {
                for (std::int32_t offset_y = -cell_span; offset_y <= cell_span; ++offset_y) {
                    const enemy_spatial_hash::GridCellKey neighbor_cell{ projectile_cell.x + offset_x, projectile_cell.y + offset_y };
                    const enemy_spatial_hash::SpatialHash::const_iterator found = spatial_hash.find(neighbor_cell);
                    if (found == spatial_hash.end()) {
                        continue;
                    }

                    const enemy_spatial_hash::CellOccupants& occupants = found->second;
                    const std::size_t occupant_count = occupants.size();
                    for (std::size_t occupant_index = 0; occupant_index < occupant_count; ++occupant_index) {
                        const std::int32_t enemy_index = occupants[occupant_index];
                        const godot::Vector2 enemy_position = targets[static_cast<std::size_t>(enemy_index)].position->value;
                        const godot::Vector2 delta = enemy_position - projectile_position;
                        const godot::real_t distance_squared = delta.length_squared();
                        if (distance_squared <= hit_radius_sq) {
                            accumulated_damage[static_cast<std::size_t>(enemy_index)] += 1.0f;
                        }
                    }
                }
            }
        }

        for (std::int32_t enemy_index = 0; enemy_index < enemy_count; ++enemy_index) {
            const float damage = accumulated_damage[static_cast<std::size_t>(enemy_index)];
            if (damage <= 0.0f) {
                continue;
            }

            targets[static_cast<std::size_t>(enemy_index)].hit_points->value -= damage;
        }
    });
});
