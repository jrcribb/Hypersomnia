#pragma once
#include "test_scenes/test_id_to_pool_id.h"

enum class test_scene_sound_id {
	// GEN INTROSPECTOR enum class test_scene_sound_id
	BLANK,

	BILMER2000_MUZZLE,
	PRO90_MUZZLE,
	ASSAULT_RIFLE_MUZZLE,
	LEWSII_MUZZLE,
	KEK9_MUZZLE,
	SN69_MUZZLE,
	AO44_MUZZLE,
	CALICO_MUZZLE,
	PLASMA_MUZZLE,
	BLUNAZ_MUZZLE,
	WARX_FQ12_MUZZLE,
	ELECTRIC_PROJECTILE_FLIGHT,
	STEEL_PROJECTILE_FLIGHT,
	ELECTRIC_DISCHARGE_EXPLOSION,

	IMPACT,
	DEATH,
	BULLET_PASSES_THROUGH_HELD_ITEM,

	WIND,
	ENGINE,

	LOW_AMMO_CUE,
#if 0
	VINDICATOR_LOW_AMMO_CUE,
#endif

	FIREARM_ENGINE,
#if 0
	FIREARM_HEAT_ENGINE,
#endif

	HEAVY_HEAT_START,
	LIGHT_HEAT_START,

	CAST_SUCCESSFUL,
	CAST_UNSUCCESSFUL,

	CAST_CHARGING,

	EXPLOSION,
	GREAT_EXPLOSION,

	INTERFERENCE_EXPLOSION,
	PED_EXPLOSION,

	GRENADE_UNPIN,
	GRENADE_THROW,

	ITEM_THROW,

	COLLISION_METAL_WOOD,
	COLLISION_METAL_METAL,

	COLLISION_KNIFE_WOOD,
	COLLISION_KNIFE_METAL,

	COLLISION_GRENADE,

	STANDARD_FOOTSTEP,
	FOOTSTEP_DIRT,
	FOOTSTEP_FLOOR,

	STANDARD_HOLSTER,

	STANDARD_KNIFE_DRAW,
	STANDARD_GUN_DRAW,
	STANDARD_PISTOL_DRAW,
	STANDARD_SMG_DRAW,
	LEWSII_DRAW,
	MAGAZINE_DRAW,
	PLASMA_DRAW,

	BACKPACK_WEAR,
	BACKPACK_INSERT,

	STEAM_BURST,

	GLASS_DAMAGE,
	COLLISION_GLASS,

	WOOD_DAMAGE,

	STEEL_PROJECTILE_DESTRUCTION,

	ELECTRIC_RICOCHET,
	STEEL_RICOCHET,

	AQUARIUM_AMBIENCE_LEFT,
	AQUARIUM_AMBIENCE_RIGHT,

	HUMMING_DISABLED,
	LOUDY_FAN,

	BEEP,

	BOMB_PLANTING,
	STARTED_DEFUSING,
	POWER_OUT,

	BOMB_EXPLOSION,

	MT_START,

	MT_BOMB_PLANTED,
	MT_BOMB_DEFUSED,

	MT_ITS_TOO_LATE_RUN,

	MT_METROPOLIS_WINS,
	MT_RESISTANCE_WINS,

	RE_BOMB_PLANTED,
	RE_SECURING_OBJECTIVE,

	REVOLVER_CHAMBERING,
	HEAVY_PISTOL_CHAMBERING,
	MEDIUM_PISTOL_CHAMBERING,
	LIGHT_PISTOL_CHAMBERING,
	RIFLE_CHAMBERING,
	BILMER_CHAMBERING,
	CALICO_CHAMBERING,
	ZAMIEC_CHAMBERING,
	AUTOMATIC_SHOTGUN_CHAMBERING,
	ELECTRIC_CHAMBERING,
	PRO90_CHAMBERING,

	TRIGGER_PULL,


	STANDARD_START_UNLOAD,

	STANDARD_RIFLE_FINISH_UNLOAD,
	STANDARD_RIFLE_START_LOAD,
	STANDARD_RIFLE_FINISH_LOAD,

	STANDARD_PISTOL_FINISH_UNLOAD,
	AO44_FINISH_LOAD,

	STANDARD_LEARNT_SPELL,

	STANDARD_KNIFE_PRIMARY,
	STANDARD_KNIFE_SECONDARY,

	STANDARD_KNIFE_PRIMARY_IMPACT,
	STANDARD_KNIFE_SECONDARY_IMPACT,

	STANDARD_KNIFE_CLASH,

	STANDARD_KNIFE_SENTIENCE_IMPACT,

	SHEATH_KNIFE,

	SKULL_ROCKET_FLIGHT,
	SKULL_ROCKET_DESTRUCTION,
	ELON_HRL_MUZZLE,

	STANDARD_DASH,

	GRADOBICIE_MUZZLE,
	GRADOBICIE_CHAMBERING,
	SHOTGUN_INSERT_CHARGE,

	ICE_PROJECTILE_DESTRUCTION,

	BAKA47_MUZZLE,
	BULWARK_MUZZLE,

	ZAMIEC_MUZZLE,
	ZAMIEC_DRAW,

	ASSAULT_RATTLE_HUMMING,

	ASSAULT_RATTLE_PRIMARY,
	ASSAULT_RATTLE_SECONDARY,

	ASSAULT_RATTLE_PRIMARY_IMPACT,
	ASSAULT_RATTLE_SECONDARY_IMPACT,

	ASSAULT_RATTLE_CLASH,
	ASSAULT_RATTLE_DRAW,

	FLASHBANG_EXPLOSION,

	FLASH_NOISE,

	COLLISION_FLASHBANG,

	AIR_DUCT_IMPACT,
	FOOTSTEP_AIR_DUCT,

	FOOTSTEP_FENCE,

	SZTURM_MUZZLE,

	SZTURM_FINISH_UNLOAD,
	SZTURM_FINISH_LOAD,

	AIR_DUCT_COLLISION,

	DEAGLE_MUZZLE,
	DEAGLE_CHAMBERING,

	HPSR_MUZZLE,
	HPSR_CHAMBERING,
	HPSR_ROUND_DESTRUCTION,

	BULLDUP2000_MUZZLE,

	BULLDUP2000_START_UNLOAD,
	BULLDUP2000_FINISH_UNLOAD,
	BULLDUP2000_START_LOAD,
	BULLDUP2000_CHAMBERING,

	GALILEA_MUZZLE,
	HUNTER_MUZZLE,
	HUNTER_CHAMBERING,
	STANDARD_SNIPER_DRAW,

	SMALL_SMG_DRAW,

	CYBERSPRAY_MUZZLE,
	CYBERSPRAY_CHAMBERING,

	ELON_HRL_LEVER,
	
	COVERT_MUZZLE,
	HEADSHOT,

	CORPSE_CATCH_FIRE,

	ANNOUNCE_FIRSTBLOOD,
	ANNOUNCE_LUDACRISSKILL,
	ANNOUNCE_HOLYSHIT,
	ANNOUNCE_PREPARE,
	ANNOUNCE_RAMPAGE,
	ANNOUNCE_WICKEDSICK,
	ANNOUNCE_MEGAKILL,
	ANNOUNCE_ONEANDONLY,
	ANNOUNCE_MAYTHEFORCE,
	ANNOUNCE_MONSTERKILL,
	ANNOUNCE_HUMILIATION,
	ANNOUNCE_HEADSHOT,
	ANNOUNCE_GODLIKE,
	ANNOUNCE_ULTRAKILL,
	ANNOUNCE_MULTIKILL,
	ANNOUNCE_KILLINGSPREE,
	ANNOUNCE_UNSTOPPABLE,

	ARABESQUE,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto to_sound_id(const test_scene_sound_id id) {
	return to_pool_id<assets::sound_id>(id);
}
