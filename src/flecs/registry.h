#pragma once

#include <functional>
#include <vector>

#include <flecs.h>

using RegistrationCallback = void (*)(flecs::world &);

// Call all registered callbacks to configure the ECS world.
void register_with_world(flecs::world &world);

// Helper for static auto-registration from translation units.
struct FlecsRegistry
{
    explicit FlecsRegistry(RegistrationCallback callback);
};
