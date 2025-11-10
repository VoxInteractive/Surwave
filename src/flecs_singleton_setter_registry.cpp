#include "flecs_singleton_setter_registry.h"

static std::unordered_map<std::string, FlecsSingletonSetterRegistry> g_singleton_setters;

void register_singleton_setter(const std::string &component_name, FlecsSingletonSetterRegistry setter)
{
    g_singleton_setters.emplace(component_name, std::move(setter));
}

const std::unordered_map<std::string, FlecsSingletonSetterRegistry> &get_global_singleton_setters()
{
    return g_singleton_setters;
}
