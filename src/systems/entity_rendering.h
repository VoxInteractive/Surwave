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
#include "../components/entity_rendering.h"

using godot::Color;
using godot::PackedFloat32Array;
using godot::RenderingServer;
using godot::Transform2D;
using godot::Transform3D;
using godot::UtilityFunctions;

// Buffer format: https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer

namespace
{
    void write_color_to_buffer(PackedFloat32Array& buffer, int& cursor, const Color& color)
    {
        buffer.set(cursor++, color.r);
        buffer.set(cursor++, color.g);
        buffer.set(cursor++, color.b);
        buffer.set(cursor++, color.a);
    }

    template <typename TransformType>
    void update_multimesh_buffer(
        RenderingServer* rendering_server,
        const MultiMeshRenderer& renderer,
        const std::vector<TransformType>& transforms,
        const std::vector<Color>& instance_colors,
        const std::vector<Color>& instance_custom_data)
    {
        size_t instance_count = transforms.size();
        if (instance_count == 0)
        {
            return;
        }

        int floats_per_instance = 0;
        if constexpr (std::is_same_v<TransformType, Transform2D>)
        {
            floats_per_instance = 8;
        }
        else if constexpr (std::is_same_v<TransformType, Transform3D>)
        {
            floats_per_instance = 12;
        }

        floats_per_instance += (renderer.use_colors ? 4 : 0) + (renderer.use_custom_data ? 4 : 0);

        PackedFloat32Array buffer;
        buffer.resize(static_cast<int>(instance_count * floats_per_instance));

        int buffer_cursor = 0;
        for (size_t i = 0; i < instance_count; ++i)
        {
            const TransformType& t = transforms[i];

            if constexpr (std::is_same_v<TransformType, Transform2D>)
            {
                buffer.set(buffer_cursor++, t.columns[0].x); buffer.set(buffer_cursor++, t.columns[1].x); buffer.set(buffer_cursor++, 0.0f); buffer.set(buffer_cursor++, t.columns[2].x);
                buffer.set(buffer_cursor++, t.columns[0].y); buffer.set(buffer_cursor++, t.columns[1].y); buffer.set(buffer_cursor++, 0.0f); buffer.set(buffer_cursor++, t.columns[2].y);
            }
            else if constexpr (std::is_same_v<TransformType, Transform3D>)
            {
                buffer.set(buffer_cursor++, t.basis.rows[0][0]); buffer.set(buffer_cursor++, t.basis.rows[0][1]); buffer.set(buffer_cursor++, t.basis.rows[0][2]); buffer.set(buffer_cursor++, t.origin.x);
                buffer.set(buffer_cursor++, t.basis.rows[1][0]); buffer.set(buffer_cursor++, t.basis.rows[1][1]); buffer.set(buffer_cursor++, t.basis.rows[1][2]); buffer.set(buffer_cursor++, t.origin.y);
                buffer.set(buffer_cursor++, t.basis.rows[2][0]); buffer.set(buffer_cursor++, t.basis.rows[2][1]); buffer.set(buffer_cursor++, t.basis.rows[2][2]); buffer.set(buffer_cursor++, t.origin.z);
            }

            if (renderer.use_colors && !instance_colors.empty())
            {
                write_color_to_buffer(buffer, buffer_cursor, instance_colors[i % instance_colors.size()]);
            }

            if (renderer.use_custom_data && !instance_custom_data.empty())
            {
                write_color_to_buffer(buffer, buffer_cursor, instance_custom_data[i % instance_custom_data.size()]);
            }
        }

        rendering_server->multimesh_set_buffer(renderer.rid, buffer);
    }
}

inline FlecsRegistry register_entity_rendering_multimesh_system([](flecs::world& world)
{
    // System registered as OnUpdate by default; user can also call it by name.
    world.system<>("Entity Rendering (MultiMesh)")
        .kind(flecs::OnStore)
        .run([&](flecs::iter& it) {

        const EntityRenderers* entity_renderers = nullptr;
        if (it.world().has<EntityRenderers>())
        {
            entity_renderers = &it.world().get<EntityRenderers>();
        }
        if (!entity_renderers)
        {
            UtilityFunctions::push_warning("Entity Rendering (MultiMesh): EntityRenderers singleton component not found");
            return;
        }

        std::unordered_map<RendererType, std::unordered_map<std::string, MultiMeshRenderer>>::const_iterator multimesh_renderers_iterator =
            entity_renderers->renderers_by_type.find(RendererType::MultiMesh);
        if (multimesh_renderers_iterator == entity_renderers->renderers_by_type.end())
        {
            UtilityFunctions::push_warning("Entity Rendering (MultiMesh): No multimesh renderers found in EntityRenderers");
            return;
        }

        RenderingServer* rendering_server = RenderingServer::get_singleton();
        if (!rendering_server)
        {
            UtilityFunctions::push_error("Entity Rendering (MultiMesh): RenderingServer singleton not available");
            return;
        }

        // Collect entities that are instances of each prefab
        for (const std::pair<const std::string, MultiMeshRenderer>& prefab_renderer_pair : multimesh_renderers_iterator->second)
        {
            const std::string& prefab_name = prefab_renderer_pair.first;
            const MultiMeshRenderer& renderer = prefab_renderer_pair.second;

            // Query Flecs for entities that are instances of this prefab.
            flecs::entity prefab = it.world().lookup(prefab_name.c_str());
            if (!prefab.is_valid())
            {
                UtilityFunctions::push_warning("Entity Rendering (MultiMesh): Prefab entity not found: " + godot::String(prefab_name.c_str()));
                continue;
            }

            std::vector<Transform2D> transforms2d;
            std::vector<Transform3D> transforms3d;
            std::vector<Color> instance_colors;
            std::vector<Color> instance_custom_data; // Stored as Color for convenience

            flecs::query<const Transform2D*, const Transform3D*, const Color*> prefab_instance_query =
                it.world().query_builder<const Transform2D*, const Transform3D*, const Color*>()
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

            if (!transforms2d.empty())
            {
                update_multimesh_buffer<Transform2D>(rendering_server, renderer, transforms2d, instance_colors, instance_custom_data);
            }
            else if (!transforms3d.empty())
            {
                update_multimesh_buffer<Transform3D>(rendering_server, renderer, transforms3d, instance_colors, instance_custom_data);
            }
        }
    });
});
