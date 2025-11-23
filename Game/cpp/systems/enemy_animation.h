#pragma once

#include <cstdint>

#include <godot_cpp/core/math.hpp>

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
    world.system<const Velocity2D, const AnimationFrameOffset, const DeathTimer, RenderingCustomData>("Enemy Animation")
        .with(flecs::IsA, world.lookup("Enemy"))
        .kind(flecs::PostUpdate)
        .run([](flecs::iter& it) {
        const EnemyAnimationSettings* animation_settings = it.world().try_get<EnemyAnimationSettings>();
        if (animation_settings == nullptr) {
            return;
        }

        const float animation_interval = animation_settings->animation_interval;
        const float animation_speed = animation_interval > 0.0f ? 1.0f / animation_interval : 0.0f;
        const float animation_range = animation_settings->walk_animation_range;
        const float death_animation_frame_count = animation_settings->death_animation_frame_count;
        const float death_animation_range = death_animation_frame_count - 1.0f;
        const float frame_interval = animation_interval > 0.0f ? animation_interval : 0.001f;
        const float up_direction_frame_offset = animation_settings->up_direction_frame_offset;

        while (it.next()) {
            flecs::field<const Velocity2D> velocities = it.field<const Velocity2D>(0);
            flecs::field<const AnimationFrameOffset> frame_offsets = it.field<const AnimationFrameOffset>(1);
            flecs::field<const DeathTimer> death_timer = it.field<const DeathTimer>(2);
            flecs::field<RenderingCustomData> custom_data_field = it.field<RenderingCustomData>(3);

            const size_t count = it.count();
            for (size_t i = 0; i < count; ++i) {
                const godot::Vector2 velocity_value = velocities[i].value;
                const bool moving_north = velocity_value.y < 0.0f;
                const bool moving_left = velocity_value.x < 0.0f;

                const float death_timer_value = death_timer[i].value;

                const float base_offset = frame_offsets[i].value;
                const float walk_directional_offset = (moving_north ? 1.0f : 0.0f) * up_direction_frame_offset + 12.0f;
                const float death_directional_offset = (moving_north ? 1.0f : 0.0f) * up_direction_frame_offset;

                RenderingCustomData& custom_data = custom_data_field[i];

                if (death_timer_value > 0.0f) { // Dying
                    const float frames_remaining = godot::Math::ceil(death_timer_value / frame_interval);
                    const float frames_elapsed = death_animation_frame_count - frames_remaining;
                    const float clamped_frames_elapsed = godot::Math::clamp(frames_elapsed, 0.0f, death_animation_range);
                    const float clamped_frame_offset = godot::Math::floor(clamped_frames_elapsed);
                    custom_data.r = base_offset + death_directional_offset + clamped_frame_offset;
                    custom_data.g = 0.0f;
                    custom_data.b = 0.0f;
                }
                else {
                    custom_data.r = base_offset + walk_directional_offset;
                    custom_data.g = animation_range;
                    custom_data.b = static_cast<float>(animation_speed);
                }

                uint32_t animation_flags = 0U;
                if (moving_left) {
                    animation_flags |= 1U;
                }
                custom_data.a = static_cast<float>(animation_flags);
            }
        }
    });
});
