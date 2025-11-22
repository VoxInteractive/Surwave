shader_type canvas_item;

uniform float force = 0.5;
uniform float radius = 0.3;
uniform float inner_ring = 0.1;
uniform float outer_ring = 0.1;

const vec2 uv_center = vec2(0.5, 0.5);

void vertex() {
}

void fragment() {
	float mask =	 (1.0 - smoothstep(radius - outer_ring, radius, length(UV - uv_center)))
		* smoothstep(radius - outer_ring - inner_ring, radius - inner_ring, length(UV - uv_center));
	vec2 displacement = normalize(SCREEN_UV - uv_center) * force * mask;
	//COLOR = vec4(mask);
	COLOR = vec4(mask);
}

//void light() {
//	// Called for every pixel for every light affecting the CanvasItem.
//	// Uncomment to replace the default light processing function with this one.
//}
