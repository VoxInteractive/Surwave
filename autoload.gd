extends Node

func _unhandled_input(event: InputEvent) -> void:
	# temporary, until menu is implemented
	if Input.is_action_just_pressed("ui_cancel"):
		get_tree().quit()
