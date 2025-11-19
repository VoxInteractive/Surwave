extends Stage

@export var spawn_outer_margin: float = 96.0
@export var spawn_inner_margin: float = 200.0
@export var spawn_corner_bias: bool = true

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

func _spawn_initial_enemy_population(spawn_iterations: int = 1000) -> void:
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
	
	while not valid_pos:
		var rand_x: float = randf_range(-spawn_area_max.x, spawn_area_max.x)
		var rand_y: float = randf_range(-spawn_area_max.y, spawn_area_max.y)
		spawn_pos = Vector2(rand_x, rand_y)
		
		# If inside the inner margin, push it out to the margin's edge.
		if abs(spawn_pos.x) < spawn_inner_margin and abs(spawn_pos.y) < spawn_inner_margin:
			# With corner bias, we have a higher chance of pushing out diagonally.
			# Without it, we push out horizontally or vertically.
			if spawn_corner_bias or randi() % 2 == 0:
				spawn_pos.x = spawn_inner_margin * sign(spawn_pos.x)
				spawn_pos.y = spawn_inner_margin * sign(spawn_pos.y)
			else:
				if randi() % 2 == 0:
					spawn_pos.x = spawn_inner_margin * sign(spawn_pos.x)
				else:
					spawn_pos.y = spawn_inner_margin * sign(spawn_pos.y)
			
		valid_pos = true
		for area in landmark_occupied_areas:
			if area.has_point(spawn_pos):
				valid_pos = false
				break
	
	return Transform2D(0.0, spawn_pos)
