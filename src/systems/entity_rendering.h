#include <vector>
#include <string>
#include <algorithm>

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs.h>
#include "world.h"
#include "flecs_registry.h"

using godot::Color;
using godot::PackedFloat32Array;
using godot::RenderingServer;
using godot::Transform2D;
using godot::Transform3D;
using godot::UtilityFunctions;

// Buffer format: https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer

inline FlecsRegistry register_entity_rendering_multimesh_system([](flecs::world& world)
{
    // System registered as OnUpdate by default; user can also call it by name.
    world.system<>("Entity Rendering (Multimesh)")
        .kind(0) // was: flecs::OnStore
        .each([&](flecs::entity /*e*/) {

        const EntityRenderers* entity_renderers = nullptr;
        if (world.has<EntityRenderers>())
        {
            entity_renderers = &world.get<EntityRenderers>();
        }
        if (!entity_renderers)
        {
            UtilityFunctions::push_warning("Entity Rendering (Multimesh): EntityRenderers singleton component not found");
            return;
        }

        std::unordered_map<RendererType, std::unordered_map<std::string, MultiMeshRenderer>>::const_iterator multimesh_renderers_iterator =
            entity_renderers->renderers_by_type.find(RendererType::MultiMesh);
        if (multimesh_renderers_iterator == entity_renderers->renderers_by_type.end())
        {
            UtilityFunctions::push_warning("Entity Rendering (Multimesh): No multimesh renderers found in EntityRenderers");
            return;
        }

        RenderingServer* rendering_server = RenderingServer::get_singleton();
        if (!rendering_server)
        {
            UtilityFunctions::push_error("Entity Rendering (Multimesh): RenderingServer singleton not available");
            return;
        }

        // Collect entities that are instances of each prefab
        for (const std::pair<const std::string, MultiMeshRenderer>& prefab_renderer_pair : multimesh_renderers_iterator->second)
        {
            const std::string& prefab_name = prefab_renderer_pair.first;
            const MultiMeshRenderer& renderer = prefab_renderer_pair.second;

            // Query Flecs for entities that are instances of this prefab.
            flecs::entity prefab = world.lookup(prefab_name.c_str());
            if (!prefab.is_valid())
            {
                UtilityFunctions::push_warning("Entity Rendering (Multimesh): Prefab entity not found: " + godot::String(prefab_name.c_str()));
                continue;
            }

            std::vector<Transform2D> transforms2d;
            std::vector<Transform3D> transforms3d;
            std::vector<Color> instance_colors;
            std::vector<Color> instance_custom_data; // Stored as Color for convenience

            flecs::query<const Transform2D*, const Transform3D*, const Color*> prefab_instance_query =
                world.query_builder<const Transform2D*, const Transform3D*, const Color*>()
                .with(flecs::IsA, prefab)
                .build();
            // https://discordapp.com/channels/633826290415435777/1090412095893422101/1437600527738470513
            prefab_instance_query.each([&](flecs::entity entity, const Transform2D* transform2d, const Transform3D* transform3d, const Color* color) {
                if (transform2d)
                {
                    transforms2d.push_back(*transform2d);
                }
                else if (transform3d)
                {
                    transforms3d.push_back(*transform3d);
                }

                if (color)
                {
                    instance_colors.push_back(*color);
                }

                // custom_data: a user component named 'CustomData' might exist as Color or Vector4.
                if (entity.has<Color>())
                {
                    instance_custom_data.push_back(entity.get<Color>());
                }
            });

            // If we have 2D transforms, use the 2D layout, otherwise 3D layout
            if (!transforms2d.empty())
            {
                size_t instance_count = transforms2d.size();
                // Determine per-instance floats: base 8, +4 if color, +4 if custom
                int floats_per_instance = 8 +
                    (renderer.use_colors ? 4 : 0) +
                    (renderer.use_custom_data ? 4 : 0);

                PackedFloat32Array buffer;
                buffer.resize(static_cast<int>(instance_count * floats_per_instance));

                int buffer_cursor = 0;
                for (size_t instance_idx = 0; instance_idx < instance_count; ++instance_idx)
                {
                    const Transform2D& t = transforms2d[instance_idx];
                    godot::Vector2 x = t[0];
                    godot::Vector2 y = t[1];
                    godot::Vector2 origin = t[2];

                    buffer[buffer_cursor++] = x.x;
                    buffer[buffer_cursor++] = y.x;
                    buffer[buffer_cursor++] = 0.0f; // padding
                    buffer[buffer_cursor++] = origin.x;

                    buffer[buffer_cursor++] = x.y;
                    buffer[buffer_cursor++] = y.y;
                    buffer[buffer_cursor++] = 0.0f; // padding
                    buffer[buffer_cursor++] = origin.y;

                    if (renderer.use_colors && !instance_colors.empty())
                    {
                        const Color& c = instance_colors[instance_idx % instance_colors.size()];
                        buffer[buffer_cursor++] = c.r;
                        buffer[buffer_cursor++] = c.g;
                        buffer[buffer_cursor++] = c.b;
                        buffer[buffer_cursor++] = c.a;
                    }

                    if (renderer.use_custom_data && !instance_custom_data.empty())
                    {
                        const Color& c = instance_custom_data[instance_idx % instance_custom_data.size()];
                        buffer[buffer_cursor++] = c.r;
                        buffer[buffer_cursor++] = c.g;
                        buffer[buffer_cursor++] = c.b;
                        buffer[buffer_cursor++] = c.a;
                    }
                }

                rendering_server->multimesh_set_buffer(renderer.rid, buffer);
            }
            else if (!transforms3d.empty())
            {
                size_t instance_count = transforms3d.size();
                int floats_per_instance = 12 +
                    (renderer.use_colors ? 4 : 0) +
                    (renderer.use_custom_data ? 4 : 0);

                PackedFloat32Array buffer;
                buffer.resize(static_cast<int>(instance_count * floats_per_instance));

                int buffer_cursor = 0;
                for (size_t i = 0; i < instance_count; ++i)
                {
                    const Transform3D& t = transforms3d[i];
                    const godot::Basis& b = t.basis;
                    const godot::Vector3& o = t.origin;

                    buffer[buffer_cursor++] = b[0].x;
                    buffer[buffer_cursor++] = b[1].x;
                    buffer[buffer_cursor++] = b[2].x;
                    buffer[buffer_cursor++] = o.x;

                    buffer[buffer_cursor++] = b[0].y;
                    buffer[buffer_cursor++] = b[1].y;
                    buffer[buffer_cursor++] = b[2].y;
                    buffer[buffer_cursor++] = o.y;

                    buffer[buffer_cursor++] = b[0].z;
                    buffer[buffer_cursor++] = b[1].z;
                    buffer[buffer_cursor++] = b[2].z;
                    buffer[buffer_cursor++] = o.z;

                    if (renderer.use_colors && !instance_colors.empty())
                    {
                        const Color& c = instance_colors[i % instance_colors.size()];
                        buffer[buffer_cursor++] = c.r;
                        buffer[buffer_cursor++] = c.g;
                        buffer[buffer_cursor++] = c.b;
                        buffer[buffer_cursor++] = c.a;
                    }

                    if (renderer.use_custom_data && !instance_custom_data.empty())
                    {
                        const Color& c = instance_custom_data[i % instance_custom_data.size()];
                        buffer[buffer_cursor++] = c.r;
                        buffer[buffer_cursor++] = c.g;
                        buffer[buffer_cursor++] = c.b;
                        buffer[buffer_cursor++] = c.a;
                    }
                }

                rendering_server->multimesh_set_buffer(renderer.rid, buffer);
            }
        }
    });
});
