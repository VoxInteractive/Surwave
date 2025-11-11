extends FlecsWorld


func _ready() -> void:
	run_system("Prefab Instantiation", {"prefab": "BugSmall", "count": 5})

func _process(delta: float) -> void:
	progress(delta)
	
