#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

#include <flecs.h>
#include "flecs/registry.h"

// A singleton component to hold the parent node for all instantiations.
struct InstantiationParentSingleton
{
    godot::Node* parent_node;
};

// A request to instantiate either a scene from a path or a pre-existing node.
struct InstantiationRequest
{
    godot::String scene_path;
    godot::Node* node{ nullptr };
    godot::String node_name;
};

// A component to hold the root node of an instantiated scene.
struct InstantiatedSceneNode
{
    godot::Node* node;
};

// A tag to mark an entity as pending instantiation.
struct PendingInstantiation {};

inline FlecsRegistry register_instantiation_components([](flecs::world& world)
{
    world.component<InstantiationParentSingleton>().add(flecs::Singleton);
    world.component<InstantiationRequest>();
    world.component<InstantiatedSceneNode>();
    world.component<PendingInstantiation>();
});
