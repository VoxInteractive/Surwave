#include "../../src/flecs/registry.h"
#include "../../src/godot/systems/instantiation.h"
#include "systems/scene_node_transform_sync.h"

FlecsRegistry register_all_systems([](flecs::world& world)
{
    instantiation_system::registry_callback(world);
    scene_node_transform_sync::registry_callback(world);
});
