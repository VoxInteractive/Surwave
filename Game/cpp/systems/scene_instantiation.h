// This system's responsibility is to instantiate entire Godot Scenes from a PackedScene resource (typically a .tscn file)
#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs.h>

#include "../../../src/flecs_registry.h"
#include "../components/scene_instantiation.h"

namespace scene_instantiation
{
    inline void registry_callback(flecs::world &world)
    {
        world.system<const PackedSceneInstance>("Scene Instantiation")
            .with<PendingSceneInstantiation>()
            .kind(flecs::PreStore)
            .multi_threaded(false)
            .each([](flecs::entity entity, const PackedSceneInstance &instance)
                  {
            flecs::world world_handle = entity.world();
            const SceneInstantiationParentSingleton* parent_singleton = world_handle.try_get<const SceneInstantiationParentSingleton>();
            if (parent_singleton == nullptr || parent_singleton->parent_node == nullptr)
            {
                godot::UtilityFunctions::push_warning(godot::String("Scene Instantiation: SceneInstantiationParentSingleton singleton is missing or parent_node is null"));
                return;
            }

            godot::Node* parent_node = parent_singleton->parent_node;
            if (instance.scene_path.is_empty())
            {
                godot::UtilityFunctions::push_warning(godot::String("Scene Instantiation: PackedSceneInstance.scene_path is empty; cannot instantiate scene"));
                entity.remove<PendingSceneInstantiation>();
                return;
            }

            godot::String full_path = godot::String("res://") + instance.scene_path;
            godot::Ref<godot::PackedScene> packed_scene = godot::ResourceLoader::get_singleton()->load(full_path);
            if (packed_scene.is_null())
            {
                godot::UtilityFunctions::push_warning(godot::String("Scene Instantiation: Failed to load scene at path: ") + full_path);
                entity.remove<PendingSceneInstantiation>();
                return;
            }

            godot::Node* instantiated_node = packed_scene->instantiate();
            if (instantiated_node == nullptr)
            {
                godot::UtilityFunctions::push_warning(godot::String("Scene Instantiation: Failed to instantiate scene from path: ") + full_path);
                entity.remove<PendingSceneInstantiation>();
                return;
            }

            if (!instance.node_name.is_empty())
            {
                instantiated_node->set_name(instance.node_name);
            }

            parent_node->add_child(instantiated_node);

            InstantiatedSceneNode instantiated_component;
            instantiated_component.node = instantiated_node;
            entity.set<InstantiatedSceneNode>(instantiated_component);

            entity.remove<PendingSceneInstantiation>(); });
    }
} // namespace scene_instantiation
