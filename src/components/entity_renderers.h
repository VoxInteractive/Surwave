#include <map>
#include <string>

#include <godot_cpp/variant/rid.hpp>

#include "flecs_registry.h"

using godot::RID;

struct EntityRenderers
{
    std::map<std::string, std::map<std::string, RID>> renderers_by_type;
};

inline FlecsRegistry register_entity_renderers_component([](flecs::world& world) {
    world.component<EntityRenderers>();
});
