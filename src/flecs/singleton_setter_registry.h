#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/variant/dictionary.hpp>

#include <flecs.h>

using godot::Dictionary;

// Setter that receives the flecs world and the Godot Dictionary payload.
using FlecsSingletonSetterRegistry = std::function<void(flecs::world& world, const Dictionary& data)>;

// Register a setter for a singleton component. Components may call this from their
// static registration helpers (FlecsRegistry) so the FlecsWorld can pick them
// up at construction time.
void register_singleton_setter(const std::string& component_name, FlecsSingletonSetterRegistry setter);

// Return a const reference to the global map. Used by FlecsWorld when
// constructing its instance-specific dispatcher.
const std::unordered_map<std::string, FlecsSingletonSetterRegistry>& get_global_singleton_setters();
