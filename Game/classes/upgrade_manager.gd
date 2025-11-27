class_name UpgradeManager extends Node

enum Upgradeable {
	PROJECTILE_DAMAGE,
	PROJECTILE_COUNT,
	SHOCKWAVE,
	SPEED
}

const UPGRADE_INFO: Dictionary[Upgradeable, Array] = {
	Upgradeable.PROJECTILE_DAMAGE: [
		"Increase the damage of your gun",
		[
			["Base", 1.0],
			["Damage I", 2.0],
			["Damage II", 3.0],
			["Damage III", 4.0],
			["Damage IV", 5.0]
		]
	],
	Upgradeable.PROJECTILE_COUNT: [
		"Fire multiple projectiles each time you fire your gun",
		[
			["Base", 1],
			["Multi-shot I", 2],
			["Multi-shot II", 3],
			["Multi-shot III", 4],
			["Multi-shot IV", 5]
		]
	],
	Upgradeable.SHOCKWAVE: [
		"Increase the area of effect of your shockwave",
		[
			["Base", 0.3],
			["Shockwave I", 0.564],
			["Shockwave II", 0.738],
			["Shockwave III", 0.879],
			["Shockwave IV", 1.0]
		]
	],
	Upgradeable.SPEED: [
		"Increase your movement speed",
		[
			["Base", 65],
			["Speed I", 80],
			["Speed II", 100],
			["Speed III", 125],
			["Speed IV", 155]
		]
	]
}

const TIER_COSTS = [10, 20, 40, 80]

# Holds the current upgrade tier that the player has for each upgradeable type
var upgrade_tiers: Dictionary[Upgradeable, int] = {
	Upgradeable.PROJECTILE_DAMAGE: 0,
	Upgradeable.PROJECTILE_COUNT: 0,
	Upgradeable.SHOCKWAVE: 0,
	Upgradeable.SPEED: 0
}

# Holds the gem balance of the player
var gems: int = 0

@onready var world: FlecsWorld = get_node("../../World")

func get_upgrade_value(upgradeable: Upgradeable) -> Variant:
	return UPGRADE_INFO[upgradeable][1][upgrade_tiers[upgradeable]][1]

func upgrade(upgradeable: Upgradeable) -> void:
	if not UPGRADE_INFO.has(upgradeable):
		return

	var tiers: Array = UPGRADE_INFO[upgradeable][1]
	var current_tier: int = upgrade_tiers[upgradeable]
	if current_tier >= tiers.size() - 1:
		return

	current_tier += 1
	upgrade_tiers[upgradeable] = current_tier

	match upgradeable:
		Upgradeable.PROJECTILE_DAMAGE:
			_update_enemy_projectile_damage(float(tiers[current_tier][1]))
		_:
			pass

func _update_enemy_projectile_damage(new_damage: float) -> void:
	if world == null:
		push_warning("UpgradeManager: Cannot set EnemyTakeDamageSettings without a FlecsWorld reference.")
		return

	world.set_singleton_component("EnemyTakeDamageSettings", {
		"projectile_damage": new_damage
	})
