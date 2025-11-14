shader_type canvas_item;

// Flags that will be passed via INSTANCE_CUSTOM.a
const float FLIP_H = 1.0;
const float FLIP_V = 2.0;

// Spritesheet layout
uniform int columns = 1;
uniform int rows = 1;

varying vec4 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical, .a: alpha
varying vec4 vertex_to_fragment_data; // .xy: cell_size, .zw: frame_origin

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    float base_frame_data   = INSTANCE_CUSTOM.r; // Sign indicates transparency mode, absolute value holds frame index and transparency.
    float anim_properties   = INSTANCE_CUSTOM.g; // Whole number part: Animation range in frames, Decimal part: Animation time offset as a fraction of the cycle duration. Sign: positive = looping, negative = one-shot.
    float anim_speed_signed = INSTANCE_CUSTOM.b; // Animation speed in frames per second; sign controls direction. Set to zero to show the base frame (no animation).    
    float flags             = INSTANCE_CUSTOM.a; // Flags: 1.0: flip_h, 2.0: flip_v

    float column_count = max(float(columns), 1.0);
    float row_count = max(float(rows), 1.0);

    vec4 local_anim_and_flip_data;

    // If base_frame_data is negative, its fractional part determines transparency.
    float is_transparent = step(base_frame_data, -0.0001); // 1.0 if base_frame_data is negative, 0.0 otherwise
    float transparency = fract(abs(base_frame_data));
    local_anim_and_flip_data.a = mix(1.0, 1.0 - transparency, is_transparent);

    float base_frame_index = floor(abs(base_frame_data));
    local_anim_and_flip_data.y = step(0.5, mod(flags, 2.0)); // .g (flip_horizontal)
    local_anim_and_flip_data.z = step(0.5, mod(floor(flags / 2.0), 2.0)); // .b (flip_vertical)

    // ─────────────────────────────────────────
    // 2. Animation timing and current frame
    // ─────────────────────────────────────────
    // Extract animation range and direction
    float anim_window_length = floor(abs(anim_properties));
    float anim_direction     = sign(anim_speed_signed); // +1 → right, -1 → left

    // Extract animation time offset
    float anim_offset_fraction = fract(abs(anim_properties));
    
    // Calculate cycle duration and animation phase
    float anim_speed     = abs(anim_speed_signed);
    float cycle_duration = anim_window_length / max(anim_speed, 1e-5); // Add epsilon to prevent division by zero
    float time_offset    = anim_offset_fraction * cycle_duration;

    // Compute animation phase over time (0–1)
    float normalized_time = (TIME + time_offset) / max(cycle_duration, 1e-5); // Add epsilon to prevent division by zero

    // Selection between one-shot and looping animation modes
    float is_looping = step(0.0, anim_properties); // 1.0 if positive (looping), 0.0 if negative (one-shot)
    float anim_phase = mix(clamp(normalized_time, 0.0, 1.0), fract(normalized_time), is_looping);
    
    float total_frames_in_sheet = column_count * row_count;

    // Current frame offset within the animation window
    float anim_frame_offset = floor(anim_phase * anim_window_length);

    // Compute animated frame index (wrap around for looping)
    float animated_frame_index = mod(base_frame_index + anim_direction * anim_frame_offset + total_frames_in_sheet, total_frames_in_sheet);
    local_anim_and_flip_data.x = animated_frame_index;

    // Calculate and store data for the fragment shader
    // Add a small epsilon to the denominator to prevent division by zero if counts are zero.
    vec2 cell_size = 1.0 / (vec2(column_count, row_count) + 1e-5);

    float column_index = mod(animated_frame_index, column_count);
    float row_index    = floor(animated_frame_index / column_count);
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
    float alpha                = anim_and_flip_data.a;
    vec2 cell_size             = vertex_to_fragment_data.xy;
    vec2 frame_origin          = vertex_to_fragment_data.zw;

    // ─────────────────────────────────────────
    // 4. Local UV transform (flip)
    // ─────────────────────────────────────────
    // Convert UV into local [0,1] space of the current frame
    vec2 uv_local = (UV - frame_origin) / cell_size;

    // Apply flips
    uv_local.x = mix(uv_local.x, 1.0 - uv_local.x, flip_horizontal);
    uv_local.y = mix(uv_local.y, 1.0 - uv_local.y, flip_vertical);

    // ─────────────────────────────────────────
    // 5. Sample the texture
    // ─────────────────────────────────────────
    vec2 uv_final = frame_origin + uv_local * cell_size;
    vec4 tex_color = texture(TEXTURE, uv_final);

    COLOR = tex_color * COLOR;
    COLOR.a *= alpha;
}
