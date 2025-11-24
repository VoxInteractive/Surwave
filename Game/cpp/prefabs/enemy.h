#pragma once

#include "src/flecs_registry.h"

#include "components/enemy.h"


inline FlecsRegistry register_enemy_prefab([](flecs::world& world) {
    world.prefab("Enemy")
        .is_a(world.lookup("Character2D"))
        .set_auto_override<HitPoints>({ 100.0f })
        .set_auto_override<HitRadius>({ 14.0f })
        .set_auto_override<MeleeDamage>({ 10.0f })
        .set_auto_override<MovementSpeed>({ 50.0f })
        .set<AnimationFrameOffset>({ 0.0f })
        .set_auto_override<AnimationRandomOffset>({ 0.0f })
        .set_auto_override<DeathTimer>({ 0.0f })
        .set_auto_override<HFlipTimer>({ 0.5f })
        .set_auto_override<VFlipTimer>({ 0.5f });
});
