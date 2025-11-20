#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs.h>

using godot::Dictionary;
using godot::Node;

class FlecsWorld : public Node
{
    GDCLASS(FlecsWorld, Node)

public:
    FlecsWorld();

    // GDScript-visible methods that we'll bind
    void progress(double delta); // To be called every frame from GDScript attached to the FlecsWorld node. Make sure ecs_ftime_t matches the type of delta.
    void set_singleton_component(const godot::String& component_name, const godot::Variant& data);
    godot::Variant get_singleton_component(const godot::String& component_name);
    bool run_system(const godot::String& system_name, const godot::Dictionary& parameters); // For triggering on-demand (kind: 0) Flecs systems from GDScript

    // Virtual methods overridden from Node
    void _exit_tree() override;

    ~FlecsWorld();

protected:
    const flecs::world* get_world() const;
    void _notification(const int p_what);
    static void _bind_methods();

private:
    flecs::world world;
    bool is_initialised = false;
    std::unordered_map<std::string, std::function<void(const godot::Variant&)>> singleton_setters;
    std::unordered_map<std::string, std::function<godot::Variant(void)>> singleton_getters;
    void setup_entity_renderers();
    void update_physics_spaces();
};
