extends MultiMeshInstance2D

@onready var terrain: MeshInstance2D = $"../../Terrain"

var elapsed_time :float = 0

func _ready() -> void:
	# Set a custom AABB for the MultiMesh. Setting this manually prevents costly runtime AABB recalculations.
	var terrain_size: Vector2 = terrain.mesh.get_size()
	var half_extents: Vector3 = Vector3(terrain_size.x / 2.0, terrain_size.y / 2.0, 0)
	multimesh.custom_aabb = AABB(-half_extents, half_extents * 2.0)
	
	for i in multimesh.visible_instance_count:
		multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(i * 24, i * 24)))
		# Even when not used, we should set instance_color and custom_data otherwise
		# weird shit happens. Probably the existing data that was in memory is being used.
		multimesh.set_instance_color(i, Color.CYAN)
		multimesh.set_instance_custom_data(i, Color.BLACK)


func _process(delta: float) -> void:
	elapsed_time += delta
	for i in multimesh.visible_instance_count:
		multimesh.set_instance_transform_2d(i, Transform2D(0, Vector2(i * 24 + elapsed_time, i * 24 + elapsed_time)))
