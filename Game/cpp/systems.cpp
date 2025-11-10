#include "systems/enemy_spawning.h"
#include "systems/multimesh_update.h"

FlecsRegistry register_all_systems([](flecs::world& world)
{
    enemy_spawning(world);
    multimesh_update(world);
});
