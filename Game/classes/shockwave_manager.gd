class_name ShockwaveManager extends Node

@export var singleton_component_name: String = "ShockwaveData"

@onready var world: FlecsWorld = get_node("../../World")


func _process(delta: float) -> void:
	if world == null:
		push_warning("ShockwaveManager: World node not found.")
		return


	world.set_singleton_component(singleton_component_name, {
		"radius": 0
	})
