class_name Stage extends Node

@export var difficulty_curve: Curve

var altar_nodes: Array[Node]
var portal_nodes: Array[Node]

@onready var terrain: MeshInstance2D = $Terrain
@onready var borders: TileMapLayer = $Terrain/Borders
@onready var objects: TileMapLayer = $Terrain/Objects

func _ready() -> void:
	_place_borders()
	_place_objects()
	_initialise_altars()
	_initialise_portals()

func _place_borders() -> void:
	pass

func _place_objects() -> void:
	pass

func _initialise_altars() -> void:
	altar_nodes = get_tree().get_nodes_in_group("Altars")
	
func _initialise_portals() -> void:
	portal_nodes = get_tree().get_nodes_in_group("Portals")

func _process(delta: float) -> void:
	pass
