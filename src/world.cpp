#include <thread>

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "components.h"
#include "entity_renderer.h"
#include "scripts_loader.h"
#include "utilities.h"
#include "world.h"

using godot::AudioStreamPlayer;
using godot::MultiMeshInstance3D;

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::UtilityFunctions;

FlecsWorld::FlecsWorld()
{
    // Constructor is lightweight. Main initialization happens in _ready().
}

void FlecsWorld::_ready()
{
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    if (is_initialised)
    {
        return;
    }

    // Enable Flecs REST, statistics and extra logging verbosity in debug builds
#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
    world.set<flecs::Rest>({});
    world.import <flecs::stats>();
    flecs::log::set_level(0); // 0 = default, 1 = info
#endif

    // Set the number of threads Flecs should use based on CPU thread count
    unsigned int num_threads = ::project::Utilities::get_thread_count();
    world.set_threads(static_cast<int>(num_threads));

    // Register components
    register_components_for_godot_variants();
    register_custom_components();

    // Load Flecs script files that live in the project's flecs_scripts folder.
    // Use a Godot resource path so the loader can resolve it via ProjectSettings.
    FlecsScriptsLoader loader;
    loader.load(world);

    // Process the renderer mappings from the editor
    process_renderer_mappings();

    is_initialised = true;
}

void FlecsWorld::_notification(int p_what)
{
    if (p_what == NOTIFICATION_READY)
        _ready();
}

void FlecsWorld::register_components_for_godot_variants()
{
    world.component<godot::Color>("Color") // 16 bytes
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");

    world.component<godot::Vector2>("Vector2") // 8 bytes
        .member<float>("x")
        .member<float>("y");

    world.component<godot::Vector2i>("Vector2i") // 8 bytes
        .member<int32_t>("x")
        .member<int32_t>("y");

    world.component<godot::Vector3>("Vector3") // 12 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");

    world.component<godot::Vector3i>("Vector3i") // 12 bytes
        .member<int32_t>("x")
        .member<int32_t>("y")
        .member<int32_t>("z");

    world.component<godot::Vector4>("Vector4") // 16 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z")
        .member<float>("w");

    world.component<godot::Vector4i>("Vector4i") // 16 bytes
        .member<int32_t>("x")
        .member<int32_t>("y")
        .member<int32_t>("z")
        .member<int32_t>("w");

    world.component<godot::Rect2>("Rect2") // 16 bytes
        .member<godot::Vector2>("position")
        .member<godot::Vector2>("size");

    world.component<godot::Rect2i>("Rect2i") // 16 bytes
        .member<godot::Vector2i>("position")
        .member<godot::Vector2i>("size");

    world.component<godot::Plane>("Plane") // 16 bytes
        .member<godot::Vector3>("normal")
        .member<float>("d");

    world.component<godot::Quaternion>("Quaternion") // 16 bytes
        .member<float>("x")
        .member<float>("y")
        .member<float>("z")
        .member<float>("w");

    world.component<godot::Basis>("Basis") // 36 bytes - acceptable
        .member<godot::Vector3>("rows", 3);

    world.component<godot::Transform2D>("Transform2D") // 24 bytes
        .member<godot::Vector2>("columns", 3);

    world.component<godot::Transform3D>("Transform3D") // 48 bytes - borderline large, but acceptable for transform components
        .member<godot::Basis>("basis")
        .member<godot::Vector3>("origin");

    world.component<godot::AABB>("AABB") // 24 bytes
        .member<godot::Vector3>("position")
        .member<godot::Vector3>("size");

    world.component<godot::Projection>("Projection") // 64 bytes - large, use sparingly
        .member<godot::Vector4>("columns", 4);
}

void FlecsWorld::register_custom_components()
{
    world.component<GodotRenderer>("GodotRenderer")
        .member<const char*>("name");

    world.component<VisualRenderer>("VisualRenderer")
        .member<godot::MultiMeshInstance3D*>("renderer_node");
}

void FlecsWorld::process_renderer_mappings()
{
    // 1. Build a map of logical names to Godot node pointers from the inspector
    std::unordered_map<std::string, godot::Node*> renderer_nodes;
    for (int i = 0; i < entity_renderers.size(); ++i)
    {
        godot::Ref<EntityRenderer> mapping = entity_renderers[i];
        if (mapping.is_valid() && !mapping->get_renderer_name().is_empty() && !mapping->get_renderer_node_path().is_empty())
        {
            godot::Node* node = get_node_or_null(mapping->get_renderer_node_path());
            if (node)
            {
                renderer_nodes[mapping->get_renderer_name().utf8().get_data()] = node;
            }
        }
    }

    // 2. Query for prefabs with GodotRenderer and set the final VisualRenderer component
    world.each([&](flecs::entity e, const GodotRenderer& gr) {
        auto it = renderer_nodes.find(gr.name);
        if (it != renderer_nodes.end())
        {
            godot::Node* node = it->second;

            // Check the type of the node and set the appropriate component
            if (auto* visual_node = godot::Object::cast_to<MultiMeshInstance3D>(node))
            {
                e.set<VisualRenderer>({ visual_node });
            }
            // NOTE: An AudioStreamPlayer check could be added here for an AudioRenderer component
        }
        else {
            UtilityFunctions::push_warning("Flecs: Renderer name '", gr.name.c_str(), "' used in script but not mapped in FlecsWorld inspector.");
        }
    });
}

const flecs::world* FlecsWorld::get_world() const
{
    if (!is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld::get_world was called before world was initialised, returning null pointer"));
        return nullptr;
    }

    return &world;
}


void FlecsWorld::set_singleton_component(const godot::String& component_name, const Dictionary& data)
{
    if (!is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld::set_singleton_component was called before world was initialised"));
        return;
    }

    std::string name = component_name.utf8().get_data();
    // TODO: Implement actual data assignment
}


void FlecsWorld::progress(double delta)
{
    if (!is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld::progress was called before world was initialised"));
        return;
    }

    world.progress(delta);
}

void FlecsWorld::set_entity_renderers(const godot::Array& p_renderers)
{
    entity_renderers = p_renderers;
}

godot::Array FlecsWorld::get_entity_renderers() const
{
    return entity_renderers;
}


bool FlecsWorld::run_system(const godot::String& system_name)
{
    std::string name = system_name.utf8().get_data();

    // Lookup the entity by name, then request a system handle from the world
    // using that entity. This avoids matching the system_builder overload.
    flecs::entity entity = world.lookup(name.c_str());
    if (!entity.is_valid())
    {
        return false;
    }
    flecs::system sys = world.system(entity);
    if (!sys.is_valid())
    {
        return false;
    }

    sys.run();
    return true;
}

void FlecsWorld::_exit_tree()
{
    if (!is_initialised)
    {
        return;
    }

    UtilityFunctions::print(godot::String("FlecsWorld::_exit_tree(): deinitialising FlecsWorld..."));

    is_initialised = false;
}

FlecsWorld::~FlecsWorld() {}

void FlecsWorld::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("progress", "delta"), &FlecsWorld::progress);
    ClassDB::bind_method(D_METHOD("set_singleton_component", "component_name", "data"), &FlecsWorld::set_singleton_component);
    ClassDB::bind_method(D_METHOD("run_system", "system_name"), &FlecsWorld::run_system);
    ClassDB::bind_method(D_METHOD("set_entity_renderers", "renderers"), &FlecsWorld::set_entity_renderers);
    ClassDB::bind_method(D_METHOD("get_entity_renderers"), &FlecsWorld::get_entity_renderers);

    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::ARRAY, "entity_renderers", godot::PROPERTY_HINT_ARRAY_TYPE, vformat("%s/%s:%s", godot::Variant::OBJECT, godot::PROPERTY_HINT_RESOURCE_TYPE, "EntityRenderer")), "set_entity_renderers", "get_entity_renderers");
}
