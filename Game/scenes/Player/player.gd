extends CharacterBody2D

@export_group("Movement")
@export_range(1.0, 1000.0, 1.0)
var movement_speed :float = 20.0

var input_movement_vector :Vector2

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	input_movement_vector = Input.get_vector("move_left", "move_right", "move_up", "move_down")
	velocity = input_movement_vector * movement_speed
	move_and_slide()
