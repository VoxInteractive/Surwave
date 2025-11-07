#include "flecs/registry.h"

#include "systems/scene_node_transform_sync.h"

FlecsRegistry register_all_systems([](flecs::world& world)
{
    scene_node_transform_sync::registry_callback(world);
});
