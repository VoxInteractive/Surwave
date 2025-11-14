shader_type canvas_item;

// Flags that will be passed via INSTANCE_CUSTOM.a
const float FLIP_H = 1.0;
const float FLIP_V = 2.0;

// --- Spritesheet layout ---
uniform int columns = 1;
uniform int rows = 1;

// --- Spritesheet Padding and Margin ---
// All values are in UV space (0.0 to 1.0), calculated relative to the full spritesheet dimensions.
// To convert from pixels: value_in_uv = value_in_pixels / spritesheet_dimension_in_pixels.
// `sheet_margin_uv`: The empty space from the edge of the image file to the start of the animation grid.
uniform vec2 sheet_margin_uv = vec2(0.0, 0.0);
// `frame_padding_uv`: The empty space from the edge of a grid cell to the actual sprite image inside it.
uniform vec2 frame_padding_uv = vec2(0.0, 0.0);

varying vec4 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical, .a: alpha
varying vec4 vertex_to_fragment_data; // .xy: actual_frame_size_uv, .zw: frame_image_origin_uv

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    float base_frame_data   = 0.0; // INSTANCE_CUSTOM.r; // Sign indicates transparency mode, absolute value holds frame index and transparency.
    float anim_properties   = 3.0; // INSTANCE_CUSTOM.g; // Whole number part: Animation range in frames, Decimal part: Animation time offset as a fraction of the cycle duration.
    float anim_speed_signed = -1.0; // INSTANCE_CUSTOM.b; // Animation speed in frames per second; sign controls direction. Set to zero to show the base frame (no animation).
    float flags_float       = 0.0; // INSTANCE_CUSTOM.a; // Flags: 1.0: flip_h, 2.0: flip_v

    float column_count = max(float(columns), 1.0);
    float row_count = max(float(rows), 1.0);

    vec4 local_anim_and_flip_data;

    // If base_frame_data is negative, its fractional part determines transparency.
    float is_transparent = step(base_frame_data, -1e-5); // 1.0 if base_frame_data is negative, 0.0 otherwise
    float transparency = fract(abs(base_frame_data));
    local_anim_and_flip_data.a = mix(1.0, 1.0 - transparency, is_transparent);

    float base_frame_index = floor(abs(base_frame_data));
    float flags = floor(flags_float + 0.5); // Robustly round to nearest integer
    local_anim_and_flip_data.y = mod(flags, 2.0) >= 1.0 ? 1.0 : 0.0; // .g (flip_horizontal)
    local_anim_and_flip_data.z = floor(flags / 2.0) >= 1.0 ? 1.0 : 0.0; // .b (flip_vertical)

    // ─────────────────────────────────────────
    // 2. Animation timing and current frame
    // ─────────────────────────────────────────
    // Extract animation range and direction
    float anim_window_length = floor(anim_properties);
    float anim_direction     = sign(anim_speed_signed); // +1 → right, -1 → left

    // Extract animation time offset
    float anim_offset_fraction = fract(anim_properties);

    // Calculate cycle duration and animation phase
    float anim_speed = abs(anim_speed_signed);
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
    vec2 total_grid_cell_size_uv = vec2(1.0 / column_count, 1.0 / row_count);

    // Let's recalculate this more robustly:
    // Total usable area for frames after accounting for sheet margins
    vec2 total_sheet_usable_uv_size = vec2(1.0, 1.0) - sheet_margin_uv * 2.0;
    // Size of one frame slot within this usable area (including inter-frame padding)
    vec2 frame_slot_size_within_usable_area = total_sheet_usable_uv_size / vec2(column_count, row_count);
    
    // The actual frame image size is the slot size minus internal padding (padding on both sides)
    vec2 actual_frame_size_uv = frame_slot_size_within_usable_area - frame_padding_uv * 2.0;

    // Calculate the UV offset for the current frame's top-left corner
    float column_index = mod(animated_frame_index, column_count);
    float row_index    = floor(animated_frame_index / column_count);
    
    // Origin of the current frame's *image* on the spritesheet
    vec2 frame_image_origin_uv = sheet_margin_uv + 
                                 vec2(column_index, row_index) * frame_slot_size_within_usable_area +
                                 frame_padding_uv;

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
