class_name EnemySpawnManager extends Node

@export var portal_global_node_group_name: String = "Portals"

@export_category("Spawn Rate")
## Spawn probabilty curve
@export var probability_curve: Curve
## Controls the type of enemy spawned. The larger the value, the stronger the enemy.
@export var enemy_type_curve: Curve
## Controls how slowly the time advances for curve sampling. 300 means the prob. will max out at 5 minute mark.
@export_range(0.0001, 0.1, 0.001)
var time_multiplier: float = 0.003333333 # maxes out at 5 minute mark

var portal_positions: Array[Vector2] = []
var time: float = 0.0

@onready var world: FlecsWorld = $".."

func _ready() -> void:
	if world == null:
		push_warning("EnemySpawnManager: World node not found.")
		return

	for portal_node in get_tree().get_nodes_in_group(portal_global_node_group_name):
		portal_positions.append(portal_node.global_position)

	print(portal_positions)

func _process(delta: float) -> void:
	time += delta
	
	var portals := get_tree().get_nodes_in_group(portal_global_node_group_name)
	var num_portals := portals.size()
	if num_portals == 0: return
	
	var scaled_time :float = time * time_multiplier
	var prob_curve_sample = clamp(probability_curve.sample_baked(scaled_time), 0.0, 1.0)
	var should_spawn = randf() <= prob_curve_sample
	if should_spawn:
		var picked_enemy_type := _pick_enemy_type(scaled_time)
		var picked_portal = portals.pick_random()

		world.run_system("Prefab Instantiation", {
			"prefab": picked_enemy_type,
			"count": 1,
			"transforms": [picked_portal.get_global_transform()],
		})

func _pick_enemy_type(scaled_time: float) -> String:
		var enemy_type_weight: float = clamp(enemy_type_curve.sample_baked(scaled_time), 0.0, 1.0)
		if enemy_type_weight < 0.33:
			return "BugSmall"
		if enemy_type_weight < 0.66:
			return "BugHumanoid"
		return "BugLarge"
