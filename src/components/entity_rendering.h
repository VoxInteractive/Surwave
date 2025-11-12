#pragma once

#include <unordered_map>
#include <string>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>

#include "flecs_registry.h"

enum class RendererType {
    MultiMesh,
};

struct MultiMeshRenderer
{
    godot::RID rid;
    godot::MultiMesh::TransformFormat transform_format;
    bool use_colors;
    bool use_custom_data;
    int instance_count;
    int visible_instance_count;
};

struct RenderingColor {
    godot::Color value;
};

struct RenderingCustomData {
    godot::Color value;
};

struct EntityRenderers
{
    // Map from renderer type to a map of prefab names to multimesh RIDs
    std::unordered_map<RendererType, std::unordered_map<std::string, MultiMeshRenderer>> renderers_by_type;
};

inline FlecsRegistry register_entity_renderers_component([](flecs::world& world) {
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
