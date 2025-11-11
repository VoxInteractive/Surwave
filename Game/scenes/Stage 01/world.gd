extends FlecsWorld


func _ready() -> void:
	run_system("Enemy Spawning", {"count": 5})

func _process(delta: float) -> void:
	progress(delta)
	
