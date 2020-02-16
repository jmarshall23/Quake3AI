// bot_weapons.c
//

#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"


//#define DEBUG_AI_WEAP

//structure field offsets
#define WEAPON_OFS(x) (int)&(((weaponinfo_t *)0)->x)
#define PROJECTILE_OFS(x) (int)&(((projectileinfo_t *)0)->x)

//weapon definition // bk001212 - static
static fielddef_t weaponinfo_fields[] =
{
{"number", WEAPON_OFS(number), FT_INT},						//weapon number
{"name", WEAPON_OFS(name), FT_STRING},							//name of the weapon
{"level", WEAPON_OFS(level), FT_INT},
{"model", WEAPON_OFS(model), FT_STRING},						//model of the weapon
{"weaponindex", WEAPON_OFS(weaponindex), FT_INT},			//index of weapon in inventory
{"flags", WEAPON_OFS(flags), FT_INT},							//special flags
{"projectile", WEAPON_OFS(projectile), FT_STRING},			//projectile used by the weapon
{"numprojectiles", WEAPON_OFS(numprojectiles), FT_INT},	//number of projectiles
{"hspread", WEAPON_OFS(hspread), FT_FLOAT},					//horizontal spread of projectiles (degrees from middle)
{"vspread", WEAPON_OFS(vspread), FT_FLOAT},					//vertical spread of projectiles (degrees from middle)
{"speed", WEAPON_OFS(speed), FT_FLOAT},						//speed of the projectile (0 = instant hit)
{"acceleration", WEAPON_OFS(acceleration), FT_FLOAT},		//"acceleration" * time (in seconds) + "speed" = projectile speed
{"recoil", WEAPON_OFS(recoil), FT_FLOAT | FT_ARRAY, 3},		//amount of recoil the player gets from the weapon
{"offset", WEAPON_OFS(offset), FT_FLOAT | FT_ARRAY, 3},		//projectile start offset relative to eye and view angles
{"angleoffset", WEAPON_OFS(angleoffset), FT_FLOAT | FT_ARRAY, 3},//offset of the shoot angles relative to the view angles
{"extrazvelocity", WEAPON_OFS(extrazvelocity), FT_FLOAT},//extra z velocity the projectile gets
{"ammoamount", WEAPON_OFS(ammoamount), FT_INT},				//ammo amount used per shot
{"ammoindex", WEAPON_OFS(ammoindex), FT_INT},				//index of ammo in inventory
{"activate", WEAPON_OFS(activate), FT_FLOAT},				//time it takes to select the weapon
{"reload", WEAPON_OFS(reload), FT_FLOAT},						//time it takes to reload the weapon
{"spinup", WEAPON_OFS(spinup), FT_FLOAT},						//time it takes before first shot
{"spindown", WEAPON_OFS(spindown), FT_FLOAT},				//time it takes before weapon stops firing
{NULL, 0, 0, 0}
};

//projectile definition
static fielddef_t projectileinfo_fields[] =
{
{"name", PROJECTILE_OFS(name), FT_STRING},					//name of the projectile
{"model", WEAPON_OFS(model), FT_STRING},						//model of the projectile
{"flags", PROJECTILE_OFS(flags), FT_INT},						//special flags
{"gravity", PROJECTILE_OFS(gravity), FT_FLOAT},				//amount of gravity applied to the projectile [0,1]
{"damage", PROJECTILE_OFS(damage), FT_INT},					//damage of the projectile
{"radius", PROJECTILE_OFS(radius), FT_FLOAT},				//radius of damage
{"visdamage", PROJECTILE_OFS(visdamage), FT_INT},			//damage of the projectile to visible entities
{"damagetype", PROJECTILE_OFS(damagetype), FT_INT},		//type of damage (combination of the DAMAGETYPE_? flags)
{"healthinc", PROJECTILE_OFS(healthinc), FT_INT},			//health increase the owner gets
{"push", PROJECTILE_OFS(push), FT_FLOAT},						//amount a player is pushed away from the projectile impact
{"detonation", PROJECTILE_OFS(detonation), FT_FLOAT},		//time before projectile explodes after fire pressed
{"bounce", PROJECTILE_OFS(bounce), FT_FLOAT},				//amount the projectile bounces
{"bouncefric", PROJECTILE_OFS(bouncefric), FT_FLOAT}, 	//amount the bounce decreases per bounce
{"bouncestop", PROJECTILE_OFS(bouncestop), FT_FLOAT},		//minimum bounce value before bouncing stops
//recurive projectile definition??
{NULL, 0, 0, 0}
};

static structdef_t weaponinfo_struct =
{
	sizeof(weaponinfo_t), weaponinfo_fields
};
static structdef_t projectileinfo_struct =
{
	sizeof(projectileinfo_t), projectileinfo_fields
};

//weapon configuration: set of weapons with projectiles
typedef struct weaponconfig_s
{
	int numweapons;
	int numprojectiles;
	projectileinfo_t* projectileinfo;
	weaponinfo_t* weaponinfo;
} weaponconfig_t;

//the bot weapon state
typedef struct bot_weaponstate_s
{
	struct weightconfig_s* weaponweightconfig;		//weapon weight configuration
	int* weaponweightindex;							//weapon weight index
} bot_weaponstate_t;

static bot_weaponstate_t* botweaponstates[MAX_CLIENTS + 1];
static weaponconfig_t* weaponconfig;

/*
=========================
BotValidWeaponNumber
=========================
*/
int BotValidWeaponNumber(int weaponnum)
{
	if (weaponnum <= 0 || weaponnum > weaponconfig->numweapons)
	{
		G_Error("weapon number out of range\n");
		return qfalse;
	}
	return qtrue;
}

/*
=========================
BotWeaponStateFromHandle
=========================
*/
bot_weaponstate_t* BotWeaponStateFromHandle(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
		G_Error("move state handle %d out of range\n", handle);
		return NULL;
	}
	if (!botweaponstates[handle])
	{
		G_Error("invalid move state %d\n", handle);
		return NULL;
	}
	return botweaponstates[handle];
}

/*
=========================
LoadWeaponConfig
=========================
*/
weaponconfig_t* LoadWeaponConfig(char* filename)
{
	int max_weaponinfo, max_projectileinfo;
	pc_token_t token;
	char path[MAX_QPATH];
	int i, j;
	int source;
	weaponconfig_t* wc;
	weaponinfo_t weaponinfo;

	max_weaponinfo = (int)LibVarValue("max_weaponinfo", "32");
	if (max_weaponinfo < 0)
	{
		G_Error("max_weaponinfo = %d\n", max_weaponinfo);
		max_weaponinfo = 32;
		LibVarSet("max_weaponinfo", "32");
	}
	max_projectileinfo = (int)LibVarValue("max_projectileinfo", "32");
	if (max_projectileinfo < 0)
	{
		G_Error("max_projectileinfo = %d\n", max_projectileinfo);
		max_projectileinfo = 32;
		LibVarSet("max_projectileinfo", "32");
	}
	strncpy(path, filename, MAX_QPATH);
	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	source = trap_PC_LoadSource(path);
	if (!source)
	{
		G_Error("counldn't load %s\n", path);
		return NULL;
	}
	//initialize weapon config
	wc = (weaponconfig_t*)G_AllocClearedMemory(sizeof(weaponconfig_t) +
		max_weaponinfo * sizeof(weaponinfo_t) +
		max_projectileinfo * sizeof(projectileinfo_t));
	wc->weaponinfo = (weaponinfo_t*)((char*)wc + sizeof(weaponconfig_t));
	wc->projectileinfo = (projectileinfo_t*)((char*)wc->weaponinfo +
		max_weaponinfo * sizeof(weaponinfo_t));
	wc->numweapons = max_weaponinfo;
	wc->numprojectiles = 0;
	//parse the source file
	while (trap_PC_ReadToken(source, &token))
	{
		if (!strcmp(token.string, "weaponinfo"))
		{
			memset(&weaponinfo, 0, sizeof(weaponinfo_t));
			if (!trap_PC_ReadStructure(source, &weaponinfo_struct, (char*)&weaponinfo))
			{
				//FreeMemory(wc);
				trap_PC_FreeSource(source);
				return NULL;
			}

			if (weaponinfo.number < 0 || weaponinfo.number >= max_weaponinfo)
			{
				G_Error("weapon info number %d out of range in %s\n", weaponinfo.number, path);
				//FreeMemory(wc);
				trap_PC_FreeSource(source);
				return NULL;
			}
			memcpy(&wc->weaponinfo[weaponinfo.number], &weaponinfo, sizeof(weaponinfo_t));
			wc->weaponinfo[weaponinfo.number].valid = qtrue;
		} 
		else if (!strcmp(token.string, "projectileinfo"))
		{
			if (wc->numprojectiles >= max_projectileinfo)
			{
				G_Error("more than %d projectiles defined in %s\n", max_projectileinfo, path);
				//FreeMemory(wc);
				trap_PC_FreeSource(source);
				return NULL;
			}
			memset(&wc->projectileinfo[wc->numprojectiles], 0, sizeof(projectileinfo_t));
			if (!trap_PC_ReadStructure(source, &projectileinfo_struct, (char*)&wc->projectileinfo[wc->numprojectiles]))
			{
				//FreeMemory(wc);
				trap_PC_FreeSource(source);
				return NULL;
			}
			wc->numprojectiles++;
		} 
		else
		{
			G_Error("unknown definition %s in %s\n", token.string, path);
			//FreeMemory(wc);
			trap_PC_FreeSource(source);
			return NULL;
		}
	}

	trap_PC_FreeSource(source);
	
	//fix up weapons
	for (i = 0; i < wc->numweapons; i++)
	{
		if (!wc->weaponinfo[i].valid) 
			continue;
		
		if (!wc->weaponinfo[i].name[0])
		{
			G_Error("weapon %d has no name in %s\n", i, path);
			//FreeMemory(wc);
			return NULL;
		} 
		
		if (!wc->weaponinfo[i].projectile[0])
		{
			G_Error("weapon %s has no projectile in %s\n", wc->weaponinfo[i].name, path);
			//FreeMemory(wc);
			return NULL;
		} 

		//find the projectile info and copy it to the weapon info
		for (j = 0; j < wc->numprojectiles; j++)
		{
			if (!strcmp(wc->projectileinfo[j].name, wc->weaponinfo[i].projectile))
			{
				memcpy(&wc->weaponinfo[i].proj, &wc->projectileinfo[j], sizeof(projectileinfo_t));
				break;
			}
		} 
		if (j == wc->numprojectiles)
		{
			G_Error("weapon %s uses undefined projectile in %s\n", wc->weaponinfo[i].name, path);
			//FreeMemory(wc);
			return NULL;
		}
	}
	if (!wc->numweapons) 
		G_Printf("no weapon info loaded\n");

	G_Printf("loaded %s\n", path);
	return wc;
} 

/*
=====================
WeaponWeightIndex
=====================
*/
int* WeaponWeightIndex(weightconfig_t* wwc, weaponconfig_t* wc)
{
	int* index, i;

	//initialize item weight index
	index = (int*)G_AllocClearedMemory(sizeof(int) * wc->numweapons);

	for (i = 0; i < wc->numweapons; i++)
	{
		index[i] = FindFuzzyWeight(wwc, wc->weaponinfo[i].name);
	}
	return index;
}

/*
=====================
WeaponWeightIndex
=====================
*/
void BotFreeWeaponWeights(int weaponstate)
{
	bot_weaponstate_t* ws;

	ws = BotWeaponStateFromHandle(weaponstate);
	if (!ws) 
		return;
	if (ws->weaponweightconfig) 
		FreeWeightConfig(ws->weaponweightconfig);
//	if (ws->weaponweightindex) FreeMemory(ws->weaponweightindex);
} 

/*
=====================
BotLoadWeaponWeights
=====================
*/
int BotLoadWeaponWeights(int weaponstate, char* filename)
{
	bot_weaponstate_t* ws;

	ws = BotWeaponStateFromHandle(weaponstate);
	if (!ws) 
		return BLERR_CANNOTLOADWEAPONWEIGHTS;
	BotFreeWeaponWeights(weaponstate);
	
	ws->weaponweightconfig = ReadWeightConfig(filename);
	if (!ws->weaponweightconfig)
	{
		G_Error("couldn't load weapon config %s\n", filename);
		return BLERR_CANNOTLOADWEAPONWEIGHTS;
	}
	if (!weaponconfig) 
		return BLERR_CANNOTLOADWEAPONCONFIG;
	ws->weaponweightindex = WeaponWeightIndex(ws->weaponweightconfig, weaponconfig);
	return BLERR_NOERROR;
} 

/*
=====================
BotGetWeaponInfo
=====================
*/
void BotGetWeaponInfo(int weaponstate, int weapon, weaponinfo_t* weaponinfo)
{
	bot_weaponstate_t* ws;

	if (!BotValidWeaponNumber(weapon)) 
		return;
	ws = BotWeaponStateFromHandle(weaponstate);
	if (!ws) 
		return;
	if (!weaponconfig) 
		return;
	memcpy(weaponinfo, &weaponconfig->weaponinfo[weapon], sizeof(weaponinfo_t));
}

/*
=====================
BotChooseBestFightWeapon
=====================
*/
int BotChooseBestFightWeapon(int weaponstate, int* inventory)
{
	int i, index, bestweapon;
	float weight, bestweight;
	weaponconfig_t* wc;
	bot_weaponstate_t* ws;

	ws = BotWeaponStateFromHandle(weaponstate);
	if (!ws) 
		return 0;
	
	wc = weaponconfig;
	if (!weaponconfig) 
		return 0;

	//if the bot has no weapon weight configuration
	if (!ws->weaponweightconfig) 
		return 0;

	bestweight = 0;
	bestweapon = 0;
	for (i = 0; i < wc->numweapons; i++)
	{
		if (!wc->weaponinfo[i].valid) 
			continue;
		index = ws->weaponweightindex[i];
		if (index < 0) 
			continue;
		weight = FuzzyWeight(inventory, ws->weaponweightconfig, index);
		if (weight > bestweight)
		{
			bestweight = weight;
			bestweapon = i;
		}
	}
	return bestweapon;
}

/*
=====================
BotResetWeaponState
=====================
*/
void BotResetWeaponState(int weaponstate)
{
	struct weightconfig_s* weaponweightconfig;
	int* weaponweightindex;
	bot_weaponstate_t* ws;

	ws = BotWeaponStateFromHandle(weaponstate);
	if (!ws) 
		return;
	weaponweightconfig = ws->weaponweightconfig;
	weaponweightindex = ws->weaponweightindex;

	//Com_Memset(ws, 0, sizeof(bot_weaponstate_t));
	ws->weaponweightconfig = weaponweightconfig;
	ws->weaponweightindex = weaponweightindex;
}

/*
=====================
BotAllocWeaponState
=====================
*/
int BotAllocWeaponState(void)
{
	int i;

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (!botweaponstates[i])
		{
			botweaponstates[i] = G_AllocClearedMemory(sizeof(bot_weaponstate_t));
			return i;
		}
	} 
	return 0;
}

/*
=====================
BotFreeWeaponState
=====================
*/
void BotFreeWeaponState(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
// jmarshall - looks like this is a copy and paste error.
		//G_Error("move state handle %d out of range\n", handle);
		G_Error("weapon state handle %d out of range\n", handle);
// jmarshall end
		return;
	}

	if (!botweaponstates[handle])
	{
// jmarshall - looks like this is a copy and paste error.
		//G_Error("invalid move state %d\n", handle);
		G_Error("invalid weapon state %d\n", handle);
// jmarshall end
		return;
	}

	BotFreeWeaponWeights(handle);
	//FreeMemory(botweaponstates[handle]);
	botweaponstates[handle] = NULL;
}

/*
=====================
BotSetupWeaponAI
=====================
*/
int BotSetupWeaponAI(void)
{
	char* file;

	file = LibVarString("weaponconfig", "weapons.c");
	weaponconfig = LoadWeaponConfig(file);
	if (!weaponconfig)
	{
		G_Error("couldn't load the weapon config\n");
		return BLERR_CANNOTLOADWEAPONCONFIG;
	}

	return BLERR_NOERROR;
}

/*
=====================
BotShutdownWeaponAI
=====================
*/
void BotShutdownWeaponAI(void)
{
	int i;

//	if (weaponconfig) 
//		FreeMemory(weaponconfig);
	weaponconfig = NULL;

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (botweaponstates[i])
		{
			BotFreeWeaponState(i);
		} 
	} 
}