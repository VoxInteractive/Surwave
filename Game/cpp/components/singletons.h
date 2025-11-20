#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/math_defs.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"


struct CharacterContactBeginDistanceSquared {
    godot::real_t value;
};

struct CharacterContactEndDistanceSquared {
    godot::real_t value;
};

struct EnemySeparationDistanceSquared {
    godot::real_t value;
};

struct EnemySpatialHashGrid {
    struct CellBucket {
        std::vector<uint32_t> entry_indices;
    };

    godot::real_t cell_size = 1.0f;
    godot::real_t inverse_cell_size = 1.0f;
    std::vector<godot::Vector2> positions;
    std::vector<flecs::entity_t> entity_ids;
    std::unordered_map<int64_t, CellBucket> buckets;
    std::vector<int64_t> active_cell_keys;

    void begin_build(godot::real_t new_cell_size) {
        godot::real_t safe_cell_size = new_cell_size;
        if (godot::Math::is_zero_approx(safe_cell_size) || safe_cell_size < 0.0f) {
            safe_cell_size = 1.0f;
        }

        cell_size = safe_cell_size;
        inverse_cell_size = 1.0f / cell_size;

        positions.clear();
        entity_ids.clear();

        for (int64_t key : active_cell_keys) {
            auto bucket_iter = buckets.find(key);
            if (bucket_iter != buckets.end()) {
                bucket_iter->second.entry_indices.clear();
            }
        }
        active_cell_keys.clear();
    }

    void reserve(std::size_t enemy_count) {
        positions.reserve(enemy_count);
        entity_ids.reserve(enemy_count);
        active_cell_keys.reserve(enemy_count);
        if (enemy_count > buckets.size()) {
            buckets.reserve(enemy_count);
        }
    }

    void insert(flecs::entity enemy, const godot::Vector2& position) {
        const uint32_t entry_index = static_cast<uint32_t>(positions.size());
        positions.push_back(position);
        entity_ids.push_back(enemy.id());

        const int32_t cell_x = to_cell_coordinate(position.x);
        const int32_t cell_y = to_cell_coordinate(position.y);
        append_to_bucket(cell_x, cell_y, entry_index);
    }

    template<typename Callback>
    void for_each_neighbor(const godot::Vector2& sample_position, godot::real_t radius, Callback&& callback) const {
        if (positions.empty()) {
            return;
        }

        const godot::real_t normalized_radius = radius * inverse_cell_size;
        const int32_t cell_radius = normalized_radius > 0.0f
            ? static_cast<int32_t>(godot::Math::ceil(normalized_radius))
            : 0;

        const int32_t base_cell_x = to_cell_coordinate(sample_position.x);
        const int32_t base_cell_y = to_cell_coordinate(sample_position.y);

        for (int32_t y_offset = -cell_radius; y_offset <= cell_radius; ++y_offset) {
            for (int32_t x_offset = -cell_radius; x_offset <= cell_radius; ++x_offset) {
                const int64_t key = make_key(base_cell_x + x_offset, base_cell_y + y_offset);
                auto bucket_iter = buckets.find(key);
                if (bucket_iter == buckets.end()) {
                    continue;
                }

                const CellBucket& bucket = bucket_iter->second;
                for (uint32_t entry_index : bucket.entry_indices) {
                    callback(entity_ids[entry_index], positions[entry_index]);
                }
            }
        }
    }

private:
    static int64_t make_key(int32_t cell_x, int32_t cell_y) {
        return (static_cast<int64_t>(cell_x) << 32) ^ static_cast<uint32_t>(cell_y);
    }

    int32_t to_cell_coordinate(godot::real_t value) const {
        return static_cast<int32_t>(godot::Math::floor(value * inverse_cell_size));
    }

    void append_to_bucket(int32_t cell_x, int32_t cell_y, uint32_t entry_index) {
        const int64_t key = make_key(cell_x, cell_y);
        CellBucket& bucket = buckets[key];
        if (bucket.entry_indices.empty()) {
            active_cell_keys.push_back(key);
        }
        bucket.entry_indices.push_back(entry_index);
    }
};


inline FlecsRegistry register_game_singleton_components([](flecs::world& world) {
    const godot::real_t enemy_radius = 8.0f;
    const godot::real_t contact_margin = 2.0f;
    const godot::real_t hysteresis = 2.0f; // This creates a stable region where the enemy will remain in the Attacking state, preventing flapping.

    world.component<CharacterContactBeginDistanceSquared>("CharacterContactBeginDistanceSquared")
        .member<godot::real_t>("value")
        .add(flecs::Singleton)
        .set<CharacterContactBeginDistanceSquared>({ godot::Math::pow(enemy_radius + contact_margin, 2.0f) });

    world.component<CharacterContactEndDistanceSquared>("CharacterContactEndDistanceSquared")
        .member<godot::real_t>("value")
        .add(flecs::Singleton)
        .set<CharacterContactEndDistanceSquared>({ godot::Math::pow(enemy_radius + contact_margin + hysteresis, 2.0f) });
});
