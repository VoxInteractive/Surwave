class_name UpgradeManager extends Node

enum ShockwaveTier {
	TIER_1,
	TIER_2,
	TIER_3
}

const SHOCKWAVE_RADIUS_MULTIPLIERS: Array[float] = [0.3, 0.6, 1.0]

var shockwave_tier: int = ShockwaveTier.TIER_1

func get_shockwave_radius_multiplier() -> float:
	return SHOCKWAVE_RADIUS_MULTIPLIERS[clampi(shockwave_tier, 0, SHOCKWAVE_RADIUS_MULTIPLIERS.size() - 1)]

func set_shockwave_tier(new_tier: int) -> void:
	shockwave_tier = clampi(new_tier, 0, SHOCKWAVE_RADIUS_MULTIPLIERS.size() - 1)

func upgrade_shockwave_tier() -> void:
	set_shockwave_tier(shockwave_tier + 1)
