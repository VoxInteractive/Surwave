#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"

#include "src/components/physics.h"
#include "src/components/transform.h"
#include "src/utilities/godot_signal.h"

#include "components/enemy.h"
#include "components/singletons.h"

inline FlecsRegistry register_enemy_death_system([](flecs::world& world) {
    world.system<const Position2D, HitPoints, DeathTimer, MeleeDamage, MovementSpeed, Velocity2D>("Enemy Death")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::OnValidate)
        .run([](flecs::iter& it) {
        const EnemyAnimationSettings* animation_settings = it.world().try_get<EnemyAnimationSettings>();
        if (animation_settings == nullptr) { return; }

        const godot::real_t death_animation_duration = animation_settings->animation_interval * animation_settings->death_animation_frame_count;
        const godot::real_t invulnerable_hit_points = kEnemyDeathInvulnerableHitPoints;

        while (it.next()) {
            flecs::field<const Position2D> positions = it.field<const Position2D>(0);
            flecs::field<HitPoints> hit_points = it.field<HitPoints>(1);
            flecs::field<DeathTimer> death_timer = it.field<DeathTimer>(2);
            flecs::field<MeleeDamage> melee_damage = it.field<MeleeDamage>(3);
            flecs::field<MovementSpeed> movement_speed = it.field<MovementSpeed>(4);
            flecs::field<Velocity2D> velocities = it.field<Velocity2D>(5);

            const std::size_t entity_count = it.count();
            for (std::size_t entity_index = 0; entity_index < entity_count; ++entity_index) {
                if (hit_points[entity_index].value > godot::real_t(0.0)) { continue; }

                flecs::entity entity = it.entity(static_cast<std::int32_t>(entity_index));
                godot::Dictionary signal_data;
                const flecs::entity prefab_entity = entity.target(flecs::IsA);
                signal_data["enemy_type"] = godot::String(prefab_entity.name().c_str());
                signal_data["enemy_position"] = positions[entity_index].value;
                emit_godot_signal(it.world(), entity, "enemy_died", signal_data);

                hit_points[entity_index].value = invulnerable_hit_points;
                death_timer[entity_index].value = death_animation_duration;
                melee_damage[entity_index].value = godot::real_t(0.0);
                movement_speed[entity_index].value = godot::real_t(0.0);
                velocities[entity_index].value = godot::Vector2(0.0f, 0.0f);
            }
        }
    });

});

