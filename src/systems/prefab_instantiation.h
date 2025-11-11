#include <string>

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "flecs_registry.h"

using godot::Dictionary;
using godot::UtilityFunctions;
using godot::Variant;
using godot::Array;

inline FlecsRegistry register_prefab_instantiation_system([](flecs::world& world)
{
    world.system<>("Prefab Instantiation")
        .kind(0) // On-demand
        .run([&](flecs::iter& it)
    {
        const Dictionary* parameters = static_cast<const Dictionary*>(it.param());
        if (!parameters || parameters->is_empty()) {
            UtilityFunctions::push_error("Prefab Instantiation: system called without parameters. At least 'prefab' needs to be specified.");
            return;
        }

        if (!parameters->has("prefab")) {
            UtilityFunctions::push_error("Prefab Instantiation: 'prefab' parameter needs to be given with the name of the prefab to instantiate.");
            return;
        }
        if ((*parameters)["prefab"].get_type() != Variant::STRING)
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
            if ((*parameters)["count"].get_type() != Variant::INT)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'count' parameter must be an Integer.");
                return;
            }
            count = (*parameters)["count"];
        }

        bool has_transforms = false;
        bool transforms_are_2d = false;
        Array transforms_array;
        if (parameters->has("transforms"))
        {
            if ((*parameters)["transforms"].get_type() != Variant::ARRAY)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' parameter must be an Array of Transform2D or Transform3D.");
                return;
            }

            transforms_array = (*parameters)["transforms"];
            if (transforms_array.is_empty()) {
                UtilityFunctions::push_error("Prefab Instantiation: Empty 'transforms' Array given.");
                return;
            }

            if ((int)transforms_array.size() != count)
            {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' array size must be equal to 'count'.");
                return;
            }

            // Determine whether array contains Transform2D or Transform3D values
            Variant first = transforms_array[0];
            if (first.get_type() == Variant::TRANSFORM2D) {
                transforms_are_2d = true;
            }
            else if (first.get_type() == Variant::TRANSFORM3D) {
                transforms_are_2d = false;
            }
            else {
                UtilityFunctions::push_error("Prefab Instantiation: 'transforms' Array must contain Transform2D or Transform3D elements.");
                return;
            }

            // Ensure all elements are of the same transform type
            for (int transform_idx = 1; transform_idx < (int)transforms_array.size(); ++transform_idx) {
                Variant v = transforms_array[transform_idx];
                if (transforms_are_2d && v.get_type() != Variant::TRANSFORM2D) {
                    UtilityFunctions::push_error("Prefab Instantiation: all elements in 'transforms' must be Transform2D when the first element is a Transform2D.");
                    return;
                }
                if (!transforms_are_2d && v.get_type() != Variant::TRANSFORM3D) {
                    UtilityFunctions::push_error("Prefab Instantiation: all elements in 'transforms' must be Transform3D when the first element is a Transform3D.");
                    return;
                }
            }

            has_transforms = true;
        }

        for (int i = 0; i < count; ++i)
        {
            flecs::entity instance = world.entity().is_a(prefab);
            if (has_transforms) {
                int t_idx = 0;
                if (transforms_array.size() > 0) t_idx = i % transforms_array.size();
                if (transforms_are_2d) {
                    godot::Transform2D t = transforms_array[t_idx];
                    instance.set<godot::Transform2D>(t);
                }
                else {
                    godot::Transform3D t = transforms_array[t_idx];
                    instance.set<godot::Transform3D>(t);
                }
            }
        }

        UtilityFunctions::print(godot::String("Prefab Instantiation: spawned ") + godot::String::num_int64(count) + " instances of '" + prefab_name + "'");
    });
});
