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
#include "../utilities/godot_hashes.h"

using godot::Color;
using godot::PackedFloat32Array;
using godot::RenderingServer;
using godot::Transform2D;
using godot::Transform3D;
using godot::UtilityFunctions;

// Buffer format: https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer
// The per - instance data size and expected data order is :
// 2D :
//     - Position : 8 floats(8 floats for Transform2D)
//     - Position + Vertex color : 12 floats(8 floats for Transform2D, 4 floats for Color)
//     - Position + Custom data : 12 floats(8 floats for Transform2D, 4 floats of custom data)
//     - Position + Vertex color + Custom data : 16 floats(8 floats for Transform2D, 4 floats for Color, 4 floats of custom data)
// 3D :
//     - Position : 12 floats(12 floats for Transform3D)
//     - Position + Vertex color : 16 floats(12 floats for Transform3D, 4 floats for Color)
//     - Position + Custom data : 16 floats(12 floats for Transform3D, 4 floats of custom data)
//     - Position + Vertex color + Custom data : 20 floats(12 floats for Transform3D, 4 floats for Color, 4 floats of custom data)
// 
// Instance transforms are in row - major order.Specifically:
// For Transform2D the float - order is : (x.x, y.x, padding_float, origin.x, x.y, y.y, padding_float, origin.y).
// For Transform3D the float - order is : (basis.x.x, basis.y.x, basis.z.x, origin.x, basis.x.y, basis.y.y, basis.z.y, origin.y, basis.x.z, basis.y.z, basis.z.z, origin.z).


extern std::unordered_map<godot::RID, PackedFloat32Array> g_multimesh_buffer_cache;

// Collect instances for a single prefab and update the corresponding multimesh buffer.
// This helper builds a query specialized for the transform type (2D or 3D) and
// conditionally includes vertex colors and custom data as query terms when the renderer expects them.
template <typename TransformType>
void update_renderer_for_prefab(
    RenderingServer* rendering_server,
    const MultiMeshRenderer& renderer)
{
    size_t floats_per_instance = 0;
    if (renderer.transform_format == godot::MultiMesh::TRANSFORM_2D) {
        floats_per_instance = 8;
    }
    else { // TRANSFORM_3D
        floats_per_instance = 12;
    }

    floats_per_instance += (renderer.use_colors ? 4 : 0) + (renderer.use_custom_data ? 4 : 0);

    PackedFloat32Array& buffer = g_multimesh_buffer_cache[renderer.rid];
    size_t required_size = renderer.instance_count * floats_per_instance;
    if (buffer.size() != required_size) {
        UtilityFunctions::push_warning(
            godot::String("Entity Rendering (MultiMesh): Resizing MultiMesh buffer from ") +
            godot::String::num_int64(buffer.size()) + godot::String(" to ") +
            godot::String::num_int64(required_size));
        buffer.resize(required_size);
    }

    float* buffer_ptr = buffer.ptrw();
    size_t instance_count = 0;

    // Each renderer can have multiple queries (one per prefab). Iterate all queries and append matching instances
    // to the buffer until the renderer's instance_count is reached.
    for (const auto& q : renderer.queries) {
        q.run([&](flecs::iter& it) {
            while (it.next()) {
                auto transform_field = it.field<const TransformType>(0);

                for (auto i : it) {
                    if (instance_count >= renderer.instance_count) {
                        break;
                    }

                    size_t buffer_cursor = instance_count * floats_per_instance;
                    const TransformType& transform = transform_field[i];

                    if constexpr (std::is_same_v<TransformType, Transform2D>) {
                        buffer_ptr[buffer_cursor++] = transform.columns[0].x;
                        buffer_ptr[buffer_cursor++] = transform.columns[1].x;
                        buffer_ptr[buffer_cursor++] = 0.0f;
                        buffer_ptr[buffer_cursor++] = transform.columns[2].x;
                        buffer_ptr[buffer_cursor++] = transform.columns[0].y;
                        buffer_ptr[buffer_cursor++] = transform.columns[1].y;
                        buffer_ptr[buffer_cursor++] = 0.0f;
                        buffer_ptr[buffer_cursor++] = transform.columns[2].y;
                    }
                    else if constexpr (std::is_same_v<TransformType, Transform3D>) {
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[0][0];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[0][1];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[0][2];
                        buffer_ptr[buffer_cursor++] = transform.origin.x;
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[1][0];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[1][1];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[1][2];
                        buffer_ptr[buffer_cursor++] = transform.origin.y;
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[2][0];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[2][1];
                        buffer_ptr[buffer_cursor++] = transform.basis.rows[2][2];
                        buffer_ptr[buffer_cursor++] = transform.origin.z;
                    }

                    int next_field_idx = 1;
                    if (renderer.use_colors) {
                        const RenderingColor& color = it.field<const RenderingColor>(next_field_idx++)[i];
                        buffer_ptr[buffer_cursor++] = color.r;
                        buffer_ptr[buffer_cursor++] = color.g;
                        buffer_ptr[buffer_cursor++] = color.b;
                        buffer_ptr[buffer_cursor++] = color.a;
                    }
                    if (renderer.use_custom_data) {
                        const RenderingCustomData& custom_data = it.field<const RenderingCustomData>(next_field_idx++)[i];
                        buffer_ptr[buffer_cursor++] = custom_data.r;
                        buffer_ptr[buffer_cursor++] = custom_data.g;
                        buffer_ptr[buffer_cursor++] = custom_data.b;
                        buffer_ptr[buffer_cursor++] = custom_data.a;
                    }
                    instance_count++;
                }
            }
        });
        if (instance_count >= renderer.instance_count) break;
    }

    rendering_server->multimesh_set_buffer(renderer.rid, buffer);
    rendering_server->multimesh_set_visible_instances(renderer.rid, instance_count);
}


inline FlecsRegistry register_entity_rendering_multimesh_system([](flecs::world& world)
{
    // This system iterates over all MultiMesh renderers and updates their buffers.
    // It's designed to be efficient by using pre-built queries stored in the MultiMeshRenderer component.
    world.system("Entity Rendering (MultiMesh)")
        .kind(flecs::OnStore)
        .run([](flecs::iter& it) {

        if (!it.world().has<EntityRenderers>())
        {
            return; // No renderers component
        }
        const EntityRenderers& renderers = it.world().get<EntityRenderers>();

        auto multimesh_renderers_it = renderers.renderers_by_type.find(RendererType::MultiMesh);
        if (multimesh_renderers_it == renderers.renderers_by_type.end())
        {
            return; // No multimesh renderers
        }

        RenderingServer* rendering_server = RenderingServer::get_singleton();
        if (!rendering_server)
        {
            UtilityFunctions::push_error("Entity Rendering (MultiMesh): RenderingServer singleton not available");
            return;
        }

        for (auto& prefab_renderer_pair : multimesh_renderers_it->second)
        {
            const MultiMeshRenderer& renderer = prefab_renderer_pair.second;

            if (renderer.transform_format == godot::MultiMesh::TRANSFORM_2D)
            {
                update_renderer_for_prefab<Transform2D>(rendering_server, renderer);
            }
            else
            {
                update_renderer_for_prefab<Transform3D>(rendering_server, renderer);
            }
        }
    });
});
