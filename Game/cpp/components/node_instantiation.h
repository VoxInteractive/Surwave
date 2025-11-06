#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/string.hpp>

#include <flecs.h>

#include "../../../src/flecs_registry.h"

struct NodeInstantiationParentSingleton
{
    godot::Node* parent_node;
};

struct Node3DInstance
{
    godot::Node3D* node;
    godot::String node_name;
};

struct PendingInstantiation
{
};


inline FlecsRegistry register_node_instantiation_components([](flecs::world& world)
{
    world.component<NodeInstantiationParentSingleton>().add(flecs::Singleton);
    world.component<Node3DInstance>();
    world.component<PendingInstantiation>();
});
