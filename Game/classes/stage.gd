class_name Stage extends Node

@export_category("Procedural Object Placement")
## The terrain margin in terms of tile count of the Borders tile map layer.
@export var terrain_margin: int = 3
## The number of objects to place. The actual number of total objects placed may be lower due to skip logic
@export var object_count: int = 30

@export_category("Initial Enemy Population Spawning")
## The number of times the spawning logic will run. Each iteration spawns a batch of enemies.
@export var spawn_iterations: int = 50
## The margin from the terrain edge, creating an outer boundary for enemy spawning.
@export var spawn_outer_margin: float = 0.0
## The margin from the center of the map, creating an inner boundary to keep the center clear of initial enemies.
@export var spawn_inner_margin: float = 220.0
## A value greater than 0 biases enemy spawns towards the corners of the spawn area. A value of 0 disables the bias.
@export var spawn_corner_bias: float = 3.0
## Controls the radial distribution of spawns. > 1.0 pushes spawns outward, < 1.0 pulls them inward.
@export var spawn_radial_exponent: float = 1.2

@export_category("Gameplay")
@export var difficulty_curve: Curve

var altar_nodes: Array[Node]
var portal_nodes: Array[Node]
var half_outer_boundary: float
var landmark_occupied_areas: Array[Rect2]
var spawn_iteration_counter: int = 0

@onready var terrain: MeshInstance2D = $Terrain
@onready var terrain_object_multimesh_parents: Array[Node2D] = [$Terrain/Foliage]
@onready var borders: TileMapLayer = $Terrain/Borders
@onready var landmarks: Node = $Landmarks

@onready var world: FlecsWorld = $World


func _ready() -> void:
	_validate_terrain()
	half_outer_boundary = (terrain.mesh.size.x - (borders.tile_set.tile_size.x * terrain_margin)) / 2.0

	_mark_landmark_occupied_areas()
	_place_terrain_objects()
	_initialise_altars()
	_initialise_portals()
	_instantiate_player()
	_instantiate_camera()
	_set_camera_limits()
	_set_world_singletons()
	_spawn_initial_enemy_population()


func _validate_terrain() -> void:
	assert(terrain != null, "Node at path $Terrain is missing or not a MeshInstance2D.")
	assert(terrain.mesh != null, "A terrain mesh must be set for the stage.")
	assert(terrain.mesh.size.x == terrain.mesh.size.y, "Terrain mesh must be a square.")

	assert(borders != null, "Node at path $Terrain/Borders is missing or not a TileMapLayer.")
	assert(borders.tile_set != null, "Borders must be a valid TileMapLayer with a tile_set assigned.")
	assert(borders.tile_set.tile_size.x == borders.tile_set.tile_size.y, "Border tiles must be square")

	assert(landmarks != null, "'Landmarks' node is missing or is invalid.")


func _mark_landmark_occupied_areas() -> void:
	if landmarks == null: return

	landmark_occupied_areas.clear()
	for landmark in landmarks.get_children():
		if not landmark is Node2D:
			continue

		var sprite: Sprite2D
		for child in landmark.get_children():
			if child is StaticBody2D:
				for grandchild in child.get_children():
					if grandchild is Sprite2D:
						sprite = grandchild
						break
			if sprite != null:
				break

		if sprite != null:
			var landmark_size: Vector2 = sprite.get_rect().size
			var landmark_position: Vector2 = landmark.position - landmark_size / 2
			landmark_occupied_areas.append(Rect2(landmark_position, landmark_size))


func _place_terrain_objects() -> void:
	for mm_parent in terrain_object_multimesh_parents:
		if mm_parent == null: continue

		var half_terrain_size = terrain.mesh.size / 2.0
		for multimesh_instance in mm_parent.get_children():
			if multimesh_instance.multimesh == null or not multimesh_instance is MultiMeshInstance2D: continue

			for i in multimesh_instance.multimesh.instance_count:
				var random_position = Vector2(randf_range(-half_terrain_size.x, half_terrain_size.x), randf_range(-half_terrain_size.y, half_terrain_size.y))
				multimesh_instance.multimesh.set_instance_transform_2d(i, Transform2D(randf_range(0, TAU), random_position))


func _initialise_altars() -> void:
	altar_nodes = get_tree().get_nodes_in_group("Altars")
	

func _initialise_portals() -> void:
	portal_nodes = get_tree().get_nodes_in_group("Portals")


func _instantiate_player() -> void:
	var player_scene: PackedScene = preload("res://scenes/Player/player.tscn")
	var player_instance: Node2D = player_scene.instantiate() as Node2D
	add_child(player_instance)
	player_instance.position = Vector2(0, 0)


func _instantiate_camera() -> void:
	var camera_scene: PackedScene = preload("res://scenes/Camera/camera.tscn")
	var camera_instance: Camera2D = camera_scene.instantiate() as Camera2D
	add_child(camera_instance)
	camera_instance.position = Vector2(0, 0)


func _set_camera_limits() -> void:
	var camera = get_node("Camera") as Camera2D
	if camera:
		camera.set_limits(terrain.mesh.size)
	else:
		push_warning("Stage: Camera node not found, couldn't set limits.")


func _set_world_singletons() -> void:
	world.set_singleton_component("StageData", {
		"landmark_occupied_areas": landmark_occupied_areas}
	)


func _spawn_initial_enemy_population() -> void:
	while spawn_iteration_counter < spawn_iterations:
		var prefabs_to_spawn: Dictionary = {
			"BugSmall": 17,
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
	
	var spawn_area_max: Vector2 = terrain.mesh.size / 2.0 - Vector2(spawn_outer_margin, spawn_outer_margin)
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


func _process(delta: float) -> void:
	pass
