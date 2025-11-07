#pragma once

#include <flecs.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>

#include "../../../src/flecs/registry.h"
#include "../../../src/godot/components/instantiation.h"
#include "../components/character.h"

using godot::Node;
using godot::Node2D;

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

            Node2D* node2d = Node::cast_to<Node2D>(scene_node.node);
            if (node2d != nullptr)
            {
                node2d->set_global_position(transform.position);
            } });
    }
} // namespace scene_node_transform_sync
