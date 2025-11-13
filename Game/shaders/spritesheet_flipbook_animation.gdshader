shader_type canvas_item;

// Spritesheet layout
instance uniform int column_count = 1;
instance uniform int row_count = 1;

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
    float frame_index_signed      = instance_data.x; // base frame index (may be negative for flip)
    float rotation_degrees_signed = instance_data.y; // rotation degrees (may be negative for flip)
    float anim_range_signed       = instance_data.z; // animation range (positive → right, negative → left)
    float anim_speed              = instance_data.w; // seconds per frame

    float flip_horizontal = step(frame_index_signed, 0.0);     // 1 if frame_index < 0
    float flip_vertical   = step(rotation_degrees_signed, 0.0); // 1 if rotation < 0

    // Absolute frame and rotation
    float base_frame_index = abs(frame_index_signed);
    float rotation_radians = radians(abs(rotation_degrees_signed));

    // ─────────────────────────────────────────
    // 3. Animation timing and current frame
    // ─────────────────────────────────────────
    float total_frames_in_sheet = float(column_count * row_count);

    // Extract animation range and direction
    float anim_window_length = abs(anim_range_signed);
    float anim_direction     = sign(anim_range_signed); // +1 → right, -1 → left

    // Separate fractional offset component (0–1)
    float anim_offset_fraction = fract(abs(anim_range_signed));

    // Compute animation phase over time (0–1)
    float time_seconds = TIME; // built-in Godot uniform
    float cycle_duration = max(anim_window_length * anim_speed, 0.0001);
    float anim_phase = fract((time_seconds / cycle_duration) + anim_offset_fraction);

    // Current frame offset within the animation window
    float anim_frame_offset = floor(anim_phase * anim_window_length + 0.5);

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
    // 5. Local UV transform (flip + rotation)
    // ─────────────────────────────────────────
    vec2 uv_local = UV;

    // Apply flips
    uv_local.x = mix(uv_local.x, 1.0 - uv_local.x, flip_horizontal);
    uv_local.y = mix(uv_local.y, 1.0 - uv_local.y, flip_vertical);

    // Rotate around frame center (0.5, 0.5)
    vec2 uv_centered = uv_local - 0.5;
    float cos_r = cos(rotation_radians);
    float sin_r = sin(rotation_radians);
    mat2 rotation_matrix = mat2(vec2(cos_r, -sin_r), vec2(sin_r, cos_r));
    vec2 uv_rotated = rotation_matrix * uv_centered + 0.5;

    // ─────────────────────────────────────────
    // 6. Sample the texture
    // ─────────────────────────────────────────
    vec2 uv_final = frame_origin + uv_rotated * cell_size;
    vec4 tex_color = texture(TEXTURE, uv_final);

    COLOR = tex_color * COLOR;
}
