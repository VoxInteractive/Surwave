shader_type canvas_item;

uniform sampler2D SCREEN_TEXTURE : hint_screen_texture, filter_linear_mipmap;

void vertex() {
}

void fragment() {
	COLOR = texture(SCREEN_TEXTURE, UV);
}
