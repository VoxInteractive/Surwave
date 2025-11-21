#pragma once

#include "src/flecs_registry.h"

#include "components/enemy.h"
#include "components/enemy_state.h"


inline FlecsRegistry register_enemy_prefab([](flecs::world& world) {
    world.prefab("Enemy")
        .is_a(world.lookup("Character2D"))
        .set<Radius>({ 8.0f })
        .set<PlayerDetectionRadiusSquared>({ 20000.0f })
        .set<LoseTargetRadiusSquared>({ 24000.0f })
        .set<WanderMoveDuration>({ 3.0f })
        .set<RestDuration>({ 2.0f })
        .set_auto_override<HitPoints>({ 100.0f })
        .set_auto_override<MovementSpeed>({ 50.0f })
        .set_auto_override<MeleeDamage>({ 10.0f })
        .set_auto_override<TimeInState>({ 0.0f })
        .add<EnemyState, EnemyState::Idle>();
});
