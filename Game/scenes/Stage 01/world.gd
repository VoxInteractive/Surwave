extends FlecsWorld

var spawn_iterations: int = 10000
var spawn_iteration_counter: int = 0

func _ready() -> void:
	pass

func _process(delta: float) -> void:
	progress(delta)

	if spawn_iteration_counter < spawn_iterations:
		for prefab in ["BugSmall", "BugHumanoid", "BugLarge", "Mech"]:
			run_system("Prefab Instantiation", {
				"prefab": prefab,
				"count": 1,
				"transforms": [
					Transform2D(0, Vector2(randf_range(-1000, 1000), randf_range(-1000, 1000))),
				]
			})
		
		spawn_iteration_counter += 1
