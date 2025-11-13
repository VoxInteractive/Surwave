#include <algorithm>
#include <cmath>

#include "animation.h" // Include the new header

using godot::Color;

/**
 * @brief Builds per-instance custom data (Color) for the flipbook animation shader.
 *
 * Shader expects:
 *   x (r): frame_index (negative = horizontal flip)
 *   y (g): rotation_degrees (negative = vertical flip)
 *   z (b): animation_range (positive = rightwards, negative = leftwards)
 *          fractional part encodes animation phase offset (0–1)
 *   w (a): animation_speed (seconds per frame)
 *
 * @param frame_index         Base frame index (0-based)
 * @param flip_horizontal     True to flip horizontally
 * @param rotation_degrees    Rotation angle in degrees (0–360)
 * @param flip_vertical       True to flip vertically
 * @param anim_frame_count    Number of frames in animation window (positive = rightwards, negative = leftwards)
 * @param anim_phase_offset   Fractional offset of animation phase [0–1)
 * @param anim_speed_seconds  Duration per frame in seconds (> 0)
 * @return Color              Encoded instance custom data
 */
inline Color utilities::Animation::make_instance_custom_data(
    int frame_index,
    bool flip_horizontal,
    float rotation_degrees,
    bool flip_vertical,
    int anim_frame_count,
    float anim_phase_offset,
    float anim_speed_seconds)
{
    // ────────────────────────────────
    // Sanitize and clamp inputs
    // ────────────────────────────────
    rotation_degrees = std::fmod(rotation_degrees, 360.0f);
    if (rotation_degrees < 0.0f) rotation_degrees += 360.0f;

    anim_phase_offset = std::clamp(anim_phase_offset, 0.0f, 1.0f);
    anim_speed_seconds = std::max(anim_speed_seconds, 0.0001f);

    // Handle animation frame count sign (direction)
    float anim_direction = (anim_frame_count >= 0) ? 1.0f : -1.0f;
    float anim_count_abs = std::abs(static_cast<float>(anim_frame_count));

    // Encode fractional offset into the fractional part of float3 (anim_range)
    float anim_range = anim_direction * (anim_count_abs + anim_phase_offset);

    // ────────────────────────────────
    // Encode flips and angles per shader contract
    // ────────────────────────────────
    float frame_val = static_cast<float>(frame_index) * (flip_horizontal ? -1.0f : 1.0f);
    float rotation_val = rotation_degrees * (flip_vertical ? -1.0f : 1.0f);

    // ────────────────────────────────
    // Compose final Color (vec4)
    // ────────────────────────────────
    Color data;
    data.r = frame_val;
    data.g = rotation_val;
    data.b = anim_range;
    data.a = anim_speed_seconds;
    return data;
}

// Example usage:
//
// #include <godot_cpp/classes/multi_mesh.hpp>
// #include "MakeInstanceCustomData.hpp"

// using namespace godot;

// void setup_multimesh(MultiMesh* multimesh) {
//     // Example configuration:
//     int instance_index = 0;
//     int frame_index = 3;
//     bool flip_h = true;
//     float rotation = 90.0f;       // degrees
//     bool flip_v = false;
//     int anim_range = 2;           // animate over 3 frames (3,4,5)
//     float anim_offset = 0.3f;     // 30% phase offset
//     float anim_speed = 0.5f;      // 500ms per frame

//     Color custom_data = MakeInstanceCustomData(
//         frame_index,
//         flip_h,
//         rotation,
//         flip_v,
//         anim_range,
//         anim_offset,
//         anim_speed
//     );

//     multimesh->set_instance_custom_data(instance_index, custom_data);
// }
