class_name EndScreen extends CanvasLayer


func _ready() -> void:
	get_tree().paused = true
	$%RestartButton.pressed.connect(on_restart_button_pressed)
	$%QuitButton.pressed.connect(on_quit_button_pressed)


func set_defeat() -> void:
	$%TitleLabel.text = "Defeat"
	$%DescriptionLabel.text = "You lost!"


func on_restart_button_pressed() -> void:
	get_tree().paused = false
	get_tree().change_scene_to_file("res://scenes/Stage 01/stage_01.tscn")


func on_quit_button_pressed() -> void:
	get_tree().quit()
