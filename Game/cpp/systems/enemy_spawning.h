#include <random>
#include <string>

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs.h>
#include "flecs_registry.h"

using godot::Vector2;
using godot::Vector3;
using godot::UtilityFunctions;


// Spawn 256 instances of the BugSmall prefab in a 1000x1000 area centred on origin
inline void enemy_spawning(flecs::world& world)
{
    // Register a C++ system that runs on demand. We'll implement the spawn logic in this system.
    world.system<>("Enemy Spawning")
        .kind(0)
        .each([&](flecs::entity /*e*/) {
        // Lookup prefab
        flecs::entity prefab = world.lookup("BugSmall");
        if (!prefab.is_valid())
        {
            UtilityFunctions::push_warning("Enemy Spawning: prefab 'BugSmall' not found in Flecs world");
            return;
        }

        // Random generator for positions in [-500, 500]
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-500.0f, 500.0f);

        const int count = 256;
        for (int i = 0; i < count; ++i)
        {
            // Instantiate prefab using Flecs is_a shorthand
            flecs::entity instance = world.entity().is_a(prefab);

            if (world.has<godot::Transform2D>())
            {
                godot::Transform2D t;
                // build a simple transform: no rotation (identity), translation from random pos
                t[0] = Vector2(1, 0);
                t[1] = Vector2(0, 1);
                t[2] = Vector2(dist(gen), dist(gen));
                instance.set<godot::Transform2D>(t);
            }
        }

        UtilityFunctions::print("Enemy Spawning: spawned 256 instances of BugSmall");
    });
}
