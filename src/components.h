#pragma once

#include <string>

// Forward declare Godot types
namespace godot {
    class MultiMeshInstance3D;
    class AudioStreamPlayer;
}

// A component added to prefabs in FlecsScript to specify a logical renderer name.
struct GodotRenderer {
    std::string name;
};

// The final component set on prefabs, containing a direct pointer to the Godot node.
struct VisualRenderer {
    godot::MultiMeshInstance3D* renderer_node;
};
