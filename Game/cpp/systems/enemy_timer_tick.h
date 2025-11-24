#pragma once

#include <cstddef>

#include <godot_cpp/core/math.hpp>

#include "src/flecs_registry.h"

#include "components/enemy.h"
#include "components/singletons.h"

inline FlecsRegistry register_enemy_timeout_tick_system([](flecs::world& world) {
    world.system<ProjectileHitTimeout, ShockwaveHitTimeout, DeathTimer, HFlipTimer, VFlipTimer>("Enemy Timer Tick")
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
            flecs::field<HFlipTimer> horizontal_flip_timers = it.field<HFlipTimer>(3);
            flecs::field<VFlipTimer> vertical_flip_timers = it.field<VFlipTimer>(4);
            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                ProjectileHitTimeout& projectile_timeout = projectile_timeouts[entity_index];
                ShockwaveHitTimeout& shockwave_timeout = shockwave_timeouts[entity_index];
                DeathTimer& death_timer = death_timers[entity_index];
                HFlipTimer& horizontal_timer = horizontal_flip_timers[entity_index];
                VFlipTimer& vertical_timer = vertical_flip_timers[entity_index];

                projectile_timeout.value = godot::Math::min(projectile_timeout.value + delta_time, projectile_cooldown);
                shockwave_timeout.value = godot::Math::min(shockwave_timeout.value + delta_time, shockwave_cooldown);

                if (death_timer.value > godot::real_t(0.0)) {
                    death_timer.value -= delta_time;
                    if (death_timer.value <= godot::real_t(0.0)) {
                        death_timer.value = godot::real_t(0.0);
                        it.entity(static_cast<std::int32_t>(entity_index)).destruct();
                        continue;
                    }
                }

                horizontal_timer.value = godot::Math::max(horizontal_timer.value + delta_time, godot::real_t(0.0));
                vertical_timer.value = godot::Math::max(vertical_timer.value + delta_time, godot::real_t(0.0));
            }
        }
    });
});
