#include "flecs_registry.h"

#include "systems/prefab_instantiation.h"

FlecsRegistry register_base_systems([](flecs::world& world)
{
    prefab_instantiation(world);
});
