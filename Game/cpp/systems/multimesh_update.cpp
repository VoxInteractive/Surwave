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
#include "../../../src/world.h"
#include "../../../src/flecs_registry.h"

using godot::RenderingServer;
using godot::PackedFloat32Array;
using godot::Transform2D;
using godot::Transform3D;
using godot::Color;
using godot::UtilityFunctions;

void register_multimesh_update_system(flecs::world& world);
static FlecsRegistry _multimesh_update_registry(register_multimesh_update_system);

// This system will be registered under the name "MultimeshUpdate" and should be
// invoked each frame (or on-demand) to upload transforms for prefabs whose
// multimesh RID was registered in the EntityRenderers singleton.

void register_multimesh_update_system(flecs::world& world)
{
    // System registered as OnUpdate by default; user can also call it by name.
    world.system<>("MultimeshUpdate")
        .kind(flecs::OnStore)
        .each([&](flecs::entity /*e*/) {
        // Read EntityRenderers singleton
        const EntityRenderers* renderers = nullptr;
        if (world.has<EntityRenderers>())
        {
            renderers = &world.get<EntityRenderers>();
        }
        if (!renderers)
        {
            // Nothing registered
            return;
        }

        // Find the multimesh renderers mapping
        auto it = renderers->renderers_by_type.find("multimesh");
        if (it == renderers->renderers_by_type.end())
        {
            return;
        }

        RenderingServer* rs = RenderingServer::get_singleton();
        if (!rs)
        {
            UtilityFunctions::push_warning("MultimeshUpdate: RenderingServer singleton not available");
            return;
        }

        // For each prefab name -> RID, collect entities that are instances of that prefab
        for (const auto& kv : it->second)
        {
            const std::string& prefab_name = kv.first;
            const godot::RID& multimesh_rid = kv.second;

            // Query Flecs for entities that are instances of this prefab.
            flecs::entity prefab = world.lookup(prefab_name.c_str());
            if (!prefab.is_valid())
            {
                continue;
            }

            // Build a query for entities that are_a(prefab) - using descriptor string
            std::string expr = std::string("IsA(") + prefab_name + ")";

            // We'll manually collect transforms by iterating all entities and checking IsA
            std::vector<Transform2D> transforms2d;
            std::vector<Transform3D> transforms3d;
            std::vector<Color> colors;
            std::vector<Color> customs; // custom_data stored as Color for convenience

            // Iterate all entities and check if they are instances of prefab
            world.each([&](const flecs::entity& e) {
                if (!e.has(flecs::IsA, prefab)) return; // not an instance

                // Prefer Transform2D if present
                if (e.has<Transform2D>())
                {
                    transforms2d.push_back(e.get<Transform2D>());
                }
                else if (e.has<Transform3D>())
                {
                    transforms3d.push_back(e.get<Transform3D>());
                }

                if (e.has<Color>())
                {
                    colors.push_back(e.get<Color>());
                }

                // custom_data: a user component named 'CustomData' might exist as Color or Vector4.
                if (e.has<Color>())
                {
                    customs.push_back(e.get<Color>());
                }
            });

            // If we have 2D transforms, use the 2D layout, otherwise 3D layout
            if (!transforms2d.empty())
            {
                size_t instances = transforms2d.size();
                // Determine per-instance floats: base 8, +4 if color, +4 if custom
                bool has_color = !colors.empty();
                bool has_custom = !customs.empty();
                int floats_per = 8 + (has_color ? 4 : 0) + (has_custom ? 4 : 0);

                PackedFloat32Array buffer;
                buffer.resize(static_cast<int>(instances * floats_per));

                int idx = 0;
                for (size_t i = 0; i < instances; ++i)
                {
                    const Transform2D& t = transforms2d[i];
                    // The user requested ordering (row-major with padding):
                    // [ x.x, y.x, pad, origin.x, x.y, y.y, pad, origin.y ]
                    // In Godot Transform2D columns are Vector2 columns[0], columns[1], columns[2]
                    godot::Vector2 x = t[0];
                    godot::Vector2 y = t[1];
                    godot::Vector2 origin = t[2];

                    buffer[idx++] = x.x;
                    buffer[idx++] = y.x;
                    buffer[idx++] = 0.0f; // padding
                    buffer[idx++] = origin.x;

                    buffer[idx++] = x.y;
                    buffer[idx++] = y.y;
                    buffer[idx++] = 0.0f; // padding
                    buffer[idx++] = origin.y;

                    if (has_color)
                    {
                        const Color& c = colors[i % colors.size()];
                        buffer[idx++] = c.r;
                        buffer[idx++] = c.g;
                        buffer[idx++] = c.b;
                        buffer[idx++] = c.a;
                    }

                    if (has_custom)
                    {
                        const Color& c = customs[i % customs.size()];
                        buffer[idx++] = c.r;
                        buffer[idx++] = c.g;
                        buffer[idx++] = c.b;
                        buffer[idx++] = c.a;
                    }
                }

                rs->multimesh_set_buffer(multimesh_rid, buffer);
            }
            else if (!transforms3d.empty())
            {
                size_t instances = transforms3d.size();
                bool has_color = !colors.empty();
                bool has_custom = !customs.empty();
                int floats_per = 12 + (has_color ? 4 : 0) + (has_custom ? 4 : 0);

                PackedFloat32Array buffer;
                buffer.resize(static_cast<int>(instances * floats_per));

                int idx = 0;
                for (size_t i = 0; i < instances; ++i)
                {
                    const Transform3D& t = transforms3d[i];
                    const godot::Basis& b = t.basis;
                    const godot::Vector3& o = t.origin;

                    // User requested ordering for 3D:
                    // [ basis.x.x, basis.y.x, basis.z.x, origin.x,
                    //   basis.x.y, basis.y.y, basis.z.y, origin.y,
                    //   basis.x.z, basis.y.z, basis.z.z, origin.z ]

                    buffer[idx++] = b[0].x;
                    buffer[idx++] = b[1].x;
                    buffer[idx++] = b[2].x;
                    buffer[idx++] = o.x;

                    buffer[idx++] = b[0].y;
                    buffer[idx++] = b[1].y;
                    buffer[idx++] = b[2].y;
                    buffer[idx++] = o.y;

                    buffer[idx++] = b[0].z;
                    buffer[idx++] = b[1].z;
                    buffer[idx++] = b[2].z;
                    buffer[idx++] = o.z;

                    if (has_color)
                    {
                        const Color& c = colors[i % colors.size()];
                        buffer[idx++] = c.r;
                        buffer[idx++] = c.g;
                        buffer[idx++] = c.b;
                        buffer[idx++] = c.a;
                    }

                    if (has_custom)
                    {
                        const Color& c = customs[i % customs.size()];
                        buffer[idx++] = c.r;
                        buffer[idx++] = c.g;
                        buffer[idx++] = c.b;
                        buffer[idx++] = c.a;
                    }
                }

                rs->multimesh_set_buffer(multimesh_rid, buffer);
            }
        }
    });
}
