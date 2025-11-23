#pragma once

#include "src/flecs_registry.h"

#include "components/enemy.h"


inline FlecsRegistry register_enemy_prefab([](flecs::world& world) {
    world.prefab("Enemy")
        .is_a(world.lookup("Character2D"))
        .set<AnimationFrameOffset>({ 0.0f })
        .set_auto_override<HitPoints>({ 100.0f })
        .set<MeleeDamage>({ 10.0f })
        .set<MovementSpeed>({ 50.0f });
});
