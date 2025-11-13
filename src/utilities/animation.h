#pragma once

#include <godot_cpp/variant/color.hpp>

namespace utilities
{
    class Animation
    {
    public:
        static godot::Color make_instance_custom_data(
            int frame_index,
            bool flip_horizontal,
            float rotation_degrees,
            bool flip_vertical,
            int anim_frame_count,
            float anim_phase_offset,
            float anim_speed_seconds);
    };
}
