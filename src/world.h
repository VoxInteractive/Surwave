#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include <flecs.h>
#include "singleton_setter_registry.h"

using godot::Dictionary;

class FlecsWorld : public godot::Node
{
    GDCLASS(FlecsWorld, Node)

public:
    FlecsWorld();

    void register_singleton_setter(const std::string& component_name, std::function<void(const Dictionary&)> setter);

    // GDScript-visible methods that we'll bind
    void progress(double delta); // To be called every frame from GDScript attached to the FlecsWorld node. Make sure ecs_ftime_t matches the type of delta.
    void set_singleton_component(const godot::String& component_name, const Dictionary& data);

    bool run_system(const godot::String& system_name); // For triggering on-demand (kind: 0) Flecs systems from GDScript

    void _exit_tree();

    ~FlecsWorld();

protected:
    const flecs::world* get_world() const;
    static void _bind_methods();

private:
    flecs::world world;
    std::unordered_map<std::string, std::function<void(const Dictionary&)>> singleton_setters;
    bool is_initialised = false;
};
