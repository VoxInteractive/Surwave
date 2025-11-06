class_name Stage extends Node

@export_category("Procedural Object Placement")
## The terrain margin in terms of tile count of the Borders tile map layer.
@export var terrain_margin: int = 3
## The number of objects to place. The actual number of total objects placed may be lower due to skip logic
@export var object_count: int = 10000

@export_category("Gameplay")
@export var difficulty_curve: Curve

var altar_nodes: Array[Node]
var portal_nodes: Array[Node]
var calculated_outer_margin: int
var landmark_occupied_areas: Array[Rect2]

@onready var terrain: MeshInstance2D = $Terrain
@onready var borders: TileMapLayer = $Terrain/Borders
@onready var objects: TileMapLayer = $Terrain/Objects
@onready var landmarks: Node = $Landmarks


func _ready() -> void:
	_validate_terrain()

	calculated_outer_margin = borders.tile_set.tile_size.x * terrain_margin

	_mark_landmark_occupied_areas()

	_place_objects()

	_initialise_altars()

	_initialise_portals()


func _validate_terrain() -> void:
	assert(terrain != null, "Node at path $Terrain is missing or not a MeshInstance2D.")
	assert(terrain.mesh != null, "A terrain mesh must be set for the stage.")
	assert(terrain.mesh.size.x == terrain.mesh.size.y, "Terrain mesh must be a square.")

	assert(borders != null, "Node at path $Terrain/Borders is missing or not a TileMapLayer.")
	assert(borders.tile_set != null, "Borders must be a valid TileMapLayer with a tile_set assigned.")
	assert(borders.tile_set.tile_size.x == borders.tile_set.tile_size.y, "Border tiles must be square")

	assert(objects != null, "Node at path $Terrain/Objects is missing or not a TileMapLayer.")
	assert(objects.tile_set != null, "Objects must be a valid TileMapLayer with a tile_set assigned.")

	assert(landmarks != null, "'Landmarks' node is missing or is invalid.")

func _mark_landmark_occupied_areas() -> void:
	if landmarks == null: return

	landmark_occupied_areas.clear()
	for landmark in landmarks.get_children():
		if not landmark is Node2D:
			continue

		var sprite: Sprite2D
		for child in landmark.get_children():
			if child is StaticBody2D:
				for grandchild in child.get_children():
					if grandchild is Sprite2D:
						sprite = grandchild
						break
			if sprite != null:
				break

		if sprite != null:
			var landmark_size: Vector2 = sprite.get_rect().size
			var landmark_position: Vector2 = landmark.position - landmark_size / 2
			landmark_occupied_areas.append(Rect2(landmark_position, landmark_size))

func _place_objects(outer_bound: float = terrain.mesh.size.x - calculated_outer_margin, inner_bound: float = 20.0) -> void:
	var available_tiles = []
	for i in range(objects.tile_set.get_source_count()):
		var source_id = objects.tile_set.get_source_id(i)
		var source = objects.tile_set.get_source(source_id)
		if source is TileSetAtlasSource:
			for tile_index in range(source.get_tiles_count()):
				var atlas_coords = source.get_tile_id(tile_index)
				available_tiles.append({"source_id": source_id, "atlas_coords": atlas_coords})

	if available_tiles.is_empty():
		return

	var placed_count = 0
	var max_attempts = object_count * 5 # To avoid an infinite loop
	var attempts = 0

	var half_outer_bound = outer_bound / 2.0

	while placed_count < object_count and attempts < max_attempts:
		attempts += 1
		var random_pos = Vector2(randf_range(-half_outer_bound, half_outer_bound), randf_range(-half_outer_bound, half_outer_bound))

		# Discard points inside the inner square. The outer square is already handled by the random range.
		if abs(random_pos.x) < inner_bound and abs(random_pos.y) < inner_bound:
			continue
		
		var tile_size = objects.tile_set.tile_size.x
		var is_occupied = false
		for rect in landmark_occupied_areas:
			# Grow the landmark rect to create a one-cell gap
			if rect.grow(tile_size).has_point(random_pos):
				is_occupied = true
				break
		if is_occupied:
			continue
		
		var map_coords = objects.local_to_map(random_pos)
		var can_place = true
		# Check the 3x3 area around the target cell to prevent adjacent placements
		for y in range(map_coords.y - 1, map_coords.y + 2):
			for x in range(map_coords.x - 1, map_coords.x + 2):
				if objects.get_cell_source_id(Vector2i(x, y)) != -1:
					can_place = false
					break
			if not can_place:
				break
		
		if can_place:
			var random_tile = available_tiles.pick_random()
			objects.set_cell(map_coords, random_tile.source_id, random_tile.atlas_coords)
			placed_count += 1

	print("Stage::_place_objects: placed ", placed_count, " objects out of ", object_count, " maximum possible.")

func _initialise_altars() -> void:
	altar_nodes = get_tree().get_nodes_in_group("Altars")
	
func _initialise_portals() -> void:
	portal_nodes = get_tree().get_nodes_in_group("Portals")


func _process(delta: float) -> void:
	pass
