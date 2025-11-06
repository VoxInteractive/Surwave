#pragma once

namespace project
{
    class Utilities
    {
    public:
        static unsigned int get_thread_count();
    };

    // Small math helper moved here from math_utils. Kept as a free function
    // under the project namespace to avoid polluting the global namespace.
    inline float linear_interpolate(float start_value, float end_value, float factor)
    {
        return start_value + (end_value - start_value) * factor;
    }
}
