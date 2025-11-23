extends Node2D

@export_category("Movement")
@export var speed :float = 10.0;

var direction :Vector2 = Vector2.UP

func _ready() -> void:
	pass # Replace with function body.


func _process(delta: float) -> void:
	global_position += direction * speed;
