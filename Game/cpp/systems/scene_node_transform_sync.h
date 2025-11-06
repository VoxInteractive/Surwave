#pragma once

#include <flecs.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include "../../../src/flecs_registry.h"
#include "../components/character.h"
#include "../components/scene_instantiation.h"

using godot::Node;
using godot::Node3D;

namespace scene_node_transform_sync
{
    inline void registry_callback(flecs::world& world)
    {
        world.system<const LocalTransform, InstantiatedSceneNode>("Scene Node Transform Sync")
            .kind(flecs::PreStore)
            .each([](flecs::entity /*entity*/, const LocalTransform& transform, InstantiatedSceneNode& scene_node)
        {
            if (scene_node.node == nullptr)
            {
                return;
            }

            Node3D* node3d = Node::cast_to<Node3D>(scene_node.node);
            if (node3d != nullptr)
            {
                node3d->set_global_position(transform.position);
            } });
    }
} // namespace scene_node_transform_sync
