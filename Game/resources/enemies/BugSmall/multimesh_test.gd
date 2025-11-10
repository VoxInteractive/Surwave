extends MultiMeshInstance2D


func _ready() -> void:
	for i in multimesh.visible_instance_count:
		multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(i * 24, i * 24)))
		# Even when not used, we should set instance_color and custom_data otherwise
		# weird ship happens. Probably the existing data that was in memory is being used.
		multimesh.set_instance_color(i, Color.CYAN)
		multimesh.set_instance_custom_data(i, Color.BLACK)


func _process(delta: float) -> void:
	pass
