class_name Player extends AnimatedObject

@export_group("Movement")
@export_range(1.0, 1000.0, 1.0)
var movement_speed: float = 50.0

@export_group("Animation")
@export_range(0.00, 0.50, 0.01)
var chattiness: float = 0.02

var input_movement_vector: Vector2
var is_colliding: bool = false
var position_at_frame_start: Vector2
var movement_in_frame: Vector2
var has_moved_this_frame: bool = false

enum PlayerState {
	IDLE,
	TALKING,
	RELOADING,
	RUNNING,
	SHOOTING_UP,
	SHOOTING_DOWN,
	RUNNING_AND_SHOOTING_UP,
	RUNNING_AND_SHOOTING_DOWN,
	SHOOTING_RIGHT,
	RUNNING_AND_SHOOTING_RIGHT,
	DEAD
}

const PlayerAnimationFrames: Dictionary[PlayerState, Array] = {
	PlayerState.IDLE: [0],
	PlayerState.TALKING: [9],
	PlayerState.RELOADING: [16, 17, 18, 19, 20],
	PlayerState.RUNNING: [24, 25, 26, 27],
	PlayerState.SHOOTING_UP: [33, 34],
	PlayerState.SHOOTING_DOWN: [37, 38],
	PlayerState.RUNNING_AND_SHOOTING_UP: [49, 50],
	PlayerState.RUNNING_AND_SHOOTING_DOWN: [52, 53],
	PlayerState.SHOOTING_RIGHT: [32, 35],
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: [48, 51],
	PlayerState.DEAD: [40, 41, 42, 43, 44, 45, 46]
}
const PlayerAnimationModes: Dictionary[PlayerState, Game.AnimationMode] = {
	PlayerState.IDLE: Game.AnimationMode.ONCE,
	PlayerState.TALKING: Game.AnimationMode.ONCE,
	PlayerState.RELOADING: Game.AnimationMode.ONCE,
	PlayerState.RUNNING: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_UP: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_DOWN: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_UP: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_DOWN: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_RIGHT: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: Game.AnimationMode.LOOP,
	PlayerState.DEAD: Game.AnimationMode.ONCE
}

func _get_animation_frames(p_state: PlayerState) -> Array:
	return PlayerAnimationFrames[p_state]


func _get_animation_mode(p_state: PlayerState):
	return PlayerAnimationModes[p_state]

@onready var character_body: CharacterBody2D = $CharacterBody2D
@onready var _sprite: Sprite2D = $CharacterBody2D/Sprite2D

func _ready() -> void:
	set_state(PlayerState.IDLE)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _physics_process(delta: float) -> void:
	position_at_frame_start = character_body.global_position

	input_movement_vector = Input.get_vector("move_left", "move_right", "move_up", "move_down")
	character_body.velocity = input_movement_vector * movement_speed
	is_colliding = character_body.move_and_slide()

	movement_in_frame = character_body.global_position - position_at_frame_start
	has_moved_this_frame = movement_in_frame.length() > 0.01

	var is_shooting = Input.is_action_pressed("shoot_weapon")
	var aim_direction = (get_global_mouse_position() - character_body.global_position).normalized()
	var is_aiming_up = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y < 0
	var is_aiming_down = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y > 0

	if is_shooting:
		if aim_direction.x > 0:
			_sprite.flip_h = false
		elif aim_direction.x < 0:
			_sprite.flip_h = true
	elif has_moved_this_frame:
		if movement_in_frame.x > 0:
			_sprite.flip_h = false
		elif movement_in_frame.x < 0:
			_sprite.flip_h = true

	if has_moved_this_frame:
		if is_shooting:
			if is_aiming_up:
				set_state(PlayerState.RUNNING_AND_SHOOTING_UP)
			elif is_aiming_down:
				set_state(PlayerState.RUNNING_AND_SHOOTING_DOWN)
			else:
				set_state(PlayerState.RUNNING_AND_SHOOTING_RIGHT)
		else:
			if not is_colliding:
				set_state(PlayerState.RUNNING)
	else:
		if is_shooting:
			if is_aiming_up:
				set_state(PlayerState.SHOOTING_UP)
			elif is_aiming_down:
				set_state(PlayerState.SHOOTING_DOWN)
			else:
				set_state(PlayerState.SHOOTING_RIGHT)
		else:
			if randf() < chattiness:
				set_state(PlayerState.TALKING)
			else:
				set_state(PlayerState.IDLE)
