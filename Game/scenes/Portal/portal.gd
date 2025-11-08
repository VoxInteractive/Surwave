class_name Portal extends AnimatedObject

enum PortalState {
	ACTIVE,
	DESTROYED
}
var state: PortalState = PortalState.ACTIVE

var PortalAnimationFrames := {
	PortalState.ACTIVE: [0, 1, 2, 3],
	PortalState.DESTROYED: [9, 8, 7, 6, 5, 4]
} if randf() < 0.5 else {
	PortalState.ACTIVE: [10, 11, 12, 13],
	PortalState.DESTROYED: [19, 18, 17, 16, 15, 14]
}
const PortalAnimationModes: Dictionary[PortalState, Game.AnimationMode] = {
	PortalState.ACTIVE: Game.AnimationMode.LOOP,
	PortalState.DESTROYED: Game.AnimationMode.ONCE
}
var animation_interval: float = 0.25
var _animation_frames: Array = PortalAnimationFrames[PortalState.ACTIVE]
var _animation_timer: float = 0.0
var _animation_current_frame: int = 0 # index of the current frame within the particular ...AnimationFrames array

@onready var sprite: Sprite2D = $StaticBody2D/Sprite2D


func set_state(new_state: PortalState) -> void:
	if state == new_state: return
	state = new_state
	_animation_frames = PortalAnimationFrames[state]
	_animation_current_frame = 0
	_animation_timer = 0.0


func _animate(delta: float) -> void:
	_animation_timer += delta
	if ((PortalAnimationModes[state] == Game.AnimationMode.ONCE and _animation_current_frame >= _animation_frames.size() - 1)
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
