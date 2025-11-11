extends FlecsWorld


func _ready() -> void:
	pass

func _process(delta: float) -> void:
	progress(delta)
	
	run_system("Prefab Instantiation", {
		"prefab": "BugSmall",
		"count": 1,
		"transforms": [
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
		]
	})
