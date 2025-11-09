#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

class EntityRenderer : public godot::Resource {
    GDCLASS(EntityRenderer, godot::Resource);

private:
    godot::String renderer_name;
    godot::NodePath renderer_node_path;

protected:
    static void _bind_methods();

public:
    void set_renderer_name(const godot::String& name);
    godot::String get_renderer_name() const;

    void set_renderer_node_path(const godot::NodePath& path);
    godot::NodePath get_renderer_node_path() const;
};
