#pragma once

#include <functional>
#include <map>
#include <string>
#include <unordered_map>

#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs.h>

using godot::Dictionary;
using godot::Node;
using godot::RID;

struct EntityRenderers
{
    std::map<std::string, std::map<std::string, RID>> renderers_by_type;
};

class FlecsWorld : public Node
{
    GDCLASS(FlecsWorld, Node)

public:
    FlecsWorld();

    // GDScript-visible methods that we'll bind
    void progress(double delta); // To be called every frame from GDScript attached to the FlecsWorld node. Make sure ecs_ftime_t matches the type of delta.
    void set_singleton_component(const godot::String& component_name, const Dictionary& data);
    bool run_system(const godot::String& system_name); // For triggering on-demand (kind: 0) Flecs systems from GDScript

    void _exit_tree() override;

    ~FlecsWorld();

protected:
    const flecs::world* get_world() const;
    void _notification(const int p_what);
    static void _bind_methods();

private:
    flecs::world world;
    bool is_initialised = false;
    void setup_entity_renderers();
};
