#pragma once

#include <cstddef>
#include <limits>

#include "src/flecs_registry.h"

#include "components/enemy.h"
#include "components/singletons.h"

inline FlecsRegistry register_enemy_death_system([](flecs::world& world) {
    world.system<HitPoints, DeathTimer, MeleeDamage, MovementSpeed>("Enemy Death")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::OnValidate)
        .run([](flecs::iter& it) {
        const EnemyAnimationSettings* animation_settings = it.world().try_get<EnemyAnimationSettings>();
        if (animation_settings == nullptr) { return; }

        const float death_animation_duration = animation_settings->animation_interval * animation_settings->death_animation_frame_count;
        const float invulnerable_hit_points = std::numeric_limits<float>::max();

        while (it.next()) {
            flecs::field<HitPoints> hit_points = it.field<HitPoints>(0);
            flecs::field<DeathTimer> death_timer = it.field<DeathTimer>(1);
            flecs::field<MeleeDamage> melee_damage = it.field<MeleeDamage>(2);
            flecs::field<MovementSpeed> movement_speed = it.field<MovementSpeed>(3);

            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                if (hit_points[entity_index].value > 0.0f) { continue; }

                hit_points[entity_index].value = invulnerable_hit_points;
                death_timer[entity_index].value = death_animation_duration;
                melee_damage[entity_index].value = 0.0f;
                movement_speed[entity_index].value = 0.0f;
            }
        }
    });
});

