#pragma once

#include <cstddef>
#include <cstdint>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/components/transform.h"
#include "src/components/player.h"

#include "components/enemy.h"
#include "components/singletons.h"
#include "src/utilities/godot_signal.h"

inline FlecsRegistry register_player_take_damage_system([](flecs::world& world) {
    world.system<const Position2D, const HitRadius, const MeleeDamage>("Player Take Damage")
        .with(flecs::IsA, world.lookup("Enemy"))
        .run([](flecs::iter& it) {
        const PlayerPosition* player_position = it.world().try_get<PlayerPosition>();
        PlayerDamageCooldown* player_damage_cooldown = it.world().try_get_mut<PlayerDamageCooldown>();
        const PlayerTakeDamageSettings* damage_settings = it.world().try_get<PlayerTakeDamageSettings>();
        if (player_position == nullptr || player_damage_cooldown == nullptr || damage_settings == nullptr) {
            return;
        }

        const godot::real_t cooldown = godot::Math::max(damage_settings->damage_cooldown, godot::real_t(0.0));
        const bool can_take_damage = cooldown <= godot::real_t(0.0) || player_damage_cooldown->value >= cooldown;
        if (!can_take_damage) {
            return;
        }

        const godot::Vector2 player_position_value = player_position->value;

        while (it.next()) {
            flecs::field<const Position2D> positions = it.field<const Position2D>(0);
            flecs::field<const HitRadius> hit_radii = it.field<const HitRadius>(1);
            flecs::field<const MeleeDamage> melee_damages = it.field<const MeleeDamage>(2);
            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                const godot::Vector2 enemy_position = positions[entity_index].value;
                const godot::Vector2 delta = player_position_value - enemy_position;
                const godot::real_t distance_squared = delta.length_squared();
                const godot::real_t contact_radius = godot::Math::max(hit_radii[entity_index].value, godot::real_t(1.0));
                const godot::real_t contact_radius_squared = contact_radius * contact_radius;
                if (distance_squared > contact_radius_squared) {
                    continue;
                }

                const godot::real_t damage_amount = godot::Math::max(melee_damages[entity_index].value, godot::real_t(0.0));
                if (damage_amount <= godot::real_t(0.0)) {
                    continue;
                }

                flecs::entity damaging_enemy = it.entity(static_cast<std::int32_t>(entity_index));
                godot::Dictionary signal_data;
                signal_data["entity_id"] = static_cast<int64_t>(damaging_enemy.id());
                signal_data["damage_amount"] = damage_amount;
                emit_godot_signal(it.world(), damaging_enemy, "player_took_damage", signal_data);

                player_damage_cooldown->value = godot::real_t(0.0);
                return;
            }
        }
    });
});
