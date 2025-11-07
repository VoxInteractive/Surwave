#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs.h>
#include "../../flecs/registry.h"
#include "../components/instantiation.h"

namespace instantiation_system
{
    inline void registry_callback(flecs::world& world)
    {
        world.system<const InstantiationRequest>("Instantiation System")
            .with<PendingInstantiation>()
            .kind(flecs::PreStore)
            .multi_threaded(false) // Interaction with Godot's scene system can only safely happen on the main thread
            .each([](flecs::entity entity, const InstantiationRequest& request)
        {
            flecs::world world_handle = entity.world();
            const InstantiationParentSingleton* parent_singleton = world_handle.try_get<const InstantiationParentSingleton>();
            if (parent_singleton == nullptr || parent_singleton->parent_node == nullptr)
            {
                godot::UtilityFunctions::push_warning(godot::String("Instantiation System: InstantiationParentSingleton is missing or parent_node is null"));
                return;
            }

            godot::Node* parent_node = parent_singleton->parent_node;
            godot::Node* instantiated_node = nullptr;

            // Case 1: Instantiate a scene from a path
            if (!request.scene_path.is_empty())
            {
                godot::String full_path = godot::String("res://") + request.scene_path;
                godot::Ref<godot::PackedScene> packed_scene = godot::ResourceLoader::get_singleton()->load(full_path);
                if (packed_scene.is_null())
                {
                    godot::UtilityFunctions::push_warning(godot::String("Instantiation System: Failed to load scene at path: ") + full_path);
                    entity.remove<PendingInstantiation>();
                    return;
                }

                instantiated_node = packed_scene->instantiate();
                if (instantiated_node == nullptr)
                {
                    godot::UtilityFunctions::push_warning(godot::String("Instantiation System: Failed to instantiate scene from path: ") + full_path);
                    entity.remove<PendingInstantiation>();
                    return;
                }
                // This component is used to sync transforms to the instantiated scene's root node
                entity.set<InstantiatedSceneNode>({ instantiated_node });
            }
            // Case 2: Use a pre-existing node
            else if (request.node != nullptr)
            {
                instantiated_node = request.node;
            }

            if (instantiated_node != nullptr)
            {
                if (!request.node_name.is_empty())
                {
                    instantiated_node->set_name(request.node_name);
                }
                parent_node->add_child(instantiated_node);
            }
            else
            {
                godot::UtilityFunctions::push_warning(godot::String("Instantiation System: InstantiationRequest has neither a valid scene_path nor a node pointer."));
            }

            entity.remove<PendingInstantiation>();
        });
    }
} // namespace instantiation_system
