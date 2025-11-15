shader_type canvas_item;

// Flags that will be passed via INSTANCE_CUSTOM.a
const float FLIP_H_FLAG = 1.0;
const float FLIP_V_FLAG = 2.0;

// Color Masking constants
const int BLEND_MODE_ADD = 0;
const int BLEND_MODE_SUBTRACT = 1;
const int BLEND_MODE_MULTIPLY = 2;

// --- Spritesheet layout ---
uniform int columns = 1;
uniform int rows = 1;

varying vec4 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical, .a: alpha
varying vec4 vertex_to_fragment_data; // .xy: actual_frame_size_uv, .zw: frame_image_origin_uv
varying vec3 color_mask;
varying float blend_mode;
varying float has_color_mask;


void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    // The hardcoded values are for temporary testing purposes
    float base_frame_and_transparency = 66.0; // INSTANCE_CUSTOM.r; // Abs(Whole number part) holds base frame index. When negative, the fractional part determines transparency as a percentage.
    float anim_range_and_time_offset  = 5.0;  // INSTANCE_CUSTOM.g; // Abs(Whole number part) Animation range in frames, Decimal part: Animation time offset as a fraction of the cycle duration (sign respected).
    float anim_speed_and_direction    = 2.0;  // INSTANCE_CUSTOM.b; // Animation speed in frames per second; sign controls direction. Set to zero to show the base frame without animation.    
    float flags                       = 8323072.0;  // INSTANCE_CUSTOM.a; // Bitwise: 1 for flip_h, 2 for flip_v. Positive values for color masking.

    float column_count = max(float(columns), 1.0);
    float row_count = max(float(rows), 1.0);

    vec4 local_anim_and_flag_data;

    // If base_frame_and_transparency is negative, its fractional part determines transparency.
    float is_transparent = step(base_frame_and_transparency, -1e-5); // 1.0 if base_frame_and_transparency is negative, 0.0 otherwise
    float transparency = fract(abs(base_frame_and_transparency));
    local_anim_and_flag_data.a = mix(1.0, 1.0 - transparency, is_transparent);

    int flags_int = int(flags);
    has_color_mask = step(0.0, flags); // 1.0 if flags is positive or zero

    if (has_color_mask > 0.5) {
        // Positive flags are for color masking
        int r = (flags_int >> 10) & 0x7F;
        int g = (flags_int >> 17) & 0x7F;
        int b = (flags_int >> 24) & 0x7F;
        color_mask = vec3(float(r) / 127.0, float(g) / 127.0, float(b) / 127.0);
        blend_mode = float((flags_int >> 7) & 0x07);
        local_anim_and_flag_data.y = float(flags_int & 1); // flip_h
        local_anim_and_flag_data.z = float((flags_int >> 1) & 1); // flip_v
    } else {
        // Negative flags are for flipping only
        float flags_rounded = floor(abs(flags) + 0.5);
        local_anim_and_flag_data.y = step(0.5, mod(flags_rounded, 2.0)); // flip_h
        local_anim_and_flag_data.z = step(1.5, flags_rounded); // flip_v
        has_color_mask = 0.0;
    }

    float base_frame_index = floor(abs(base_frame_and_transparency));

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
    local_anim_and_flag_data.x = animated_frame_index;

    // Calculate and store data for the fragment shader
    // UV size of a single grid cell (including padding and margin distribution)
    vec2 actual_frame_size_uv = vec2(1.0 / column_count, 1.0 / row_count);

    // Calculate the UV offset for the current frame's top-left corner
    float column_index = mod(animated_frame_index, column_count);
    float row_index    = floor(animated_frame_index / column_count);
    
    // Origin of the current frame's *image* on the spritesheet
    vec2 frame_image_origin_uv = vec2(column_index, row_index) * actual_frame_size_uv;

    vertex_to_fragment_data = vec4(actual_frame_size_uv, frame_image_origin_uv);
    anim_and_flip_data = local_anim_and_flag_data;
}

void fragment() {
    // ─────────────────────────────────────────
    // 3. Unpack vertex shader results
    // ─────────────────────────────────────────
    float animated_frame_index = anim_and_flip_data.r;
    float flip_horizontal = anim_and_flip_data.g;
    float flip_vertical = anim_and_flip_data.b;
    float alpha = anim_and_flip_data.a;
    vec2 actual_frame_size_uv = vertex_to_fragment_data.xy;
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

    // Branchless color masking
    // Determine which blend mode is active.
    float is_add = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_ADD)));
    float is_subtract = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_SUBTRACT)));
    float is_multiply = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_MULTIPLY)));

    // Calculate the result for each blend mode.
    vec3 add_result = tex_color.rgb + color_mask;
    vec3 subtract_result = tex_color.rgb - color_mask;
    vec3 multiply_result = tex_color.rgb * color_mask;

    // Combine the results based on the active blend mode and whether masking is enabled.
    vec3 masked_color = is_add * add_result + is_subtract * subtract_result + is_multiply * multiply_result;
    tex_color.rgb = mix(tex_color.rgb, masked_color, has_color_mask);

    COLOR = tex_color;
    COLOR.a *= alpha;
}
