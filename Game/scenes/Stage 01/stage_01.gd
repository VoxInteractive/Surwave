extends Stage


func _ready() -> void:
	super._ready()


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	if signal_name == "enemy_died":
		print("Enemy died! Entity ID: ", data.get("entity_id"))

	elif signal_name == "player_took_damage":
		print("Player got hit! Entity ID: ", data.get("entity_id"), " Damage: ", data.get("damage_amount"))
