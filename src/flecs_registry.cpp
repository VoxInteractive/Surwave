#include <mutex>

#include "flecs_registry.h"

namespace
{
    std::vector<RegistrationCallback>& get_callbacks()
    {
        static std::vector<RegistrationCallback> callbacks;
        return callbacks;
    }

    std::mutex& get_mutex()
    {
        static std::mutex mtx;
        return mtx;
    }
}

FlecsRegistry::FlecsRegistry(RegistrationCallback callback)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    get_callbacks().push_back(callback);
}

void register_game_components_and_systems_with_world(flecs::world& world)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    for (RegistrationCallback callback : get_callbacks())
    {
        if (callback != nullptr)
        {
            callback(world);
        }
    }
}
