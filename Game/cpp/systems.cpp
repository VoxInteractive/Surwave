#include "flecs_registry.h"

#include "systems/multimesh_update.h"

FlecsRegistry register_game_systems([](flecs::world& world)
{
    // multimesh_update(world);
});
