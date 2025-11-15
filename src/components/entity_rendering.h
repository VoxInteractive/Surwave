#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>

#include "flecs_registry.h"
#include "../utilities/godot_hashes.h"


enum class RendererType {
    MultiMesh,
};

struct MultiMeshRenderer
{
    godot::RID rid;
    std::vector<flecs::query<>> queries; // One MultiMeshInstance can render multiple prefab types. Store a list of queries (one per prefab) for each renderer.
    godot::MultiMesh::TransformFormat transform_format;
    bool use_colors;
    bool use_custom_data;
    size_t instance_count;
    size_t visible_instance_count;
};

struct RenderingColor {
    float r;
    float g;
    float b;
    float a;
};

struct RenderingCustomData {
    float r;
    float g;
    float b;
    float a;
};

struct EntityRenderers
{
    // Map from renderer type to a map of prefab names to multimesh RIDs.
    // Key for the inner map is a string representation of the MultiMesh RID ID (we group all prefab queries for a single MultiMesh into one MultiMeshRenderer).
    // Use godot::RID as the inner map key so each MultiMesh RID maps directly to its MultiMeshRenderer. Ensure std::hash<godot::RID> is available below.
    std::unordered_map<RendererType, std::unordered_map<godot::RID, MultiMeshRenderer>> renderers_by_type;
};


inline FlecsRegistry register_entity_rendering_components([](flecs::world& world) {
    world.component<EntityRenderers>().add(flecs::Singleton);

    world.component<RenderingColor>("RenderingColor")
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");

    world.component<RenderingCustomData>("RenderingCustomData")
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");
});
