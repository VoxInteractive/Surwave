extends FlecsWorld

var spawn_iterations: int = 1
var spawn_iteration_counter: int = 0

func _ready() -> void:
	pass

func _process(delta: float) -> void:
	progress(delta)

	if spawn_iteration_counter < spawn_iterations:
		for prefab in ["BugSmall", "BugHumanoid", "BugLarge"]:
			run_system("Prefab Instantiation", {
				"prefab": prefab,
				"count": 1,
				"transforms": [
					Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
					# Transform2D(0, Vector2(randf_range(-200, 200), randf_range(-200, 200))),
				]
			})
		
		spawn_iteration_counter += 1
