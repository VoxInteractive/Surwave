extends Camera2D

@export var tracking_speed :float = 2

var player_nodes :Array[Node]
var player :Node2D
var target_position :Vector2

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	make_current()
	target_position = Vector2.ZERO


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _acquire_target(target_node_group_name: String = "Players") -> void:
	player_nodes = get_tree().get_nodes_in_group(target_node_group_name)
	if player_nodes.size() > 0:
		player = player_nodes[0] as Node2D
		target_position = player.global_position
	else:
		push_warning("Camera: No players found")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	_acquire_target()
	global_position = global_position.lerp(target_position, 1.0 - exp(tracking_speed * -delta))
