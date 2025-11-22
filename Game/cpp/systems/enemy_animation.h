#pragma once

#include <cstdint>

#include "src/flecs_registry.h"

#include "src/components/entity_rendering.h"
#include "src/components/physics.h"

#include "components/enemy.h"
#include "components/singletons.h"

// The spritesheet contains 6 columns and 12 rows, with the death animations consisting of only the
// leftmost 4 frames and walk animations using all 6 columns in their rows. Row by row layout:
// BugSmall death(down)
// BugSmall death(up)
// BugSmall walk(down)
// BugSmall walk(up)
// BugHumanoid death(down)
// BugHumanoid death(up)
// BugHumanoid walk(down)
// BugHumanoid walk(up)
// BugLarge death(down)
// BugLarge death(up)
// BugLarge walk(down)
// BugLarge walk(up)

inline FlecsRegistry register_enemy_animation_system([](flecs::world& world) {
    world.system<const Velocity2D, const AnimationFrameOffset, RenderingCustomData>("Enemy Animation")
        .with(flecs::IsA, world.lookup("Enemy"))
        .run([](flecs::iter& it) {
        const EnemyAnimationSettings* animation_settings = it.world().try_get<EnemyAnimationSettings>();
        if (animation_settings == nullptr) {
            return;
        }

        const godot::real_t animation_interval = animation_settings->animation_interval;
        const godot::real_t animation_speed = animation_interval > 0.0f ? 1.0f / animation_interval : 0.0f;
        const float animation_range = 5.0f;
        const size_t up_direction_frame_offset = 6U;

        while (it.next()) {
            flecs::field<const Velocity2D> velocities = it.field<const Velocity2D>(0);
            flecs::field<const AnimationFrameOffset> frame_offsets = it.field<const AnimationFrameOffset>(1);
            flecs::field<RenderingCustomData> custom_data_field = it.field<RenderingCustomData>(2);

            const size_t count = it.count();
            for (size_t i = 0; i < count; ++i) {
                const godot::Vector2 velocity_value = velocities[i].value;
                const bool moving_north = velocity_value.y < 0.0f;
                const bool moving_left = velocity_value.x < 0.0f;

                const float base_offset = frame_offsets[i].value;
                const float directional_offset = (moving_north ? 1.0f : 0.0f) * static_cast<float>(up_direction_frame_offset) + 12.0f;
                const float base_frame = base_offset + directional_offset;

                RenderingCustomData& custom_data = custom_data_field[i];
                custom_data.r = base_frame;
                custom_data.g = animation_range;
                custom_data.b = static_cast<float>(animation_speed);

                uint32_t animation_flags = 0U;
                if (moving_left) {
                    animation_flags |= 1U;
                }
                custom_data.a = static_cast<float>(animation_flags);
            }
        }
    });
});
