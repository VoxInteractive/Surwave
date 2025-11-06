#include "systems/node_instantiation.h"
#include "systems/scene_instantiation.h"
#include "systems/scene_node_transform_sync.h"

FlecsRegistry register_all_systems([](flecs::world& world)
    {
        node_instantiation::registry_callback(world);
        scene_instantiation::registry_callback(world);
        scene_node_transform_sync::registry_callback(world);
    });
