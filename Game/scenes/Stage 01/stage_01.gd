extends Stage


func _ready() -> void:
	super._ready()
	$World.flecs_signal_emitted.connect(_on_flecs_signal)
	# print("landmark_occupied_areas: ", landmark_occupied_areas)


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	if signal_name == "enemy_died":
		print("Enemy died! Entity ID: ", data.get("entity_id"))
