extends Stage


func _ready() -> void:
	super._ready()


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	print("Stage 01 received signal: %s with data: %s" % [signal_name, data])
