#include "entity_renderer.h"

#include <godot_cpp/core/class_db.hpp>

using godot::ClassDB;
using godot::D_METHOD;
using godot::PropertyInfo;
using godot::PROPERTY_HINT_NODE_PATH_VALID_TYPES;
using godot::Variant;

void EntityRenderer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_renderer_name", "name"), &EntityRenderer::set_renderer_name);
    ClassDB::bind_method(D_METHOD("get_renderer_name"), &EntityRenderer::get_renderer_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "renderer_name"), "set_renderer_name", "get_renderer_name");

    ClassDB::bind_method(D_METHOD("set_renderer_node_path", "path"), &EntityRenderer::set_renderer_node_path);
    ClassDB::bind_method(D_METHOD("get_renderer_node_path"), &EntityRenderer::get_renderer_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "renderer_node_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D,AudioStreamPlayer"), "set_renderer_node_path", "get_renderer_node_path");
}

void EntityRenderer::set_renderer_name(const godot::String& name) {
    renderer_name = name;
}

godot::String EntityRenderer::get_renderer_name() const {
    return renderer_name;
}

void EntityRenderer::set_renderer_node_path(const godot::NodePath& path) {
    renderer_node_path = path;
}

godot::NodePath EntityRenderer::get_renderer_node_path() const {
    return renderer_node_path;
}
