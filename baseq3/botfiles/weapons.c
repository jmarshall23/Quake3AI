//===========================================================================
//
// Name:			weapons.c
// Function:		weapon configuration
// Programmer:		Mr Elusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		4 (real tabs)
//===========================================================================

#include "inv.h"
#include "game.h"

#define VEC_ORIGIN						{0, 0, 0}
//projectile flags
#define PFL_WINDOWDAMAGE			1		//projectile damages through window
#define PFL_RETURN					2		//set when projectile returns to owner
//weapon flags
#define WFL_FIRERELEASED			1		//set when projectile is fired with key-up event
//damage types
#define DAMAGETYPE_IMPACT			1		//damage on impact
#define DAMAGETYPE_RADIAL			2		//radial damage
#define DAMAGETYPE_VISIBLE			4		//damage to all entities visible to the projectile
#define DAMAGETYPE_IGNOREARMOR	8		//projectile goes right through armor

#define WEAPONINDEX_GAUNTLET			1
#define WEAPONINDEX_MACHINEGUN			2
#define WEAPONINDEX_SHOTGUN				3
#define WEAPONINDEX_GRENADE_LAUNCHER	4
#define WEAPONINDEX_ROCKET_LAUNCHER		5
#define WEAPONINDEX_LIGHTNING			6
#define WEAPONINDEX_RAILGUN				7
#define WEAPONINDEX_PLASMAGUN			8
#define WEAPONINDEX_BFG					9
#define WEAPONINDEX_GRAPPLING_HOOK		10

//===========================================================================
// Gauntlet
//===========================================================================

projectileinfo //for Gauntlet
{
	name				"gauntletdamage"
	damage				50
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Gauntlet
{
	name				"Gauntlet"
	number				WEAPONINDEX_GAUNTLET
	projectile			"gauntletdamage"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Machinegun
//===========================================================================

projectileinfo //for Machinegun
{
	name				"machinegunbullet"
	damage				8
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Machinegun
{
	name				"Machinegun"
	number				WEAPONINDEX_MACHINEGUN
	projectile			"machinegunbullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Shotgun
//===========================================================================

projectileinfo //for Shotgun
{
	name				"shotgunbullet"
	damage				10
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Shotgun
{
	name				"Shotgun"
	number				WEAPONINDEX_SHOTGUN
	projectile			"shotgunbullet"
	numprojectiles		11
	speed				0
} //end weaponinfo

//===========================================================================
// Grenade Launcher
//===========================================================================

projectileinfo //for Grenade Launcher
{
	name				"grenade"
	damage				120
	radius				160
	damagetype			$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo //Grenade Launcher
{
	name				"Grenade Launcher"
	number				WEAPONINDEX_GRENADE_LAUNCHER
	projectile			"grenade"
	numprojectiles		1
	speed				700
} //end weaponinfo

//===========================================================================
// Rocket Launcher
//===========================================================================

projectileinfo //for Rocket Launcher
{
	name				"rocket"
	damage				100
	radius				120
	damagetype			$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo //Rocket Launcher
{
	name				"Rocket Launcher"
	number				WEAPONINDEX_ROCKET_LAUNCHER
	projectile			"rocket"
	numprojectiles		1
	speed				900
} //end weaponinfo

//===========================================================================
// Lightning
//===========================================================================

projectileinfo //for Lightning
{
	name				"lightning"
	damage				24
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Railgun
{
	name				"Lightning Gun"
	number				WEAPONINDEX_LIGHTNING
	projectile			"lightning"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Railgun
//===========================================================================

projectileinfo //for Railgun
{
	name				"rail"
	damage				100
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Railgun
{
	name				"Railgun"
	number				WEAPONINDEX_RAILGUN
	projectile			"rail"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Plasma Gun
//===========================================================================

projectileinfo //for Plasma Gun
{
	name				"plasma"
	damage				20
	radius				20
	damagetype			$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo //Plasma Gun
{
	name				"Plasma Gun"
	number				WEAPONINDEX_PLASMAGUN
	projectile			"plasma"
	numprojectiles		1
	speed				2000
} //end weaponinfo

//===========================================================================
// BFG10K
//===========================================================================

projectileinfo //for BFG10K
{
	name				"bfgexploision"
	damage				40
	radius				100
	damagetype			$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo //BFG10K
{
	name				"BFG10K"
	number				WEAPONINDEX_BFG
	projectile			"bfgexploision"
	numprojectiles		1
	speed				0
} //end weaponinfo

