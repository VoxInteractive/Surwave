class_name EnemySpawnManager extends Node

@export var portal_global_node_group_name: String = "Portals"

var portal_positions: Array[Vector2] = []

@onready var world: FlecsWorld = $"../World"

func _ready() -> void:
	for portal_node in get_tree().get_nodes_in_group(portal_global_node_group_name):
		print(portal_node.global_position)


func _process(delta: float) -> void:
	if world == null:
		push_warning("EnemySpawnManager: World node not found.")
		return
