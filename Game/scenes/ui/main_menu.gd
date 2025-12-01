extends CanvasLayer

var options_menu_scene = preload("res://scenes/ui/options_menu.tscn")

func _ready() -> void:
	$%PlayButton.pressed.connect(_on_play_pressed)
	$%OptionsButton.pressed.connect(_on_options_pressed)
	$%QuitButton.pressed.connect(_on_quit_pressed)
	
	AudioManager.intro.play()


func _on_play_pressed() -> void:
	get_tree().change_scene_to_file("res://game.tscn")


func _on_options_pressed() -> void:
	var options_menu_instance = options_menu_scene.instantiate()
	add_child(options_menu_instance)
	options_menu_instance.back_button_pressed.connect(_on_options_menu_closed.bind(options_menu_instance))


func _on_quit_pressed() -> void:
	get_tree().quit()


func _on_options_menu_closed(options_menu_instance: Node) -> void:
	options_menu_instance.queue_free()
