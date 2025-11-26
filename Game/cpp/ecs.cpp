#include "components/enemy.h"
#include "components/singletons.h"

#include "prefabs/character2d.h"
#include "prefabs/enemy.h"

#include "systems/timer_tick.h"
#include "systems/enemy_movement.h"
#include "systems/enemy_death.h"
#include "systems/enemy_take_damage.h"
#include "systems/player_take_damage.h"
#include "systems/enemy_animation.h"
#include "systems/enemy_count_update.h"
#include "systems/velocity_to_position.h"
