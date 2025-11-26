#pragma once

#include <cstddef>

#include <godot_cpp/core/math.hpp>

#include "src/flecs_registry.h"

#include "src/components/player.h"
#include "components/enemy.h"
#include "components/singletons.h"
#include "src/utilities/godot_signal.h"

inline FlecsRegistry register_enemy_timer_tick_system([](flecs::world& world) {
    world.system<ProjectileHitTimeout, ShockwaveHitTimeout, DeathTimer, HitReactionTimer, HFlipTimer, VFlipTimer>("Enemy Timer Tick")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::PreUpdate)
        .run([](flecs::iter& it) {
        const EnemyTakeDamageSettings* take_damage_settings = it.world().try_get<EnemyTakeDamageSettings>();
        const godot::real_t projectile_cooldown = take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->projectile_hit_cooldown, godot::real_t(0.0)) : godot::real_t(0.0);
        const godot::real_t shockwave_cooldown = take_damage_settings != nullptr ? godot::Math::max(take_damage_settings->shockwave_hit_cooldown, godot::real_t(0.0)) : godot::real_t(0.0);

        while (it.next()) {
            const godot::real_t delta_time = godot::Math::max(static_cast<godot::real_t>(it.delta_time()), godot::real_t(0.0));
            if (delta_time <= godot::real_t(0.0)) {
                continue;
            }

            flecs::field<ProjectileHitTimeout> projectile_timeouts = it.field<ProjectileHitTimeout>(0);
            flecs::field<ShockwaveHitTimeout> shockwave_timeouts = it.field<ShockwaveHitTimeout>(1);
            flecs::field<DeathTimer> death_timers = it.field<DeathTimer>(2);
            flecs::field<HitReactionTimer> hit_reaction_timers = it.field<HitReactionTimer>(3);
            flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(4);
            flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(5);
            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                ProjectileHitTimeout& projectile_timeout = projectile_timeouts[entity_index];
                ShockwaveHitTimeout& shockwave_timeout = shockwave_timeouts[entity_index];
                DeathTimer& death_timer = death_timers[entity_index];
                HitReactionTimer& hit_reaction_timer = hit_reaction_timers[entity_index];
                HFlipTimer& horizontal_timer = horizontal_flip_timers[entity_index];
                VFlipTimer& vertical_timer = vertical_flip_timers[entity_index];

                projectile_timeout.value = godot::Math::min(projectile_timeout.value + delta_time, projectile_cooldown);
                shockwave_timeout.value = godot::Math::min(shockwave_timeout.value + delta_time, shockwave_cooldown);

                if (death_timer.value > godot::real_t(0.0)) {
                    death_timer.value -= delta_time;
                    if (death_timer.value <= godot::real_t(0.0)) {
                        death_timer.value = godot::real_t(0.0);

                        flecs::entity entity = it.entity(static_cast<std::int32_t>(entity_index));

                        godot::Dictionary signal_data;
                        signal_data["entity_id"] = (int64_t)entity.id();
                        emit_godot_signal(it.world(), entity, "enemy_died", signal_data);

                        entity.destruct();
                        continue;
                    }
                }

                hit_reaction_timer.value = godot::Math::max(hit_reaction_timer.value - delta_time, godot::real_t(0.0));

                horizontal_timer.value = godot::Math::max(horizontal_timer.value + delta_time, godot::real_t(0.0));
                vertical_timer.value = godot::Math::max(vertical_timer.value + delta_time, godot::real_t(0.0));
            }
        }
    });

    world.system<PlayerDamageCooldown>("Player Damage Timer Tick")
        .kind(flecs::PreUpdate)
        .run([](flecs::iter& it) {
        const PlayerTakeDamageSettings* player_damage_settings = it.world().try_get<PlayerTakeDamageSettings>();
        if (player_damage_settings == nullptr) {
            return;
        }

        const godot::real_t cooldown = godot::Math::max(player_damage_settings->damage_cooldown, godot::real_t(0.0));

        while (it.next()) {
            const godot::real_t delta_time = godot::Math::max(static_cast<godot::real_t>(it.delta_time()), godot::real_t(0.0));
            flecs::field<PlayerDamageCooldown> cooldown_components = it.field<PlayerDamageCooldown>(0);
            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                PlayerDamageCooldown& cooldown_component = cooldown_components[entity_index];
                if (cooldown <= godot::real_t(0.0)) {
                    cooldown_component.value = cooldown;
                    continue;
                }

                if (delta_time <= godot::real_t(0.0)) {
                    continue;
                }

                cooldown_component.value = godot::Math::min(cooldown_component.value + delta_time, cooldown);
            }
        }
    });
});
