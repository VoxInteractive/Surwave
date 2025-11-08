class_name Portal extends AnimatedObject

enum PortalState {
	ACTIVE, # 0
	DESTROYED # 1
}

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


func _get_animation_frames(state: PortalState) -> Array:
	return PortalAnimationFrames[state]


func _get_animation_mode(state: PortalState):
	return PortalAnimationModes[state]


func _ready() -> void:
	set_state(PortalState.ACTIVE)
