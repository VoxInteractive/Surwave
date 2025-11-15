shader_type canvas_item;

// Flags that will be passed via INSTANCE_CUSTOM.a
const float FLIP_H = 1.0;
const float FLIP_V = 2.0;

// --- Spritesheet layout ---
uniform int columns = 1;
uniform int rows = 1;

varying vec4 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical, .a: alpha
varying vec4 vertex_to_fragment_data; // .xy: actual_frame_size_uv, .zw: frame_image_origin_uv

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    float base_frame_and_transparency = INSTANCE_CUSTOM.r; // Abs(Whole number part) holds base frame index. When negative, the fractional part determines transparency as a percentage.
    float anim_range_and_time_offset  = INSTANCE_CUSTOM.g; // Abs(Whole number part) Animation range in frames, Decimal part: Animation time offset as a fraction of the cycle duration (sign respected).
    float anim_speed_and_direction    = INSTANCE_CUSTOM.b; // Animation speed in frames per second; sign controls direction. Set to zero to show the base frame without animation.
    float flags                       = INSTANCE_CUSTOM.a; // 1.0: flip_h, 2.0: flip_v, 3.0: flip_h & flip_v

    float column_count = max(float(columns), 1.0);
    float row_count = max(float(rows), 1.0);

    vec4 local_anim_and_flip_data;

    // If base_frame_and_transparency is negative, its fractional part determines transparency.
    float is_transparent = step(base_frame_and_transparency, -1e-5); // 1.0 if base_frame_and_transparency is negative, 0.0 otherwise
    float transparency = fract(abs(base_frame_and_transparency));
    local_anim_and_flip_data.a = mix(1.0, 1.0 - transparency, is_transparent);

    float base_frame_index = floor(abs(base_frame_and_transparency));
    float flags_rounded = floor(flags + 0.5); // Robustly round to nearest integer
    // Branchless flip logic using step()
    local_anim_and_flip_data.y = step(1.0, mod(flags_rounded, 2.0)); // .g (flip_horizontal) - equivalent to (flags_rounded & 1)
    local_anim_and_flip_data.z = step(1.0, floor(flags_rounded / 2.0)); // .b (flip_vertical) - equivalent to (flags_rounded & 2)

    // ─────────────────────────────────────────
    // 2. Animation timing and current frame
    // ─────────────────────────────────────────
    // Extract animation range and direction
    float anim_window_length = floor(abs(anim_range_and_time_offset)) + 1.0;
    float anim_direction     = sign(anim_speed_and_direction); // +1 → right, -1 → left

    // Extract animation time offset
    float anim_offset_fraction = anim_range_and_time_offset - trunc(anim_range_and_time_offset);

    // Calculate cycle duration and animation phase
    float anim_speed = abs(anim_speed_and_direction);
    float cycle_duration = anim_window_length / max(anim_speed, 1e-5); // Add epsilon to prevent division by zero
    float time_offset = anim_offset_fraction * cycle_duration;
    
    // Compute time within the current animation cycle, accounting for direction
    float time_in_cycle = mod((TIME * anim_direction) + time_offset, cycle_duration);

    // Current frame offset within the animation window
    float raw_anim_frame_offset = floor(time_in_cycle * anim_speed);
    float anim_frame_offset = min(raw_anim_frame_offset, anim_window_length - 1.0);

    float total_frames_in_sheet = column_count * row_count;
    float animated_frame_index = mod(base_frame_index + anim_frame_offset, total_frames_in_sheet);
    local_anim_and_flip_data.x = animated_frame_index;

    // Calculate and store data for the fragment shader
    // UV size of a single grid cell (including padding and margin distribution)
    vec2 actual_frame_size_uv = vec2(1.0 / column_count, 1.0 / row_count);

    // Calculate the UV offset for the current frame's top-left corner
    float column_index = mod(animated_frame_index, column_count);
    float row_index    = floor(animated_frame_index / column_count);
    
    // Origin of the current frame's *image* on the spritesheet
    vec2 frame_image_origin_uv = vec2(column_index, row_index) * actual_frame_size_uv;

    vertex_to_fragment_data = vec4(actual_frame_size_uv, frame_image_origin_uv);
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
    vec2 actual_frame_size_uv  = vertex_to_fragment_data.xy;
    vec2 frame_image_origin_uv = vertex_to_fragment_data.zw;

    // ─────────────────────────────────────────
    // 4. Local UV transform (flip)
    // ─────────────────────────────────────────
    vec2 uv_flipped = UV;

    // Apply flips
    uv_flipped.x = mix(uv_flipped.x, 1.0 - uv_flipped.x, flip_horizontal);
    uv_flipped.y = mix(uv_flipped.y, 1.0 - uv_flipped.y, flip_vertical);

    // ─────────────────────────────────────────
    // 5. Sample the texture
    // ─────────────────────────────────────────
    vec4 tex_color = texture(TEXTURE, frame_image_origin_uv + uv_flipped * actual_frame_size_uv);

    COLOR = tex_color; // Assign full texture color, including alpha
    COLOR.a *= alpha;
}
