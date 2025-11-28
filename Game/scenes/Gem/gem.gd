class_name Gem extends Node2D

@export var timer: Timer
@export var blink_duration: float = 5.0
@export var blink_interval: float = 0.2

var is_about_to_expire: bool = false
var blink_timer: float = 0.0

@onready var sprite: Sprite2D = $Sprite2D


func _process(delta: float) -> void:
	if not is_about_to_expire: return
	# Blink by toggling visibility when close to full expiry
	blink_timer += delta
	if blink_timer > blink_interval:
		sprite.visible = not sprite.visible
		blink_timer = 0.0


func _on_expiry() -> void:
	if not is_about_to_expire: # Start blinking
		timer.start(blink_duration)
		is_about_to_expire = true
	else: # Fully expired (ExpirationTimer.wait_time + blink_duration has elapsed)
		queue_free()


func _on_area_entered(body: Node2D) -> void:
	if not body.is_in_group("Players"): return
	var player_root: Node = body.get_parent()
	if player_root == null: return
	player_root.gem_collected.emit(1)
	queue_free()
