class_name StatusOverlay extends CanvasLayer

@onready var gem_count_label: Label = $OverlayMargin/GemCount/VBoxContainer/GemCount

func _ready() -> void:
	await get_tree().process_frame
	var stage = get_tree().get_first_node_in_group("stage")
	if stage:
		stage.gem_balance_changed.connect(update_gem_count)
		update_gem_count(stage.gem_balance)

func update_gem_count(new_balance: int) -> void:
	gem_count_label.text = str(new_balance)
