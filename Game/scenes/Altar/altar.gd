extends Node2D

enum AltarState {
	AVAILABLE,
	SPENT
}
var state: AltarState = AltarState.AVAILABLE

const AltarAnimationFrames: Dictionary[AltarState, Array] = {
	AltarState.AVAILABLE: [0, 1, 2, 3, 4, 5],
	AltarState.SPENT: [9, 8, 7, 6]
}

const AltarAnimationModes: Dictionary[AltarState, Game.AnimationMode] = {
	AltarState.AVAILABLE: Game.AnimationMode.LOOP,
	AltarState.SPENT: Game.AnimationMode.ONCE
}
var animation_interval: float = 0.2
var _animation_frames: Array = AltarAnimationFrames[AltarState.AVAILABLE]
var _animation_timer: float = 0.0
var _animation_current_frame: int = 0 # index of the current frame within the particular ...AnimationFrames array

@onready var sprite: Sprite2D = $StaticBody2D/Sprite2D


func set_state(new_state: AltarState) -> void:
	if state == new_state: return
	state = new_state
	_animation_frames = AltarAnimationFrames[state]
	_animation_current_frame = 0
	_animation_timer = 0.0


func _animate(delta: float) -> void:
	_animation_timer += delta
	if ((AltarAnimationModes[state] == Game.AnimationMode.ONCE and _animation_current_frame >= _animation_frames.size() - 1)
		or _animation_timer < animation_interval):
			return
	
	var frame_count: int = _animation_frames.size()
	if _animation_timer >= animation_interval:
		_animation_timer = 0.0
		_animation_current_frame = (_animation_current_frame + 1) % frame_count
		sprite.frame = _animation_frames[_animation_current_frame]


func _ready() -> void:
	pass # Replace with function body.


func _process(delta: float) -> void:
	_animate(delta)


func _on_area_2d_body_entered(body: Node2D) -> void:
	if body.is_in_group("Players") and state == AltarState.AVAILABLE:
		# TODO: Implement upgrade logic
		print("Player upgraded!")
		set_state(AltarState.SPENT)
