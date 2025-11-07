// This system's responsibility is to create single, individual Godot Nodes.
#pragma once

#include <flecs.h>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../../../src/flecs_registry.h"
#include "../components/node_instantiation.h"

namespace node_instantiation
{
    inline void registry_callback(flecs::world& world)
    {
        world.system<const NodeInstance>("Node Instantiation")
            .with<PendingInstantiation>()
            .kind(flecs::PreStore)
            .multi_threaded(false)
            .each([](flecs::entity entity, const NodeInstance& instance)
        {
            flecs::world world_handle = entity.world();
            // NodeInstantiationParentSingleton is a world singleton component now.
            const NodeInstantiationParentSingleton* parent_singleton = world_handle.try_get<const NodeInstantiationParentSingleton>();
            if (parent_singleton == nullptr || parent_singleton->parent_node == nullptr)
            {
                godot::UtilityFunctions::push_warning(godot::String("Node Instantiation: NodeInstantiationParentSingleton singleton is missing or parent_node is null"));
                return;
            }

            godot::Node* parent_node = parent_singleton->parent_node;
            if (instance.node != nullptr)
            {
                godot::Node* node = instance.node;
                if (!instance.node_name.is_empty())
                {
                    node->set_name(instance.node_name);
                }
                parent_node->add_child(node);
            }
            else
            {
                godot::UtilityFunctions::push_warning(godot::String("Node Instantiation: NodeInstance.node is null; cannot instantiate node"));
            }

            entity.remove<PendingInstantiation>(); });
    }
} // namespace node_instantiation
