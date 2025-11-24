#pragma once

#include "src/flecs_registry.h"

#include "components/enemy.h"


inline FlecsRegistry register_enemy_prefab([](flecs::world& world) {
    world.prefab("Enemy")
        .is_a(world.lookup("Character2D"))
        .set_auto_override<HitPoints>({ godot::real_t(100.0) })
        .set_auto_override<HitRadius>({ godot::real_t(14.0) })
        .set_auto_override<MeleeDamage>({ godot::real_t(10.0) })
        .set_auto_override<MovementSpeed>({ godot::real_t(50.0) })
        .set<AnimationFrameOffset>({ godot::real_t(0.0) })
        .set_auto_override<DeathTimer>({ godot::real_t(0.0) })
        .set_auto_override<HFlipTimer>({ godot::real_t(0.5) })
        .set_auto_override<VFlipTimer>({ godot::real_t(0.5) });
});
