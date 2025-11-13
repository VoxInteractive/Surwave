shader_type canvas_item;

// Spritesheet layout
uniform int column_count = 1;
uniform int row_count = 1;

varying vec4 instance_data;

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve per-instance data
    // ─────────────────────────────────────────
    instance_data = INSTANCE_CUSTOM;
}

void fragment() {
    // ─────────────────────────────────────────
    // 2.  Unpack and Decode the instance data
    // ─────────────────────────────────────────
    float base_frame_index        = instance_data.r; // Base frame index
    float anim_range_and_offset   = instance_data.g; // Whole number part: Animation range in frames, Decimal part: Animation time offset as a fraction of anim_speed
    float anim_speed_signed       = instance_data.b; // Duration per frame in seconds; sign controls direction. Set to zero just show the base frame (no animation).
    float flip_mode               = instance_data.a; // 0 = none, -1 = horizontal, 1 = vertical flip

    float flip_horizontal = step(flip_mode, -0.5); // 1 if flip_mode is -1
    float flip_vertical   = step(0.5, flip_mode);  // 1 if flip_mode is 1

    // ─────────────────────────────────────────
    // 3. Animation timing and current frame
    float anim_speed = abs(anim_speed_signed);

    // Extract animation range and direction
    // The whole number part of anim_range_and_offset determines the number of frames in the animation window.
    float anim_window_length = floor(anim_range_and_offset);
    float anim_direction     = sign(anim_speed_signed); // +1 → right, -1 → left

    // The fractional part of anim_range_and_offset determines the animation time offset.
    // This offset is applied as a fraction of the anim_speed (seconds per frame).
    float anim_offset_fraction = fract(anim_range_and_offset);

    // Compute animation phase over time (0–1)
    float time_seconds = TIME + anim_offset_fraction * anim_speed;
    float cycle_duration = max(anim_window_length * anim_speed, 0.0001);
    float anim_phase = fract(time_seconds / cycle_duration);

    // Current frame offset within the animation window
    float anim_frame_offset = floor(anim_phase * anim_window_length);

    // Compute animated frame index (wrap around for looping)
    float animated_frame_index = base_frame_index + anim_direction * anim_frame_offset;
    animated_frame_index = mod(animated_frame_index, total_frames_in_sheet);

    // ─────────────────────────────────────────
    // 4. Compute frame grid coordinates
    // ─────────────────────────────────────────
    float column_index = mod(animated_frame_index, float(column_count));
    float row_index    = floor(animated_frame_index / float(column_count));

    vec2 cell_size    = vec2(1.0 / float(column_count), 1.0 / float(row_count));
    vec2 frame_origin = vec2(column_index, row_index) * cell_size;

    // ─────────────────────────────────────────
    // 5. Local UV transform (flip)
    // ─────────────────────────────────────────
    // Convert UV into local [0,1] space of the current frame
    vec2 uv_local = (UV - frame_origin) / cell_size;

    // Apply flips
    uv_local.x = mix(uv_local.x, 1.0 - uv_local.x, flip_horizontal);
    uv_local.y = mix(uv_local.y, 1.0 - uv_local.y, flip_vertical);

    // ─────────────────────────────────────────
    // 6. Sample the texture
    // ─────────────────────────────────────────
    vec2 uv_final = frame_origin + uv_local * cell_size;
    vec4 tex_color = texture(TEXTURE, uv_final);

    COLOR = tex_color * COLOR;
}
