shader_type canvas_item;

uniform float speed : hint_range(-0.1, 0.1, 0.001) = 1.0;

void fragment() {
	vec2 new_uv = UV;
	new_uv.x = fract(UV.x + (TIME * speed));
	COLOR = texture(TEXTURE, new_uv);
}
