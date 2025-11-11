extends FlecsWorld


func _ready() -> void:
	run_system("Prefab Instantiation", {
		"prefab": "BugSmall",
		"count": 3,
		"transforms": [
			Transform2D(0, Vector2(-50, -50)),
			Transform2D(0, Vector2(0, 0)),
			Transform2D(0, Vector2(50, 50))
		]
	})
	
	run_system("Entity Rendering (Multimesh)")

func _process(delta: float) -> void:
	progress(delta)
