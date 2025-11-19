extends Stage

@export var spawn_outer_margin: float = 96.0
@export var spawn_inner_margin: float = 200.0
@export var spawn_corner_bias: float = 3.0
@export var spawn_radial_exponent: float = 2.0

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

	# New sampling: uniformly sample the rectangular spawn area, reject points
	# inside the central inner square, and accept samples with probability
	# proportional to a radial weight. This avoids producing extra density
	# around the inner-square edges while producing a radial outward bias.
	var max_global_radius: float = Vector2(spawn_area_max.x, spawn_area_max.y).length()
	while not valid_pos:
		# Sample a point uniformly in the allowed rectangle
		var x: float = randf_range(-spawn_area_max.x, spawn_area_max.x)
		var y: float = randf_range(-spawn_area_max.y, spawn_area_max.y)
		var r: float = sqrt(x * x + y * y)

		# Reject points that fall inside the central inner square (avoid circle-vs-square mismatch)
		if max(abs(x), abs(y)) < spawn_inner_margin:
			continue

		# Optional corner bias (preserves previous behaviour if configured)
		if bias_strength > 0.0:
			var angle: float = atan2(y, x)
			var corner_weight: float = 1.0 + bias_strength * ((1.0 - cos(4.0 * angle)) * 0.5)
			if randf() > (corner_weight / max_weight):
				continue

		# Radial acceptance: favor larger radii. Normalize by a conservative global maximum
		# so the weight is in [0,1]. Higher `spawn_radial_exponent` increases outward bias.
		var radial_weight: float = 0.0
		if max_global_radius > spawn_inner_margin:
			var norm_r: float = clamp((r - spawn_inner_margin) / (max_global_radius - spawn_inner_margin), 0.0, 1.0)
			radial_weight = pow(norm_r, spawn_radial_exponent)
		else:
			radial_weight = 1.0
		# Accept according to radial weight
		if randf() > radial_weight:
			continue

		spawn_pos = Vector2(x, y)
		valid_pos = true

		valid_pos = true
		for area in landmark_occupied_areas:
			if area.has_point(spawn_pos):
				valid_pos = false
				break
	
	return Transform2D(0.0, spawn_pos)
