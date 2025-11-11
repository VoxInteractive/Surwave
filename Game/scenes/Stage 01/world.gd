extends FlecsWorld


func _ready() -> void:
	run_system("Prefab Instantiation", {
		"prefab": "BugSmall",
		"count": 5,
		"transforms": [
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
			Transform2D(0, Vector2(randf_range(-100, 100), randf_range(-100, 100))),
		]
	})

func _process(delta: float) -> void:
	progress(delta)
