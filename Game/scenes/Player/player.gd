class_name Player extends AnimatedObject

@export_category("Movement")
@export_range(1.0, 200.0, 1.0)
var movement_speed: float = 70.0
var adjusted_movement_speed := movement_speed
@export_range(0.0, 1.0, 0.01)
var movement_speed_penalty_when_shooting: float = 0.4

@export_category("Cooldowns")
@export_range(0.5, 10.0, 0.1)
var shockwave_cooldown: float = 5.0

var input_movement_vector: Vector2
var is_colliding: bool = false
var position_at_frame_start: Vector2
var movement_in_frame: Vector2
var has_moved_this_frame: bool = false

var shoot_weapon_timer: float
var can_shoot_weapon: bool = false
var just_fired_weapon: bool = false

var shockwave_timer: float = 0.0
var can_fire_shockwave: bool = false

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
	DYING
}

const PlayerAnimationFrames: Dictionary[PlayerState, Array] = {
	PlayerState.IDLE: [0, 2, 3, 4, 5],
	PlayerState.TALKING: [9], # unused
	PlayerState.RELOADING: [16, 17, 18, 19, 20], # unused
	PlayerState.RUNNING: [24, 25, 26, 27],
	PlayerState.SHOOTING_RIGHT: [32, 35],
	PlayerState.SHOOTING_UP: [33, 34],
	PlayerState.SHOOTING_DOWN: [37, 38],
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: [48, 51],
	PlayerState.RUNNING_AND_SHOOTING_UP: [49, 50],
	PlayerState.RUNNING_AND_SHOOTING_DOWN: [52, 53],
	PlayerState.DYING: [40, 41, 42, 43, 44, 45, 46]
}
const PlayerAnimationModes: Dictionary[PlayerState, Game.AnimationMode] = {
	PlayerState.IDLE: Game.AnimationMode.LOOP,
	PlayerState.TALKING: Game.AnimationMode.ONCE,
	PlayerState.RELOADING: Game.AnimationMode.ONCE,
	PlayerState.RUNNING: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_RIGHT: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_UP: Game.AnimationMode.LOOP,
	PlayerState.SHOOTING_DOWN: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_UP: Game.AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_DOWN: Game.AnimationMode.LOOP,
	PlayerState.DYING: Game.AnimationMode.ONCE
}
const first_shooting_animation_frames: Array[int] = [
	PlayerAnimationFrames[PlayerState.SHOOTING_RIGHT][0],
	PlayerAnimationFrames[PlayerState.SHOOTING_UP][0],
	PlayerAnimationFrames[PlayerState.SHOOTING_DOWN][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_RIGHT][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_UP][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_DOWN][0]
]

const PROJECTILE = preload("uid://cdgbhqr7en0pv")

func _get_animation_frames(p_state: PlayerState) -> Array:
	return PlayerAnimationFrames[p_state]

func _get_animation_mode(p_state: PlayerState):
	return PlayerAnimationModes[p_state]

@onready var world: FlecsWorld = get_node("../World")
@onready var character_body: CharacterBody2D = $CharacterBody2D
@onready var _sprite: Sprite2D = $CharacterBody2D/Sprite2D
@onready var action_vfx_animation_player: AnimationPlayer = $CharacterBody2D/ActionVFX/AnimationPlayer

func _ready() -> void:
	animation_frame_changed.connect(_on_animation_frame_changed)
	set_state(PlayerState.IDLE)
	shoot_weapon_timer = animation_interval
	can_shoot_weapon = true


func _process(delta: float) -> void:
	just_fired_weapon = false # Reset at the beginning of every frame
	_tick_cooldowns(delta)
	_handle_input(delta)
	super._process(delta);


func _on_animation_frame_changed(frame: int) -> void:
	if first_shooting_animation_frames.has(frame):
		just_fired_weapon = true
		if can_shoot_weapon:
			_fire_projectile()


func _fire_projectile() -> void:
	var projectile: Node2D = PROJECTILE.instantiate()
	projectile.global_position = character_body.global_position
	projectile.direction = (get_global_mouse_position() - character_body.global_position).normalized()
	add_child(projectile)
	
	can_shoot_weapon = false
	shoot_weapon_timer = 0.0


func _tick_cooldowns(delta: float) -> void:
	shoot_weapon_timer += delta
	if not can_shoot_weapon and shoot_weapon_timer >= animation_interval:
		can_shoot_weapon = true

	shockwave_timer += delta
	if not can_fire_shockwave and shockwave_timer >= shockwave_cooldown:
		can_fire_shockwave = true


func _handle_input(delta: float) -> void:
	position_at_frame_start = character_body.global_position

	var is_shooting_input = Input.is_action_pressed("shoot_weapon")

	var is_shooting = is_shooting_input or shoot_weapon_timer < animation_interval

	adjusted_movement_speed = movement_speed * (1 - movement_speed_penalty_when_shooting) if is_shooting else movement_speed
	var aim_direction = (get_global_mouse_position() - character_body.global_position).normalized()
	var is_aiming_up = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y < 0
	var is_aiming_down = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y > 0

	input_movement_vector = Input.get_vector("move_left", "move_right", "move_up", "move_down")
	character_body.velocity = input_movement_vector * adjusted_movement_speed

	is_colliding = character_body.move_and_slide()
	world.set_singleton_component("PlayerPosition", character_body.global_position)

	movement_in_frame = character_body.global_position - position_at_frame_start
	has_moved_this_frame = movement_in_frame.length() > 0.01

	var new_state = state

	# Sprite Animations
	if is_shooting:
		if aim_direction.x > 0:
			_sprite.flip_h = false
		elif aim_direction.x < 0:
			_sprite.flip_h = true
	elif has_moved_this_frame:
		if Input.is_action_pressed("move_right") and movement_in_frame.x > 0:
			_sprite.flip_h = false
		elif Input.is_action_pressed("move_left") and movement_in_frame.x < 0:
			_sprite.flip_h = true

	if has_moved_this_frame:
		if is_shooting:
			if is_aiming_up:
				new_state = PlayerState.RUNNING_AND_SHOOTING_UP
			elif is_aiming_down:
				new_state = PlayerState.RUNNING_AND_SHOOTING_DOWN
			else:
				new_state = PlayerState.RUNNING_AND_SHOOTING_RIGHT
		else:
			if not is_colliding:
				new_state = PlayerState.RUNNING
	else:
		if is_shooting:
			if is_aiming_up:
				new_state = PlayerState.SHOOTING_UP
			elif is_aiming_down:
				new_state = PlayerState.SHOOTING_DOWN
			else:
				new_state = PlayerState.SHOOTING_RIGHT
		else:
			new_state = PlayerState.IDLE
	
	set_state(new_state)

	# Action VFX
	if Input.is_action_just_pressed("shockwave") && can_fire_shockwave:
		action_vfx_animation_player.play("Shockwave")
		can_fire_shockwave = false
		shockwave_timer = 0.0
