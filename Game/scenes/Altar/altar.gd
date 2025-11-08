class_name Altar extends AnimatedObject

enum AltarState {
	AVAILABLE, # 0
	SPENT # 1
}

const AltarAnimationFrames: Dictionary[AltarState, Array] = {
	AltarState.AVAILABLE: [0, 1, 2, 3, 4, 5],
	AltarState.SPENT: [9, 8, 7, 6]
}
const AltarAnimationModes: Dictionary[AltarState, Game.AnimationMode] = {
	AltarState.AVAILABLE: Game.AnimationMode.LOOP,
	AltarState.SPENT: Game.AnimationMode.ONCE
}


func _get_animation_frames(p_state: AltarState) -> Array:
	return AltarAnimationFrames[p_state]


func _get_animation_mode(p_state: AltarState):
	return AltarAnimationModes[p_state]


func _ready() -> void:
	set_state(AltarState.AVAILABLE)


func _on_area_2d_body_entered(body: Node2D) -> void:
	if body.is_in_group("Players") and state == AltarState.AVAILABLE:
		# TODO: Implement upgrade logic
		print("Player upgraded!")
		set_state(AltarState.SPENT)
