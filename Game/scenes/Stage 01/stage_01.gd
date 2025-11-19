extends Stage

@export var spawn_outer_margin: float = 96.0
@export var spawn_inner_margin: float = 10.0
@export var spawn_corner_bias: float = 3.0
@export var spawn_radial_exponent: float = 1.2

var spawn_iteration_counter: int = 0
var terrain_size: Vector2

@onready var world: FlecsWorld = $World

func _ready() -> void:
	super._ready()
	terrain_size = terrain.mesh.size
	_spawn_initial_enemy_population()
	
	# print("landmark_occupied_areas: ", landmark_occupied_areas)

func _process(delta: float) -> void:
	super._process(delta)

func _spawn_initial_enemy_population(spawn_iterations: int = 4000) -> void:
	while spawn_iteration_counter < spawn_iterations:
		var prefabs_to_spawn: Dictionary = {
			"BugSmall": 10,
			"BugHumanoid": 2,
			"BugLarge": 1,
		}
		
		for prefab in prefabs_to_spawn:
			var count: int = prefabs_to_spawn[prefab]
			var transforms: Array = []
			
			for i in range(count):
				var transform: Transform2D = _get_random_spawn_transform()
				transforms.append(transform)

			world.run_system("Prefab Instantiation", {
				"prefab": prefab,
				"count": count,
				"transforms": transforms,
			})
		spawn_iteration_counter += 1

func _get_random_spawn_transform() -> Transform2D:
	var spawn_pos: Vector2
	var valid_pos: bool = false
	
	var spawn_area_max: Vector2 = terrain_size / 2.0 - Vector2(spawn_outer_margin, spawn_outer_margin)
	var bias_strength: float = spawn_corner_bias if spawn_corner_bias > 0.0 else 0.0
	var max_weight: float = 1.0 + bias_strength
	# Choose angles uniformly so directions are equally likely, compute angle-specific max radius inside the rectangular spawn area,
	# then sample radius using an inverse CDF so radial density increases outward while remaining normalized per-angle.
	while not valid_pos:
		var angle: float = randf_range(0.0, TAU) # Sample angle uniformly to ensure equal directional counts

		if bias_strength > 0.0: # Apply corner bias
			var weight: float = 1.0 + bias_strength * ((1.0 - cos(4.0 * angle)) * 0.5)
			if randf() > (weight / max_weight):
				continue

		var cos_angle: float = cos(angle)
		var sin_angle: float = sin(angle)

		# Compute the angle-specific maximum radius constrained by the spawn rectangle
		var max_radius: float
		if abs(cos_angle) > 0.0001 and abs(sin_angle) > 0.0001:
			max_radius = min(spawn_area_max.x / abs(cos_angle), spawn_area_max.y / abs(sin_angle))
		else:
			max_radius = spawn_area_max.x if abs(cos_angle) > abs(sin_angle) else spawn_area_max.y

		# Sample a radius normalized on [spawn_inner_margin, max_radius]
		var radius: float = _sample_radius_for_angle(max_radius, spawn_inner_margin, spawn_radial_exponent)
		var x: float = cos_angle * radius
		var y: float = sin_angle * radius

		spawn_pos = Vector2(x, y)
		valid_pos = true
		for area in landmark_occupied_areas:
			if area.has_point(spawn_pos):
				valid_pos = false
				break
	
	return Transform2D(0.0, spawn_pos)


func _radial_cdf_unnormalized(radius: float, inner_margin: float, exponent: float) -> float:
	# Returns an unnormalized primitive for the target radial density. The density model is proportional to (r - in_margin)^p in area terms,
	# and converting to polar coordinates yields a primitive that can be evaluated and inverted numerically.
	var radius_minus_margin: float = radius - inner_margin
	if radius_minus_margin <= 0.0:
		return 0.0
	return (pow(radius_minus_margin, exponent + 2) / (exponent + 2)) + inner_margin * (pow(radius_minus_margin, exponent + 1) / (exponent + 1))


func _sample_radius_for_angle(max_radius: float, inner_margin: float, exponent: float) -> float:
	# Samples a radius in [in_margin, max_radius] according to the radial distribution controlled by exponent p.
	# Uses numeric inversion (binary search) of the unnormalized CDF for robustness.
	if max_radius <= inner_margin:
		return inner_margin

	var cdf_max: float = _radial_cdf_unnormalized(max_radius, inner_margin, exponent)
	if cdf_max <= 0.0:
		# degenerate: fall back to uniform between inner and max
		return randf_range(inner_margin, max_radius)

	var uniform_sample: float = randf() * cdf_max
	var lower_bound: float = inner_margin
	var higher_bound: float = max_radius
	for i in range(28):
		var mid: float = (lower_bound + higher_bound) * 0.5
		if _radial_cdf_unnormalized(mid, inner_margin, exponent) < uniform_sample:
			lower_bound = mid
		else:
			higher_bound = mid
	return (lower_bound + higher_bound) * 0.5
