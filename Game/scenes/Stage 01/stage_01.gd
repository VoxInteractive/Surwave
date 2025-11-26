extends Stage


func _ready() -> void:
	super._ready()


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	if signal_name == "enemy_died":
		print("Enemy died! ", data.get("enemy_type"))

	elif signal_name == "player_took_damage":
		print("Player took: ", data.get("damage_amount"), " damage")
