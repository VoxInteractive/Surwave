shader_type canvas_item;

// Spritesheet layout
uniform int column_count = 1;
uniform int row_count = 1;

varying vec3 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical
varying vec4 vertex_to_fragment_data; // .xy: cell_size, .zw: frame_origin

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    // Unpack instance data
    float base_frame_index        = INSTANCE_CUSTOM.r; // Base frame index
    float anim_range_and_offset   = INSTANCE_CUSTOM.g; // Whole number part: Animation range in frames, Decimal part: Animation time offset as a fraction of anim_speed
    float anim_speed_signed       = INSTANCE_CUSTOM.b; // Duration per frame in seconds; sign controls direction. Set to zero just show the base frame (no animation).
    float flip_mode               = INSTANCE_CUSTOM.a; // 0 = none, -1 = horizontal, 1 = vertical flip

    vec3 local_anim_and_flip_data;

    // Decode flip data
    local_anim_and_flip_data.g = step(flip_mode, -0.5); // .g (flip_horizontal): 1 if flip_mode is -1
    local_anim_and_flip_data.b = step(0.5, flip_mode);  // .b (flip_vertical): 1 if flip_mode is 1

    // ─────────────────────────────────────────
    // 2. Animation timing and current frame
    // ─────────────────────────────────────────
    float anim_speed = abs(anim_speed_signed);

    // Extract animation range and direction
    float anim_window_length = floor(anim_range_and_offset);
    float anim_direction     = sign(anim_speed_signed); // +1 → right, -1 → left

    // Extract animation time offset
    float anim_offset_fraction = fract(anim_range_and_offset);

    // Compute animation phase over time (0–1)
    float time_seconds = TIME + anim_offset_fraction * anim_speed;
    float cycle_duration = max(anim_window_length * anim_speed, 0.0001);
    float anim_phase = fract(time_seconds / cycle_duration);

    // Calculate total frames
    float total_frames_in_sheet = float(column_count * row_count);

    // Current frame offset within the animation window
    float anim_frame_offset = floor(anim_phase * anim_window_length);

    // Compute animated frame index (wrap around for looping)
    float animated_frame_index = mod(base_frame_index + anim_direction * anim_frame_offset, total_frames_in_sheet);
    local_anim_and_flip_data.r = animated_frame_index;

    // Calculate and store data for the fragment shader
    vec2 cell_size = vec2(1.0 / float(column_count), 1.0 / float(row_count));
    float column_index = mod(animated_frame_index, float(column_count));
    float row_index    = floor(animated_frame_index / float(column_count));
    vec2 frame_origin = vec2(column_index, row_index) * cell_size;

    vertex_to_fragment_data = vec4(cell_size, frame_origin);
    anim_and_flip_data = local_anim_and_flip_data;
}

void fragment() {
    // ─────────────────────────────────────────
    // 3. Unpack vertex shader results
    // ─────────────────────────────────────────
    float animated_frame_index = anim_and_flip_data.r;
    float flip_horizontal      = anim_and_flip_data.g;
    float flip_vertical        = anim_and_flip_data.b;
    vec2 cell_size             = vertex_to_fragment_data.xy;
    vec2 frame_origin          = vertex_to_fragment_data.zw;

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
