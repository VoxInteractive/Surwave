#include <thread>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "registry.h"
#include "world.h"
#include "scripts_loader.h"
#include "../godot/components/instantiation.h"
#include "Game/cpp/components.h"
#include "../utilities.h"

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

    // Don't initialise if we're running in the editor (not playing)
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // Enable Flecs REST, statistics and extra logging verbosity in debug builds
#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
    world.set<flecs::Rest>({});
    world.import <flecs::stats>();
    flecs::log::set_level(1);
#endif

    // Set the number of threads Flecs should use based on CPU thread count
    unsigned int num_threads = ::project::Utilities::get_thread_count();
    world.set_threads(static_cast<int>(num_threads));

    // Register all Flecs components and systems
    register_with_world(world);

    // Register global singleton setters
    const auto& global = get_global_singleton_setters();
    for (const auto& pair : global)
    {
        singleton_setters.emplace(pair.first, [this, setter = pair.second](const Dictionary& data)
        { setter(this->world, data); });
    }

    // Set self as the instantiation parent for nodes and scenes.
    InstantiationParentSingleton instantiation_parent_component;
    instantiation_parent_component.parent_node = this;
    world.set<InstantiationParentSingleton>(instantiation_parent_component);

    // Load Flecs script files that live in the project's flecs_scripts folder.
    // Use a Godot resource path so the loader can resolve it via ProjectSettings.
    FlecsScriptsLoader loader("res://flecs_scripts");
    loader.load(world);

    is_initialised = true;
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
    auto singleton_setter = singleton_setters.find(name);
    if (singleton_setter != singleton_setters.end())
    {
        singleton_setter->second(data);
    }
}

void FlecsWorld::register_singleton_setter(const std::string& component_name, std::function<void(const Dictionary&)> setter)
{
    // Allow registering setters even before the world is initialised. They
    // will be used when init_world copies global setters into the instance
    // map. Storing here ensures late registrations are still respected.
    singleton_setters.emplace(component_name, std::move(setter));
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

    // Clear any Godot pointers stored in singleton components so systems won't try to use them
    InstantiationParentSingleton instantiation_parent_component;
    instantiation_parent_component.parent_node = nullptr;
    world.set<InstantiationParentSingleton>(instantiation_parent_component);

    // Clear stored setters to drop any references to this node or Godot data
    singleton_setters.clear();
}

FlecsWorld::~FlecsWorld() {}

void FlecsWorld::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("progress", "delta"), &FlecsWorld::progress);
    ClassDB::bind_method(D_METHOD("set_singleton_component", "component_name", "data"), &FlecsWorld::set_singleton_component);
    ClassDB::bind_method(D_METHOD("run_system", "system_name"), &FlecsWorld::run_system);
}
