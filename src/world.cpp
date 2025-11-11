#include <thread>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "world.h"
#include "utilities.h"
#include "flecs_registry.h"
#include "scripts_loader.h"

// Define the global buffer cache that is declared in entity_rendering.h
std::unordered_map<godot::RID, godot::PackedFloat32Array> g_multimesh_buffer_cache;

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

    register_components_and_systems_with_world(world);

    // Load Flecs script files that live in the project's flecs_scripts folder.
    // Use a Godot resource path so the loader can resolve it via ProjectSettings.
    FlecsScriptsLoader loader;
    loader.load(world);

    is_initialised = true;
}


void FlecsWorld::setup_entity_renderers()
{
    EntityRenderers renderers;
    int renderer_count = 0;

    for (int i = 0; i < get_child_count(); ++i) {
        godot::Node* child = get_child(i);
        godot::RID multimesh_rid;
        godot::MultiMesh* multimesh = nullptr;

        if (!child->has_meta("prefabs_rendered"))
        {
            continue;
        }

        if (auto mmi2d = godot::Object::cast_to<godot::MultiMeshInstance2D>(child)) {
            godot::Ref<godot::MultiMesh> mm = mmi2d->get_multimesh();
            if (mm.is_valid()) {
                multimesh = mm.ptr();
            }
        }
        else if (auto mmi3d = godot::Object::cast_to<godot::MultiMeshInstance3D>(child)) {
            godot::Ref<godot::MultiMesh> mm = mmi3d->get_multimesh();
            if (mm.is_valid()) {
                multimesh = mm.ptr();
            }
        }

        if (multimesh)
        {
            multimesh_rid = multimesh->get_rid();

            // Pre-populate the buffer cache to avoid the initial resizing
            g_multimesh_buffer_cache[multimesh_rid] = multimesh->get_buffer();
        }
        else
        {
            if (godot::Object::cast_to<godot::MultiMeshInstance2D>(child) || godot::Object::cast_to<godot::MultiMeshInstance3D>(child))
            {
                UtilityFunctions::push_warning(child->get_class() + godot::String(" node has no MultiMesh resource assigned."));
            }
            else {
                UtilityFunctions::push_warning(child->get_class() + godot::String(" nodes are not supported as entity renderers."));
            }
            continue;
        }

        godot::Array prefabs = child->get_meta("prefabs_rendered");
        if (prefabs.is_empty())
        {
            UtilityFunctions::push_warning(godot::String("Child node '") + child->get_name() + "' has 'prefabs_rendered' metadata, but it is empty.");
            continue;
        }

        for (int j = 0; j < prefabs.size(); ++j)
        {
            godot::String prefab_name = prefabs[j];
            std::string prefab_name_str = prefab_name.utf8().get_data();

            if (!world.lookup(prefab_name_str.c_str()).is_valid())
            {
                UtilityFunctions::push_warning(godot::String("Prefab '") + prefab_name + "' referenced in node '" + child->get_name() + "' does not exist in the Flecs world.");
                continue;
            }

            MultiMeshRenderer renderer_data;
            renderer_data.rid = multimesh_rid;
            renderer_data.transform_format = multimesh->get_transform_format();
            renderer_data.use_colors = multimesh->is_using_colors();
            renderer_data.use_custom_data = multimesh->is_using_custom_data();
            renderer_data.instance_count = multimesh->get_instance_count();
            renderer_data.visible_instance_count = multimesh->get_visible_instance_count();

            renderers.renderers_by_type[RendererType::MultiMesh][prefab_name_str] = renderer_data;
            renderer_count++;
        }
    }

    if (renderer_count > 0)
    {
        world.component<EntityRenderers>();
        world.set<EntityRenderers>(renderers);
        UtilityFunctions::print(godot::String("Found and registered ") +
            godot::String::num_int64(renderer_count) +
            " entity renderers.");
    }
}

void FlecsWorld::_notification(const int p_what)
{
    if (p_what == NOTIFICATION_READY)
    {
        setup_entity_renderers();
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

bool FlecsWorld::run_system(const godot::String& system_name, const godot::Dictionary& parameters)
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

    if (parameters.is_empty())
    {
        sys.run();
    }
    else
    {
        sys.run(0.0f, (void*)&parameters);
    }
    return true;
}

void FlecsWorld::_exit_tree()
{
    if (!is_initialised)
    {
        return;
    }

    is_initialised = false;
}


FlecsWorld::~FlecsWorld() {}


void FlecsWorld::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("progress", "delta"), &FlecsWorld::progress);
    ClassDB::bind_method(D_METHOD("set_singleton_component", "component_name", "data"), &FlecsWorld::set_singleton_component);
    ClassDB::bind_method(D_METHOD("run_system", "system_name", "data"), &FlecsWorld::run_system, DEFVAL(godot::Dictionary()));
}
