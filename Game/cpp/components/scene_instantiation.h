#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <flecs.h>

#include "../../../src/flecs_registry.h"

struct SceneInstantiationParentSingleton
{
    godot::Node* parent_node;
};

struct PackedSceneInstance
{
    godot::String scene_path;
    godot::String node_name;
    godot::Vector3 spawn_position;
    bool apply_spawn_position = false;
};

struct InstantiatedSceneNode
{
    godot::Node* node;
};

struct PendingSceneInstantiation
{
};


inline FlecsRegistry register_scene_instantiation_components([](flecs::world& world)
{
    world.component<SceneInstantiationParentSingleton>().add(flecs::Singleton);
    world.component<PackedSceneInstance>();
    world.component<InstantiatedSceneNode>();
    world.component<PendingSceneInstantiation>();
});
