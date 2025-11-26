class_name EnemySpawnManager extends Node

## Spawn probabilty curve
@export var probability_curve: Curve
## Controls the type of enemy spawned. The larger the value, the stronger the enemy.
@export var enemy_type_curve: Curve
## Controls how slowly the time advances for curve sampling. 300 means the prob. will max out at 5 minute mark.
@export_range(0.0001, 0.1, 0.001)
var time_multiplier: float = 0.003333333 # maxes out at 5 minute mark
## Length to exclude from corners when spawning enemies
@export var corner_exclusion_length: int = 64
## Margin from sides when spawning enemies
@export var side_margin: int = 20

var time: float = 0.0
var max_enemy_count: int

@onready var world: FlecsWorld = $".."
@onready var terrain: MeshInstance2D = $"../../Terrain"
@onready var enemies_multimesh: MultiMeshInstance2D = $"../Enemies"

func _ready() -> void:
	if world == null:
		push_warning("EnemySpawnManager: World node not found.")
		return

	max_enemy_count = enemies_multimesh.multimesh.instance_count

func _process(delta: float) -> void:
	time += delta
	
	var current_enemy_count = (world.get_singleton_component("EnemyCount"))
	if current_enemy_count >= max_enemy_count:
		return # Rendering limit reached; can't spawn any more
	
	var scaled_time: float = time * time_multiplier
	var prob_curve_sample: float = probability_curve.sample_baked(scaled_time)
	var should_spawn: bool = randf() < prob_curve_sample
	if should_spawn:
		var picked_enemy_type: String = _pick_enemy_type(scaled_time)
		var spawn_position: Vector2 = _pick_spawn_position()

		world.run_system("Prefab Instantiation", {
			"prefab": picked_enemy_type,
			"count": 1,
			"transforms": [Transform2D(0, spawn_position)],
		})


func _pick_enemy_type(scaled_time: float) -> String:
	var enemy_type_sample: float = enemy_type_curve.sample_baked(scaled_time) - randf()
	if enemy_type_sample < 0.1:
		return "BugSmall"
	if enemy_type_sample < 0.5:
		return "BugHumanoid"
	return "BugLarge"


func _pick_spawn_position() -> Vector2:
	if not terrain or not terrain.mesh:
		return Vector2.ZERO

	var size: Vector2 = terrain.mesh.size
	var half_size: Vector2 = size / 2.0
	var spawnable_width: float = max(size.x - float(corner_exclusion_length * 2), 0.0)
	var spawnable_height: float = max(size.y - float(corner_exclusion_length * 2), 0.0)

	var total_perimeter: float = 2 * spawnable_width + 2 * spawnable_height
	if total_perimeter == 0.0:
		return Vector2.ZERO

	var pick: float = randf() * total_perimeter

	if pick < spawnable_width: # Top edge
		var x_pos: float = pick - (spawnable_width / 2.0)
		return Vector2(x_pos, -half_size.y - side_margin)
	pick -= spawnable_width
	
	if pick < spawnable_width: # Bottom edge
		var x_pos: float = pick - (spawnable_width / 2.0)
		return Vector2(x_pos, half_size.y + side_margin)
	pick -= spawnable_width
	
	if pick < spawnable_height: # Left edge
		var y_pos: float = pick - (spawnable_height / 2.0)
		return Vector2(-half_size.x - side_margin, y_pos)
	pick -= spawnable_height
	
	# Right edge
	var y_pos: float = pick - (spawnable_height / 2.0)
	return Vector2(half_size.x + side_margin, y_pos)
