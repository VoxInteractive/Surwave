#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs.h>

using godot::Dictionary;

class FlecsWorld : public godot::Node
{
    GDCLASS(FlecsWorld, Node)

public:
    FlecsWorld();

    // GDScript-visible methods that we'll bind
    void progress(double delta); // To be called every frame from GDScript attached to the FlecsWorld node. Make sure ecs_ftime_t matches the type of delta.
    void set_singleton_component(const godot::String& component_name, const Dictionary& data);
    bool run_system(const godot::String& system_name); // For triggering on-demand (kind: 0) Flecs systems from GDScript

    void set_entity_renderers(const godot::Array& p_renderers);
    godot::Array get_entity_renderers() const;

    void _ready() override;
    void _exit_tree() override;

    ~FlecsWorld();

protected:
    const flecs::world* get_world() const;
    static void _bind_methods();
    virtual void _notification(int p_what);

private:
    flecs::world world;
    bool is_initialised = false;
    godot::Array entity_renderers;

    void register_components_for_godot_variants();
    void register_custom_components();
    void process_renderer_mappings();
};
