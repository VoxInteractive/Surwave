#pragma once

#include "src/flecs_registry.h"

#include "components/singletons.h"

inline FlecsRegistry register_enemy_count_update_system([](flecs::world& world) {
    world.system<>("Enemy Count Update")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::PostUpdate)
        .run([](flecs::iter& it) {

        flecs::world world = it.world();
        size_t current_enemy_count = 0;

        while (it.next()) {
            current_enemy_count += it.count();
        }

        const EnemyCount* singleton_component = world.try_get<EnemyCount>();
        if (singleton_component) {
            if (singleton_component->value != current_enemy_count) {
                world.set<EnemyCount>({ current_enemy_count });
            }
        }
    });
});
