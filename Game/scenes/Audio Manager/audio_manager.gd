extends Node

var world: FlecsWorld

# SFX
@onready var bug_small_die: AudioStreamPlayer = $SFXPlayers/BugSmallDie
@onready var bug_humanoid_die: AudioStreamPlayer = $SFXPlayers/BugHumanoidDie
@onready var bug_large_die: AudioStreamPlayer = $SFXPlayers/BugLargeDie
@onready var enemy_hurt: AudioStreamPlayer = $SFXPlayers/EnemyHurt
@onready var gem_collect: AudioStreamPlayer = $SFXPlayers/GemCollect
@onready var gem_drop: AudioStreamPlayer = $SFXPlayers/GemDrop
@onready var player_hurt: AudioStreamPlayer = $SFXPlayers/PlayerHurt
@onready var projectile: AudioStreamPlayer = $SFXPlayers/Projectile
@onready var shockwave: AudioStreamPlayer = $SFXPlayers/Shockwave
@onready var upgrade_acquired: AudioStreamPlayer = $SFXPlayers/UpgradeAcquired
# Music
@onready var defeat: AudioStreamPlayer = $MusicPlayers/Defeat
@onready var victory: AudioStreamPlayer = $MusicPlayers/Victory
@onready var sound_track: AudioStreamPlayer = $MusicPlayers/SoundTrack


func connect_to_flecs_signal():
	if world == null:
		push_warning("AudioManager: connect_to_flecs_signal called but the world instance is null")
		return
	world.flecs_signal_emitted.connect(_on_flecs_signal)


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	match signal_name:
		"enemy_died":
			match data.enemy_type:
				"BugSmall": await _delay(); bug_small_die.play()
				"BugHumanoid": await _delay(); bug_humanoid_die.play()
				"BugLarge": await _delay(); bug_large_die.play()
		"enemy_took_damage":
			enemy_hurt.play()


func _delay(delay: float = 0.5):
	await get_tree().create_timer(delay).timeout
