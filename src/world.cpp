#include <thread>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
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
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "scripts_loader.h"
#include "utilities.h"
#include "world.h"

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::UtilityFunctions;

FlecsWorld::FlecsWorld()
{
    if (is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld's constructor was called when it was already initialised"));
        return;
    }

    // Don't initialise if we're running in the editor
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // Enable Flecs REST, statistics and extra logging verbosity in debug builds
#if defined(DEBUG_ENABLED)
    UtilityFunctions::print(godot::String("Debug build. Enabling Flecs Explorer and verbose logging ..."));
    world.set<flecs::Rest>({});
    world.import <flecs::stats>();
    flecs::log::set_level(1);
#endif

    // Set the number of threads Flecs should use based on CPU thread count
    unsigned int num_threads = ::project::Utilities::get_thread_count();
    world.set_threads(static_cast<int>(num_threads));

    register_components_for_godot_variants();

    // Load Flecs script files that live in the project's flecs_scripts folder.
    // Use a Godot resource path so the loader can resolve it via ProjectSettings.
    FlecsScriptsLoader loader;
    loader.load(world);

    is_initialised = true;
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

void FlecsWorld::_setup_entity_renderers()
{
    EntityRenderers renderers;

    for (int i = 0; i < get_child_count(); ++i) {
        godot::Node* child = get_child(i);
        godot::RID multimesh_rid;

        if (auto mmi2d = godot::Object::cast_to<godot::MultiMeshInstance2D>(child))
        {
            multimesh_rid = mmi2d->get_multimesh()->get_rid();
        }
        else if (auto mmi3d = godot::Object::cast_to<godot::MultiMeshInstance3D>(child))
        {
            multimesh_rid = mmi3d->get_multimesh()->get_rid();
        }
        else
        {
            continue;
        }

        if (!child->has_meta("prefabs_rendered"))
        {
            continue;
        }

        godot::Array prefabs = child->get_meta("prefabs_rendered");
        if (prefabs.is_empty())
        {
            continue;
        }

        for (int j = 0; j < prefabs.size(); ++j)
        {
            godot::String prefab_name = prefabs[j];
            std::string prefab_name_str = prefab_name.utf8().get_data();
            renderers.prefab_to_multimesh[prefab_name_str] = multimesh_rid;
        }
    }

    if (!renderers.prefab_to_multimesh.empty())
    {
        world.set<EntityRenderers>(renderers);
        UtilityFunctions::print(godot::String("Found and registered ") +
            godot::String::num_int64(renderers.prefab_to_multimesh.size()) +
            " prefab renderers.");
    }
}

void FlecsWorld::_notification(const int p_what)
{
    if (p_what == NOTIFICATION_READY)
    {
        _setup_entity_renderers();
    }
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

    world.progress(static_cast<ecs_ftime_t>(delta));
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
}
