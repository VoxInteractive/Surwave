#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"

#include "src/components/physics.h"

#include "components/enemy.h"
#include "components/singletons.h"

inline FlecsRegistry register_enemy_death_system([](flecs::world& world) {
    world.system<HitPoints, DeathTimer, MeleeDamage, MovementSpeed, Velocity2D>("Enemy Death")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::OnValidate)
        .run([](flecs::iter& it) {
        const EnemyAnimationSettings* animation_settings = it.world().try_get<EnemyAnimationSettings>();
        if (animation_settings == nullptr) { return; }

        const float death_animation_duration = animation_settings->animation_interval * animation_settings->death_animation_frame_count;
        const float invulnerable_hit_points = kEnemyDeathInvulnerableHitPoints;

        while (it.next()) {
            flecs::field<HitPoints> hit_points = it.field<HitPoints>(0);
            flecs::field<DeathTimer> death_timer = it.field<DeathTimer>(1);
            flecs::field<MeleeDamage> melee_damage = it.field<MeleeDamage>(2);
            flecs::field<MovementSpeed> movement_speed = it.field<MovementSpeed>(3);
            flecs::field<Velocity2D> velocities = it.field<Velocity2D>(4);

            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                if (hit_points[entity_index].value > 0.0f) { continue; }

                hit_points[entity_index].value = invulnerable_hit_points;
                death_timer[entity_index].value = death_animation_duration;
                melee_damage[entity_index].value = 0.0f;
                movement_speed[entity_index].value = 0.0f;
                velocities[entity_index].value = godot::Vector2(0.0f, 0.0f);
            }
        }
    });

    world.system<DeathTimer>("Enemy Death Timer")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::OnUpdate)
        .run([](flecs::iter& it) {
        while (it.next()) {
            const float delta_time = static_cast<float>(it.delta_time());
            if (delta_time <= 0.0f) {
                continue;
            }

            flecs::field<DeathTimer> death_timer = it.field<DeathTimer>(0);
            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                float& timer_value = death_timer[entity_index].value;
                // Timer already expired on a previous frame, nothing to update.
                if (timer_value <= 0.0f) { continue; }

                timer_value -= delta_time;
                if (timer_value <= 0.0f) {
                    // Clamp before destruction so other systems observing this tick never see negatives.
                    timer_value = 0.0f;
                    it.entity(static_cast<std::int32_t>(entity_index)).destruct();
                }
            }
        }
    });
});

