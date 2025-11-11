#include <string>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs.h>
#include "flecs_registry.h"

using godot::Vector2;
using godot::Vector3;
using godot::UtilityFunctions;


inline void prefab_instantiation(flecs::world& world)
{
    world.system<>("Prefab Instantiation")
        .kind(0) // On-demand
        .run([&](flecs::iter& it)
    {
        const godot::Dictionary* parameters = static_cast<const godot::Dictionary*>(it.param());
        if (!parameters || parameters->is_empty()) {
            UtilityFunctions::push_error("Prefab Instantiation: system called without parameters. At least 'prefab' needs to be specified.");
            return;
        }

        if (!parameters->has("prefab")) {
            UtilityFunctions::push_error("Prefab Instantiation: 'prefab' parameter needs to be given with the name of the prefab to instantiate.");
            return;
        }
        if ((*parameters)["prefab"].get_type() != godot::Variant::STRING)
        {
            UtilityFunctions::push_error("Prefab Instantiation: 'prefab' parameter must be a String.");
            return;
        }
        godot::String prefab_name = (*parameters)["prefab"];
        std::string prefab_name_str = prefab_name.utf8().get_data();
        flecs::entity prefab = world.lookup(prefab_name_str.c_str());
        if (!prefab.is_valid())
        {
            UtilityFunctions::push_error(godot::String("Prefab Instantiation: prefab '") + prefab_name + "' not found in Flecs world");
            return;
        }

        int count = 1;
        if (parameters->has("count"))
        {
            if ((*parameters)["count"].get_type() != godot::Variant::INT)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'count' parameter must be an Integer.");
                return;
            }
            count = (*parameters)["count"];
        }

        for (int i = 0; i < count; ++i)
        {
            flecs::entity instance = world.entity().is_a(prefab);
        }

        UtilityFunctions::print(godot::String("Prefab Instantiation: spawned ") + godot::String::num_int64(count) + " instances of '" + prefab_name + "'");
    });
}
