#include <thread>
#include <cctype>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/world2d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "src/world.h"
#include "src/utilities/platform.h"
#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"
#include "src/scripts_loader.h"

#include "src/components/godot_variants.h"
#include "src/components/entity_rendering.h"
#include "src/components/transform.h"
#include "src/components/physics.h"
#include "src/components/player.h"

#include "src/systems/prefab_instantiation.h"
#include "src/systems/transform_update.h"
#include "src/systems/entity_rendering.h"
#include "src/systems/physics.h"

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
    unsigned int num_threads = ::utilities::Platform::get_thread_count();
    world.set_threads(static_cast<int>(num_threads));

    register_components_and_systems_with_world(world);

    // Populate the instance's singleton setters from the global registry
    for (const auto& pair : get_singleton_setters())
    {
        const std::string& component_name = pair.first;
        const FlecsSingletonSetter& global_setter = pair.second;
        singleton_setters[component_name] = [this, global_setter](const godot::Variant& data) { global_setter(this->world, data); };
    }

    // Populate the instance's singleton getters from the global registry
    for (const auto& pair : get_singleton_getters())
    {
        const std::string& component_name = pair.first;
        const FlecsSingletonGetter& global_getter = pair.second;
        singleton_getters[component_name] = [this, global_getter]() { return global_getter(this->world); };
    }
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

        auto& renderer_map = renderers.renderers_by_type[RendererType::MultiMesh];
        // Use godot::RID directly as the key. try_emplace will create a new renderer only if one for this RID doesn't exist.
        auto [it, inserted] = renderer_map.try_emplace(multimesh_rid);
        if (inserted) {
            it->second.rid = multimesh_rid;
            it->second.transform_format = multimesh->get_transform_format();
            it->second.use_colors = multimesh->is_using_colors();
            it->second.use_custom_data = multimesh->is_using_custom_data();
            it->second.instance_count = multimesh->get_instance_count();
            it->second.visible_instance_count = multimesh->get_visible_instance_count();
            renderer_count++;
        }
        MultiMeshRenderer* mm_renderer = &it->second;

        // Build a single query for all prefabs associated with this renderer.
        // This ensures that entities from different prefabs are sorted together.
        auto qb = world.query_builder();

        // If a draw order sorting axis is specified in metadata, set up the ordering.
        // Warning: This is expensive. Measured 11x slowdown when sorting 40k entities vs no sorting.
        char sort_axis = '\0';
        if (child->has_meta("draw_order")) {
            godot::Variant meta_val = child->get_meta("draw_order");
            if (meta_val.get_type() == godot::Variant::STRING || meta_val.get_type() == godot::Variant::STRING_NAME) {
                godot::String sort_axis_str = meta_val;
                if (sort_axis_str.length() == 1) {
                    sort_axis = std::tolower(sort_axis_str.utf8().get_data()[0]);
                }
            }
        }

        if (multimesh->get_transform_format() == godot::MultiMesh::TRANSFORM_2D)
        {
            qb.with<const godot::Transform2D>();
            if (sort_axis != '\0') {
                switch (sort_axis) {
                case 'x':
                    qb.order_by<godot::Transform2D>(
                        [](flecs::entity_t, const godot::Transform2D* t1, flecs::entity_t, const godot::Transform2D* t2) {
                        if (t1->get_origin().x > t2->get_origin().x) return 1; if (t1->get_origin().x < t2->get_origin().x) return -1; return 0;
                    });
                    break;
                case 'y':
                    qb.order_by<godot::Transform2D>(
                        [](flecs::entity_t, const godot::Transform2D* t1, flecs::entity_t, const godot::Transform2D* t2) {
                        if (t1->get_origin().y > t2->get_origin().y) return 1; if (t1->get_origin().y < t2->get_origin().y) return -1; return 0;
                    });
                    break;
                default:
                    // z-axis is not applicable for 2D, so we ignore it.
                    break;
                }
            }
        }
        else
        {
            qb.with<const godot::Transform3D>();
            if (sort_axis != '\0') {
                switch (sort_axis) {
                case 'x':
                    qb.order_by<godot::Transform3D>(
                        [](flecs::entity_t, const godot::Transform3D* t1, flecs::entity_t, const godot::Transform3D* t2) {
                        if (t1->origin.x > t2->origin.x) return 1; if (t1->origin.x < t2->origin.x) return -1; return 0;
                    });
                    break;
                case 'y':
                    qb.order_by<godot::Transform3D>(
                        [](flecs::entity_t, const godot::Transform3D* t1, flecs::entity_t, const godot::Transform3D* t2) {
                        if (t1->origin.y > t2->origin.y) return 1; if (t1->origin.y < t2->origin.y) return -1; return 0;
                    });
                    break;
                case 'z':
                    qb.order_by<godot::Transform3D>(
                        [](flecs::entity_t, const godot::Transform3D* t1, flecs::entity_t, const godot::Transform3D* t2) {
                        if (t1->origin.z > t2->origin.z) return 1; if (t1->origin.z < t2->origin.z) return -1; return 0;
                    });
                    break;
                }
            }
        }

        if (multimesh->is_using_colors())
        {
            qb.with<const RenderingColor>();
        }
        if (multimesh->is_using_custom_data())
        {
            qb.with<const RenderingCustomData>();
        }

        // Chain prefabs with the OR operator. The logic is to add `.or_()` to all but the last term.
        int prefab_count = prefabs.size();
        for (int j = 0; j < prefab_count; ++j)
        {
            godot::String prefab_name = prefabs[j];
            std::string prefab_name_str = prefab_name.utf8().get_data();
            qb.with(flecs::IsA, world.lookup(prefab_name_str.c_str()));
            if (j < prefab_count - 1) {
                qb.or_();
            }
        }

        mm_renderer->queries.push_back(qb.build());
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

void FlecsWorld::update_physics_spaces()
{
    if (!is_inside_tree()) { return; }

    godot::Viewport* viewport = get_viewport();
    if (!viewport) { return; }

    godot::Ref<godot::World2D> world_2d = viewport->find_world_2d();
    if (world_2d.is_valid())
    {
        godot::RID space = world_2d->get_space();
        if (space.is_valid())
        {
            world.set<PhysicsSpace2D>({ space });
        }
    }

    godot::Ref<godot::World3D> world_3d = viewport->find_world_3d();
    if (world_3d.is_valid())
    {
        godot::RID space = world_3d->get_space();
        if (space.is_valid())
        {
            world.set<PhysicsSpace3D>({ space });
        }
    }
}

void FlecsWorld::_notification(const int p_what)
{
    if (p_what == NOTIFICATION_READY)
    {
        setup_entity_renderers();
        update_physics_spaces();
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

void FlecsWorld::set_singleton_component(const godot::String& component_name, const godot::Variant& data)
{
    if (!is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld::set_singleton_component was called before world was initialised"));
        return;
    }

    std::string name = component_name.utf8().get_data();
    auto setter = singleton_setters.find(name);
    if (setter != singleton_setters.end())
    {
        setter->second(data);
    }
    else
    {
        godot::UtilityFunctions::push_warning(godot::String("No setter for singleton component '") + component_name + "' found.");
    }
}

godot::Variant FlecsWorld::get_singleton_component(const godot::String& component_name)
{
    if (!is_initialised)
    {
        UtilityFunctions::push_warning(godot::String("FlecsWorld::get_singleton_component was called before world was initialised"));
        return godot::Variant();
    }

    std::string name = component_name.utf8().get_data();
    auto it = singleton_getters.find(name);
    if (it == singleton_getters.end())
    {
        UtilityFunctions::push_warning(godot::String("No getter for singleton component '") + component_name + "' found.");
        return godot::Variant();
    }

    return it->second();
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
    ClassDB::bind_method(D_METHOD("get_singleton_component", "component_name"), &FlecsWorld::get_singleton_component);
    ClassDB::bind_method(D_METHOD("run_system", "system_name", "data"), &FlecsWorld::run_system, DEFVAL(godot::Dictionary()));
}
