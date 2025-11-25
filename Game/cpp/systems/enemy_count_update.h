#pragma once

#include "src/flecs_registry.h"

#include "components/singletons.h"

inline FlecsRegistry register_enemy_count_update_system([](flecs::world& world) {
    const flecs::entity enemy_prefab = world.lookup("Enemy");
    const flecs::query<> enemy_instance_query = world.query_builder<>()
        .with(flecs::IsA, enemy_prefab)
        .build();

    world.system<>("Enemy Count Update")
        .kind(flecs::PostUpdate)
        .run([enemy_instance_query](flecs::iter& it) {
        flecs::world stage_world = it.world();
        const size_t current_enemy_count = static_cast<size_t>(
            enemy_instance_query.iter(stage_world.c_ptr()).count());

        const EnemyCount* singleton_component = stage_world.try_get<EnemyCount>();
        if (singleton_component == nullptr || singleton_component->value != current_enemy_count) {
            stage_world.set<EnemyCount>({ current_enemy_count });
        }
    });
});
