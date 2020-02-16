// bot_goal.c
//

#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

//#define DEBUG_AI_GOAL
#ifdef RANDOMIZE
#define UNDECIDEDFUZZY
#endif //RANDOMIZE
#define DROPPEDWEIGHT
//minimum avoid goal time
#define AVOID_MINIMUM_TIME		10
//default avoid goal time
#define AVOID_DEFAULT_TIME		30
//avoid dropped goal time
#define AVOID_DROPPED_TIME		10
//
#define TRAVELTIME_SCALE		0.01
//item flags
#define IFL_NOTFREE				1		//not in free for all
#define IFL_NOTTEAM				2		//not in team play
#define IFL_NOTSINGLE			4		//not in single player
#define IFL_NOTBOT				8		//bot should never go for this
#define IFL_ROAM				16		//bot roam goal

//location in the map "target_location"
typedef struct maplocation_s
{
	vec3_t origin;
	int areanum;
	char name[MAX_EPAIRKEY];
	struct maplocation_s* next;
} maplocation_t;

//camp spots "info_camp"
typedef struct campspot_s
{
	vec3_t origin;
	int areanum;
	char name[MAX_EPAIRKEY];
	float range;
	float weight;
	float wait;
	float random;
	struct campspot_s* next;
} campspot_t;

typedef struct levelitem_s
{
	int number;							//number of the level item
	int iteminfo;						//index into the item info
	int flags;							//item flags
	float weight;						//fixed roam weight
	vec3_t origin;						//origin of the item
	int goalareanum;					//area the item is in
	vec3_t goalorigin;					//goal origin within the area
	int entitynum;						//entity number
	float timeout;						//item is removed after this time
	struct levelitem_s* prev, * next;
} levelitem_t;

typedef struct iteminfo_s
{
	char classname[32];					//classname of the item
	char name[MAX_STRINGFIELD];			//name of the item
	char model[MAX_STRINGFIELD];		//model of the item
	int modelindex;						//model index
	int type;							//item type
	int index;							//index in the inventory
	float respawntime;					//respawn time
	vec3_t mins;						//mins of the item
	vec3_t maxs;						//maxs of the item
	int number;							//number of the item info
} iteminfo_t;

#define ITEMINFO_OFS(x)	(int)&(((iteminfo_t *)0)->x)

fielddef_t iteminfo_fields[] =
{
{"name", ITEMINFO_OFS(name), FT_STRING},
{"model", ITEMINFO_OFS(model), FT_STRING},
{"modelindex", ITEMINFO_OFS(modelindex), FT_INT},
{"type", ITEMINFO_OFS(type), FT_INT},
{"index", ITEMINFO_OFS(index), FT_INT},
{"respawntime", ITEMINFO_OFS(respawntime), FT_FLOAT},
{"mins", ITEMINFO_OFS(mins), FT_FLOAT | FT_ARRAY, 3},
{"maxs", ITEMINFO_OFS(maxs), FT_FLOAT | FT_ARRAY, 3},
{0, 0, 0}
};

structdef_t iteminfo_struct =
{
	sizeof(iteminfo_t), iteminfo_fields
};

typedef struct itemconfig_s
{
	int numiteminfo;
	iteminfo_t* iteminfo;
} itemconfig_t;

//goal state
typedef struct bot_goalstate_s
{
	struct weightconfig_s* itemweightconfig;	//weight config
	int* itemweightindex;						//index from item to weight
	//
	int client;									//client using this goal state
	int lastreachabilityarea;					//last area with reachabilities the bot was in
	//
	bot_goal_t goalstack[MAX_GOALSTACK];		//goal stack
	int goalstacktop;							//the top of the goal stack
	//
	int avoidgoals[MAX_AVOIDGOALS];				//goals to avoid
	float avoidgoaltimes[MAX_AVOIDGOALS];		//times to avoid the goals
} bot_goalstate_t;

bot_goalstate_t* botgoalstates[MAX_CLIENTS + 1]; // bk001206 - FIXME: init?
//item configuration
itemconfig_t* itemconfig = NULL; // bk001206 - init
//level items
levelitem_t* levelitemheap = NULL; // bk001206 - init
levelitem_t* freelevelitems = NULL; // bk001206 - init
levelitem_t* levelitems = NULL; // bk001206 - init
int numlevelitems = 0;
//map locations
maplocation_t* maplocations = NULL; // bk001206 - init
//camp spots
campspot_t* campspots = NULL; // bk001206 - init
//the game type

//additional dropped item weight
libvar_t* droppedweight = NULL; // bk001206 - init

/*
==================
BotGoalStateFromHandle
==================
*/
bot_goalstate_t* BotGoalStateFromHandle(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
		G_Error( "goal state handle %d out of range\n", handle);
		return NULL;
	} 
	
	if (!botgoalstates[handle])
	{
		G_Error( "invalid goal state %d\n", handle);
		return NULL;
	} 
	
	return botgoalstates[handle];
} 

/*
==================
BotInterbreedGoalFuzzyLogic
==================
*/
void BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child)
{
	bot_goalstate_t* p1, * p2, * c;

	p1 = BotGoalStateFromHandle(parent1);
	p2 = BotGoalStateFromHandle(parent2);
	c = BotGoalStateFromHandle(child);

	InterbreedWeightConfigs(p1->itemweightconfig, p2->itemweightconfig, c->itemweightconfig);
} 

/*
========================
BotSaveGoalFuzzyLogic
========================
*/
void BotSaveGoalFuzzyLogic(int goalstate, char* filename)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);

	//WriteWeightConfig(filename, gs->itemweightconfig);
} 

/*
========================
BotMutateGoalFuzzyLogic
========================
*/
void BotMutateGoalFuzzyLogic(int goalstate, float range)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);

	EvolveWeightConfig(gs->itemweightconfig);
} 

/*
========================
LoadItemConfig
========================
*/
itemconfig_t* LoadItemConfig(char* filename)
{
	int max_iteminfo;
	pc_token_t token;
	char path[MAX_QPATH];
	int source;
	itemconfig_t* ic;
	iteminfo_t* ii;

	max_iteminfo = (int)LibVarValue("max_iteminfo", "256");
	if (max_iteminfo < 0)
	{
		G_Error( "max_iteminfo = %d\n", max_iteminfo);
		max_iteminfo = 256;
		LibVarSet("max_iteminfo", "256");
	}

	strncpy(path, filename, MAX_QPATH);
	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	source = trap_PC_LoadSource(path);
	if (!source) {
		G_Error( "counldn't load %s\n", path);
		return NULL;
	}

	//initialize item config
	ic = (itemconfig_t*)G_Alloc(sizeof(itemconfig_t) + max_iteminfo * sizeof(iteminfo_t));
	ic->iteminfo = (iteminfo_t*)((char*)ic + sizeof(itemconfig_t));
	ic->numiteminfo = 0;
	
	//parse the item config file
	while (trap_PC_ReadToken(source, &token))
	{
		if (!strcmp(token.string, "iteminfo"))
		{
			if (ic->numiteminfo >= max_iteminfo)
			{
				G_Error("more than %d item info defined\n", max_iteminfo);
				//FreeMemory(ic);
				trap_PC_FreeSource(source);
				return NULL;
			} //end if
			ii = &ic->iteminfo[ic->numiteminfo];
			memset(ii, 0, sizeof(iteminfo_t));
			if (!trap_PC_ExpectTokenType(source, TT_STRING, 0, &token))
			{
				//FreeMemory(ic);
				//FreeMemory(source);
				return NULL;
			} 
			StripDoubleQuotes(token.string);
			strncpy(ii->classname, token.string, sizeof(ii->classname) - 1);
			if (!trap_PC_ReadStructure(source, &iteminfo_struct, (char*)ii))
			{
				//FreeMemory(ic);
				//FreeSource(source);
				return NULL;
			} 
			ii->number = ic->numiteminfo;
			ic->numiteminfo++;
		} 
		else
		{
			G_Error("unknown definition %s\n", token.string);
			//FreeMemory(ic);
			//FreeSource(source);
			return NULL;
		} 
	} 
	trap_PC_FreeSource(source);
	
	if (!ic->numiteminfo) {
		G_Printf("no item info loaded\n");
	}

	G_Printf( "loaded %s\n", path);
	return ic;
} 

/*
========================
LoadItemConfig
========================
*/
int* ItemWeightIndex(weightconfig_t* iwc, itemconfig_t* ic)
{
	int* index, i;

	//initialize item weight index
	index = (int*)G_AllocClearedMemory(sizeof(int) * ic->numiteminfo);

	for (i = 0; i < ic->numiteminfo; i++)
	{
		index[i] = FindFuzzyWeight(iwc, ic->iteminfo[i].classname);
		if (index[i] < 0)
		{
		//	Log_Write("item info %d \"%s\" has no fuzzy weight\r\n", i, ic->iteminfo[i].classname);
		} 
	} 
	return index;
} 

/*
========================
InitLevelItemHeap
========================
*/
void InitLevelItemHeap(void)
{
	int i, max_levelitems;

// jmarshall - fix me
	//if (levelitemheap) 
	//	FreeMemory(levelitemheap);
// jmarshall end

	max_levelitems = (int)LibVarValue("max_levelitems", "256");
	levelitemheap = (levelitem_t*)G_AllocClearedMemory(max_levelitems * sizeof(levelitem_t));

	for (i = 0; i < max_levelitems - 1; i++)
	{
		levelitemheap[i].next = &levelitemheap[i + 1];
	}
	
	levelitemheap[max_levelitems - 1].next = NULL;
	
	freelevelitems = levelitemheap;
}

/*
========================
AllocLevelItem
========================
*/
levelitem_t* AllocLevelItem(void)
{
	levelitem_t* li;

	li = freelevelitems;
	if (!li)
	{
		G_Error( "out of level items\n");
		return NULL;
	}

	freelevelitems = freelevelitems->next;
	memset(li, 0, sizeof(levelitem_t));
	return li;
}

/*
========================
FreeLevelItem
========================
*/
void FreeLevelItem(levelitem_t* li)
{
	li->next = freelevelitems;
	freelevelitems = li;
}

/*
========================
AddLevelItemToList
========================
*/
void AddLevelItemToList(levelitem_t* li)
{
	if (levelitems) levelitems->prev = li;
	li->prev = NULL;
	li->next = levelitems;
	levelitems = li;
}

/*
========================
RemoveLevelItemFromList
========================
*/
void RemoveLevelItemFromList(levelitem_t* li)
{
	if (li->prev) 
		li->prev->next = li->next;
	else 
		levelitems = li->next;
	if (li->next) 
		li->next->prev = li->prev;
} 

/*
========================
BotFreeInfoEntities
========================
*/
void BotFreeInfoEntities(void)
{
	maplocation_t* ml, * nextml;
	campspot_t* cs, * nextcs;

	for (ml = maplocations; ml; ml = nextml)
	{
		nextml = ml->next;
// jmarshall
		//FreeMemory(ml);
// jmarshall end
	} 
	maplocations = NULL;
	for (cs = campspots; cs; cs = nextcs)
	{
		nextcs = cs->next;
// jmarshall
		//FreeMemory(cs);
// jmarshall end
	}
	campspots = NULL;
}

/*
========================
BotInitInfoEntities
========================
*/
void BotInitInfoEntities(void)
{
	char classname[MAX_EPAIRKEY];
	maplocation_t* ml;
	campspot_t* cs;
	int numlocations, numcampspots;

	BotFreeInfoEntities();
	
	numlocations = 0;
	numcampspots = 0;
	
	for(int i = 0; i < level.num_entities; i++)
	{
		gentity_t* ent = &g_entities[i];

		if (!ent->inuse)
			continue;

		if (ent->classname == NULL || ent->classname[0] == 0)
			continue;

		G_Printf(va("%s\n", classname));

		strcpy(classname, ent->classname);		

		//map locations
		if (!strcmp(classname, "target_location"))
		{
			ml = (maplocation_t*)G_AllocClearedMemory(sizeof(maplocation_t));
			//AAS_VectorForBSPEpairKey(ent, "origin", ml->origin);
			VectorCopy(ent->r.currentOrigin, ml->origin);
			//AAS_ValueForBSPEpairKey(ent, "message", ml->name, sizeof(ml->name));
			if (ent->message)
			{
				strcpy(ml->name, ent->message);
			}
			ml->areanum = 0; // AAS_PointAreaNum(ml->origin);
			ml->next = maplocations;
			maplocations = ml;
			numlocations++;
		}
		//camp spots
		else if (!strcmp(classname, "info_camp"))
		{
			cs = (campspot_t*)G_AllocClearedMemory(sizeof(campspot_t));
			//AAS_VectorForBSPEpairKey(ent, "origin", cs->origin);
			VectorCopy(ent->r.currentOrigin, cs->origin);
			//cs->origin[2] += 16;
			//AAS_ValueForBSPEpairKey(ent, "message", cs->name, sizeof(cs->name));
			if (ent->message)
			{
				strcpy(cs->name, ent->message);
			}

			//AAS_FloatForBSPEpairKey(ent, "range", &cs->range);
			cs->range = ent->range;

			//AAS_FloatForBSPEpairKey(ent, "weight", &cs->weight);
			cs->weight = ent->weight;

			//AAS_FloatForBSPEpairKey(ent, "wait", &cs->wait);
			cs->wait = ent->wait;

			//AAS_FloatForBSPEpairKey(ent, "random", &cs->random);
			cs->random = ent->random;

			//cs->areanum = AAS_PointAreaNum(cs->origin);
			//if (!cs->areanum)
			//{
			//	G_Printf( "camp spot at %1.1f %1.1f %1.1f in solid\n", cs->origin[0], cs->origin[1], cs->origin[2]);
			//	FreeMemory(cs);
			//	continue;
			//} //end if

			cs->next = campspots;
			campspots = cs;
			//AAS_DrawPermanentCross(cs->origin, 4, LINECOLOR_YELLOW);
			numcampspots++;
		} 
	} 
	G_Printf( "%d map locations\n", numlocations);
	G_Printf( "%d camp spots\n", numcampspots);	
}

/*
=======================
BotInitLevelItems
=======================
*/
void BotInitLevelItems(void)
{
	int i, spawnflags, value;
	char classname[MAX_EPAIRKEY];
	vec3_t origin, end;
	int ent, goalareanum;
	itemconfig_t* ic;
	levelitem_t* li;
	//bsp_trace_t trace;

	//initialize the map locations and camp spots
	BotInitInfoEntities();

	//initialize the level item heap
	InitLevelItemHeap();
	levelitems = NULL;
	numlevelitems = 0;

	ic = itemconfig;
	if (!ic) 
		return;

	//update the modelindexes of the item info
	//for (i = 0; i < ic->numiteminfo; i++)
	//{
	//	if (!ic->iteminfo[i].modelindex)
	//	{
	//		Log_Write("item %s has modelindex 0", ic->iteminfo[i].classname);
	//	} 
	//}

	//for (ent = AAS_NextBSPEntity(0); ent; ent = AAS_NextBSPEntity(ent))
	for (int idx = 0; idx < level.num_entities; idx++)
	{		
		gentity_t* ent = &g_entities[idx];

		if (ent->classname == NULL || ent->classname[0] == 0)
			continue;

		// AAS_IntForBSPEpairKey(ent, "spawnflags", &spawnflags);
		spawnflags = ent->spawnflags;
		
		
		for (i = 0; i < ic->numiteminfo; i++)
		{
			if (!strcmp(classname, ic->iteminfo[i].classname)) 
				break;
		} 
		if (i >= ic->numiteminfo)
		{
			G_Printf("entity %s unknown item\r\n", classname);
			continue;
		}

		//get the origin of the item
		//if (!AAS_VectorForBSPEpairKey(ent, "origin", origin))
		//{
		//	G_Error( "item %s without origin\n", classname);
		//	continue;
		//}
		VectorCopy(ent->r.currentOrigin, origin);

		goalareanum = 0;
// jmarshall - fix floating items
		//if it is a floating item
		//if (spawnflags & 1)
		//{
		//	//if the item is not floating in water
		//	if (!(AAS_PointContents(origin) & CONTENTS_WATER))
		//	{
		//		VectorCopy(origin, end);
		//		end[2] -= 32;
		//		trace = AAS_Trace(origin, ic->iteminfo[i].mins, ic->iteminfo[i].maxs, end, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP);
		//		//if the item not near the ground
		//		if (trace.fraction >= 1)
		//		{
		//			//if the item is not reachable from a jumppad
		//			goalareanum = AAS_BestReachableFromJumpPadArea(origin, ic->iteminfo[i].mins, ic->iteminfo[i].maxs);
		//			Log_Write("item %s reachable from jumppad area %d\r\n", ic->iteminfo[i].classname, goalareanum);
		//			//G_Printf( "item %s reachable from jumppad area %d\r\n", ic->iteminfo[i].classname, goalareanum);
		//			if (!goalareanum) continue;
		//		} //end if
		//	} //end if
		//} //end if
// jmarshall end

		li = AllocLevelItem();
		if (!li) 
			return;
		
		li->number = ++numlevelitems;
		li->timeout = 0;
		li->entitynum = 0;
		
		li->flags = 0;
		if (ent->notfree) 
			li->flags |= IFL_NOTFREE;
		if (ent->notteam) 
			li->flags |= IFL_NOTTEAM;
		if (ent->notsingle) 
			li->flags |= IFL_NOTSINGLE;
		if (ent->notbot) 
			li->flags |= IFL_NOTBOT;

		if (!strcmp(classname, "item_botroam"))
		{
			li->flags |= IFL_ROAM;
			//AAS_FloatForBSPEpairKey(ent, "weight", &li->weight);
			li->weight = ent->weight;
		}

		////if not a stationary item
		//if (!(spawnflags & 1))
		//{
		//	if (!AAS_DropToFloor(origin, ic->iteminfo[i].mins, ic->iteminfo[i].maxs))
		//	{
		//		G_Printf( "%s in solid at (%1.1f %1.1f %1.1f)\n",
		//			classname, origin[0], origin[1], origin[2]);
		//	} //end if
		//} //end if

		//item info of the level item
		li->iteminfo = i;
		
		//origin of the item
		VectorCopy(origin, li->origin);
				
		AddLevelItemToList(li);
	}
	G_Printf( "found %d level items\n", numlevelitems);
}

/*
=======================
BotGoalName
=======================
*/
void BotGoalName(int number, char* name, int size)
{
	levelitem_t* li;

	if (!itemconfig) 
		return;

	for (li = levelitems; li; li = li->next)
	{
		if (li->number == number)
		{
			strncpy(name, itemconfig->iteminfo[li->iteminfo].name, size - 1);
			name[size - 1] = '\0';
			return;
		}
	}
	strcpy(name, "");
	return;
}

/*
=======================
BotResetAvoidGoals
=======================
*/
void BotResetAvoidGoals(int goalstate)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	memset(gs->avoidgoals, 0, MAX_AVOIDGOALS * sizeof(int));
	memset(gs->avoidgoaltimes, 0, MAX_AVOIDGOALS * sizeof(float));
} 

/*
=======================
BotResetAvoidGoals
=======================
*/
void BotDumpAvoidGoals(int goalstate)
{
	int i;
	bot_goalstate_t* gs;
	char name[32];

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) return;
	for (i = 0; i < MAX_AVOIDGOALS; i++)
	{
		if (gs->avoidgoaltimes[i] >= AAS_Time())
		{
			BotGoalName(gs->avoidgoals[i], name, 32);
			//Log_Write("avoid goal %s, number %d for %f seconds", name,
			//	gs->avoidgoals[i], gs->avoidgoaltimes[i] - AAS_Time());
		} 
	}
}

/*
=======================
BotAddToAvoidGoals
=======================
*/
void BotAddToAvoidGoals(bot_goalstate_t* gs, int number, float avoidtime)
{
	int i;

	for (i = 0; i < MAX_AVOIDGOALS; i++)
	{
		//if the avoid goal is already stored
		if (gs->avoidgoals[i] == number)
		{
			gs->avoidgoals[i] = number;
			gs->avoidgoaltimes[i] = AAS_Time() + avoidtime;
			return;
		}
	}

	for (i = 0; i < MAX_AVOIDGOALS; i++)
	{
		//if this avoid goal has expired
		if (gs->avoidgoaltimes[i] < AAS_Time())
		{
			gs->avoidgoals[i] = number;
			gs->avoidgoaltimes[i] = AAS_Time() + avoidtime;
			return;
		}
	}
}

/*
=======================
BotAddToAvoidGoals
=======================
*/
void BotRemoveFromAvoidGoals(int goalstate, int number)
{
	int i;
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	
	//don't use the goals the bot wants to avoid
	for (i = 0; i < MAX_AVOIDGOALS; i++)
	{
		if (gs->avoidgoals[i] == number && gs->avoidgoaltimes[i] >= AAS_Time())
		{
			gs->avoidgoaltimes[i] = 0;
			return;
		}
	}
}

/*
=======================
BotAvoidGoalTime
=======================
*/
float BotAvoidGoalTime(int goalstate, int number)
{
	int i;
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return 0;
	
	//don't use the goals the bot wants to avoid
	for (i = 0; i < MAX_AVOIDGOALS; i++)
	{
		if (gs->avoidgoals[i] == number && gs->avoidgoaltimes[i] >= AAS_Time())
		{
			return gs->avoidgoaltimes[i] - AAS_Time();
		}
	}
	return 0;
} 

/*
=======================
BotSetAvoidGoalTime
=======================
*/
void BotSetAvoidGoalTime(int goalstate, int number, float avoidtime)
{
	bot_goalstate_t* gs;
	levelitem_t* li;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs)
		return;
	if (avoidtime < 0)
	{
		if (!itemconfig)
			return;
		
		for (li = levelitems; li; li = li->next)
		{
			if (li->number == number)
			{
				avoidtime = itemconfig->iteminfo[li->iteminfo].respawntime;
				if (!avoidtime)
					avoidtime = AVOID_DEFAULT_TIME;
				if (avoidtime < AVOID_MINIMUM_TIME)
					avoidtime = AVOID_MINIMUM_TIME;
				BotAddToAvoidGoals(gs, number, avoidtime);
				return;
			}
		}
		return;
	}
	else
	{
		BotAddToAvoidGoals(gs, number, avoidtime);
	}
}

/*
=======================
BotGetLevelItemGoal
=======================
*/
int BotGetLevelItemGoal(int index, char* name, bot_goal_t* goal)
{
	levelitem_t* li;

	if (!itemconfig) 
		return -1;
	li = levelitems;
	if (index >= 0)
	{
		for (; li; li = li->next)
		{
			if (li->number == index)
			{
				li = li->next;
				break;
			}
		}
	}

	for (; li; li = li->next)
	{
		if (g_gametype.integer == GT_SINGLE_PLAYER) {
			if (li->flags & IFL_NOTSINGLE) 
				continue;
		}
		else if (g_gametype.integer >= GT_TEAM) {
			if (li->flags & IFL_NOTTEAM) 
				continue;
		}
		else {
			if (li->flags & IFL_NOTFREE) 
				continue;
		}
		if (li->flags & IFL_NOTBOT) 
			continue;

		if (!Q_stricmp(name, itemconfig->iteminfo[li->iteminfo].name))
		{
			goal->areanum = li->goalareanum;
			VectorCopy(li->goalorigin, goal->origin);
			goal->entitynum = li->entitynum;
			VectorCopy(itemconfig->iteminfo[li->iteminfo].mins, goal->mins);
			VectorCopy(itemconfig->iteminfo[li->iteminfo].maxs, goal->maxs);
			goal->number = li->number;
			goal->flags = GFL_ITEM;
			if (li->timeout) goal->flags |= GFL_DROPPED;
			//G_Printf( "found li %s\n", itemconfig->iteminfo[li->iteminfo].name);
			return li->number;
		}
	}
	return -1;
}

/*
=========================
BotGetMapLocationGoal
=========================
*/
int BotGetMapLocationGoal(char* name, bot_goal_t* goal)
{
	maplocation_t* ml;
	vec3_t mins = { -8, -8, -8 }, maxs = { 8, 8, 8 };

	for (ml = maplocations; ml; ml = ml->next)
	{
		if (!Q_stricmp(ml->name, name))
		{
			goal->areanum = ml->areanum;
			VectorCopy(ml->origin, goal->origin);
			goal->entitynum = 0;
			VectorCopy(mins, goal->mins);
			VectorCopy(maxs, goal->maxs);
			return qtrue;
		}
	}
	return qfalse;
}

/*
=========================
BotGetNextCampSpotGoal
=========================
*/
int BotGetNextCampSpotGoal(int num, bot_goal_t* goal)
{
	int i;
	campspot_t* cs;
	vec3_t mins = { -8, -8, -8 }, maxs = { 8, 8, 8 };

	if (num < 0) num = 0;
	i = num;
	for (cs = campspots; cs; cs = cs->next)
	{
		if (--i < 0)
		{
			goal->areanum = cs->areanum;
			VectorCopy(cs->origin, goal->origin);
			goal->entitynum = 0;
			VectorCopy(mins, goal->mins);
			VectorCopy(maxs, goal->maxs);
			return num + 1;
		}
	}
	return 0;
}

/*
============================
BotFindEntityForLevelItem
============================
*/
void BotFindEntityForLevelItem(levelitem_t* li)
{
	int ent, modelindex;
	itemconfig_t* ic;
	//aas_entityinfo_t entinfo;
	vec3_t dir;

	ic = itemconfig;
	if (!itemconfig) return;
	for (int idx = 0; idx < level.num_entities; idx++)
	{
		gentity_t* ent = &g_entities[idx];

		//get the model index of the entity
		modelindex = ent->s.modelindex;
		
		if (!modelindex) 
			continue;

// jmarshall - eval moving objects
		//get info about the entity
		//AAS_EntityInfo(ent, &entinfo);
		//if the entity is still moving
		//if (entinfo.origin[0] != entinfo.lastvisorigin[0] ||
		//	entinfo.origin[1] != entinfo.lastvisorigin[1] ||
		//	entinfo.origin[2] != entinfo.lastvisorigin[2]) continue;
		//
// jmarshall end
		if (ic->iteminfo[li->iteminfo].modelindex == modelindex)
		{
			//check if the entity is very close
			VectorSubtract(li->origin, ent->r.currentOrigin, dir);
			if (VectorLength(dir) < 30)
			{
				//found an entity for this level item
				li->entitynum = ent;
			}
		}
	}
}

/*
========================
BotUpdateEntityItems
========================
*/

//NOTE: enum entityType_t in bg_public.h
#define ET_ITEM			2

void BotUpdateEntityItems(void)
{
	int i, modelindex;
	vec3_t dir;
	levelitem_t* li, * nextli;
	itemconfig_t* ic;

	//timeout current entity items if necessary
	for (li = levelitems; li; li = nextli)
	{
		nextli = li->next;
		//if it is a item that will time out
		if (li->timeout)
		{
			//timeout the item
			if (li->timeout < AAS_Time())
			{
				RemoveLevelItemFromList(li);
				FreeLevelItem(li);
			}
		}
	}

	//find new entity items
	ic = itemconfig;
	if (!itemconfig) return;
	//
	for (int idx = 0; idx < level.num_entities; idx++)
	{
		gentity_t* ent = &g_entities[idx];

		if (ent->s.eType != ET_ITEM) 
			continue;

		//get the model index of the entity
		modelindex = ent->s.modelindex;
		
		if (!modelindex) 
			continue;
// jmarshall - eval floating items.
		////get info about the entity
		//AAS_EntityInfo(ent, &entinfo);
		////FIXME: don't do this
		////skip all floating items for now
		////if (entinfo.groundent != ENTITYNUM_WORLD) continue;
		////if the entity is still moving
		//if (entinfo.origin[0] != entinfo.lastvisorigin[0] ||
		//	entinfo.origin[1] != entinfo.lastvisorigin[1] ||
		//	entinfo.origin[2] != entinfo.lastvisorigin[2]) continue;
// jmarshall end

		//check if the entity is already stored as a level item
		for (li = levelitems; li; li = li->next)
		{
			//if the level item is linked to an entity
			if (li->entitynum && li->entitynum == ent)
			{
				//the entity is re-used if the models are different
				if (ic->iteminfo[li->iteminfo].modelindex != modelindex)
				{
					//remove this level item
					RemoveLevelItemFromList(li);
					FreeLevelItem(li);
					li = NULL;
					break;
				}
				else
				{
					//if (entinfo.origin[0] != li->origin[0] ||
					//	entinfo.origin[1] != li->origin[1] ||
					//	entinfo.origin[2] != li->origin[2])
					//{
					//	VectorCopy(entinfo.origin, li->origin);
					//	//also update the goal area number
					//	li->goalareanum = AAS_BestReachableArea(li->origin,
					//		ic->iteminfo[li->iteminfo].mins, ic->iteminfo[li->iteminfo].maxs,
					//		li->goalorigin);
					//} //end if
					VectorCopy(ent->r.currentOrigin, li->origin);
					break;
				}
			}
		}
		if (li) 
			continue;

		//try to link the entity to a level item
		for (li = levelitems; li; li = li->next)
		{
			//if this level item is already linked
			if (li->entitynum) continue;
			//
			if (g_gametype.integer == GT_SINGLE_PLAYER) {
				if (li->flags & IFL_NOTSINGLE) 
					continue;
			}
			else if (g_gametype.integer >= GT_TEAM) {
				if (li->flags & IFL_NOTTEAM) 
					continue;
			}
			else {
				if (li->flags & IFL_NOTFREE) 
					continue;
			}

			//if the model of the level item and the entity are the same
			if (ic->iteminfo[li->iteminfo].modelindex == modelindex)
			{
				//check if the entity is very close
				VectorSubtract(li->origin, ent->r.currentOrigin, dir);
				if (VectorLength(dir) < 30)
				{
					//found an entity for this level item
					li->entitynum = ent;
					//if the origin is different
					//if (entinfo.origin[0] != li->origin[0] ||
					//	entinfo.origin[1] != li->origin[1] ||
					//	entinfo.origin[2] != li->origin[2])
					//{
					//	//update the level item origin
					//	VectorCopy(ent->r.currentOrigin, li->origin);
					//	//also update the goal area number
					//	li->goalareanum = AAS_BestReachableArea(li->origin,
					//		ic->iteminfo[li->iteminfo].mins, ic->iteminfo[li->iteminfo].maxs,
					//		li->goalorigin);
					//} //end if

					//update the level item origin
					VectorCopy(ent->r.currentOrigin, li->origin);
					break;
				}
			} 
		}

		if (li) 
			continue;
		//check if the model is from a known item
		for (i = 0; i < ic->numiteminfo; i++)
		{
			if (ic->iteminfo[i].modelindex == modelindex)
			{
				break;
			}
		}

		//if the model is not from a known item
		if (i >= ic->numiteminfo) 
			continue;
		//allocate a new level item
		li = AllocLevelItem();
		
		if (!li) 
			continue;
		
		//entity number of the level item
		li->entitynum = ent;
		
		//number for the level item
		li->number = numlevelitems + ent;
		
		//set the item info index for the level item
		li->iteminfo = i;
		
		//origin of the item
		VectorCopy(ent->r.currentOrigin, li->origin);

// jmarshall - fix this, bots, jump pads, and droppable items, bad combo.
		//get the item goal area and goal origin
		//li->goalareanum = AAS_BestReachableArea(li->origin,
		//	ic->iteminfo[i].mins, ic->iteminfo[i].maxs,
		//	li->goalorigin);
		////never go for items dropped into jumppads
		//if (AAS_AreaJumpPad(li->goalareanum))
		//{
		//	FreeLevelItem(li);
		//	continue;
		//} //end if
// jmarshall end
		//time this item out after 30 seconds
		//dropped items disappear after 30 seconds
		li->timeout = AAS_Time() + 30;
		//add the level item to the list
		AddLevelItemToList(li);
		//G_Printf( "found new level item %s\n", ic->iteminfo[i].classname);
	} 
}

/*
====================
BotDumpGoalStack
====================
*/
void BotDumpGoalStack(int goalstate)
{
	int i;
	bot_goalstate_t* gs;
	char name[32];

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	for (i = 1; i <= gs->goalstacktop; i++)
	{
		BotGoalName(gs->goalstack[i].number, name, 32);
		G_Printf("%d: %s", i, name);
	}
}

/*
====================
BotPushGoal
====================
*/
void BotPushGoal(int goalstate, bot_goal_t* goal)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	if (gs->goalstacktop >= MAX_GOALSTACK - 1)
	{
		G_Error( "goal heap overflow\n");
		BotDumpGoalStack(goalstate);
		return;
	}
	gs->goalstacktop++;
	memcpy(&gs->goalstack[gs->goalstacktop], goal, sizeof(bot_goal_t));
}

/*
====================
BotPopGoal
====================
*/
void BotPopGoal(int goalstate)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	if (gs->goalstacktop > 0) 
		gs->goalstacktop--;
}

/*
====================
BotEmptyGoalStack
====================
*/
void BotEmptyGoalStack(int goalstate)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	gs->goalstacktop = 0;
}

/*
====================
BotGetTopGoal
====================
*/
int BotGetTopGoal(int goalstate, bot_goal_t* goal)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return qfalse;
	if (!gs->goalstacktop) 
		return qfalse;
	memcpy(goal, &gs->goalstack[gs->goalstacktop], sizeof(bot_goal_t));
	return qtrue;
}

/*
====================
BotGetSecondGoal
====================
*/
int BotGetSecondGoal(int goalstate, bot_goal_t* goal)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return qfalse;
	if (gs->goalstacktop <= 1) 
		return qfalse;
	memcpy(goal, &gs->goalstack[gs->goalstacktop - 1], sizeof(bot_goal_t));
	return qtrue;
}

/*
====================
BotChooseLTGItem
====================
*/
int BotChooseLTGItem(int goalstate, vec3_t origin, int* inventory, int travelflags)
{
	int t, weightnum;
	float weight, bestweight, avoidtime;
	iteminfo_t* iteminfo;
	itemconfig_t* ic;
	levelitem_t* li, * bestitem;
	bot_goal_t goal;
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs)
		return qfalse;
	if (!gs->itemweightconfig)
		return qfalse;
	//get the area the bot is in
	//areanum = BotReachabilityArea(origin, gs->client);
	//if the bot is in solid or if the area the bot is in has no reachability links
	//if (!areanum || !AAS_AreaReachability(areanum))
	//{
	//	//use the last valid area the bot was in
	//	areanum = gs->lastreachabilityarea;
	//} //end if
	//remember the last area with reachabilities the bot was in
	//gs->lastreachabilityarea = areanum;
	//if still in solid
	//if (!areanum)
	//	return qfalse;
	//the item configuration
	ic = itemconfig;
	if (!itemconfig)
		return qfalse;
	//best weight and item so far
	bestweight = 0;
	bestitem = NULL;
	memset(&goal, 0, sizeof(bot_goal_t));
	//go through the items in the level
	for (li = levelitems; li; li = li->next)
	{
		if (g_gametype.integer == GT_SINGLE_PLAYER) {
			if (li->flags & IFL_NOTSINGLE)
				continue;
		}
		else if (g_gametype.integer >= GT_TEAM) {
			if (li->flags & IFL_NOTTEAM)
				continue;
		}
		else {
			if (li->flags & IFL_NOTFREE)
				continue;
		}
		if (li->flags & IFL_NOTBOT)
			continue;
		//if the item is not in a possible goal area
		if (!li->goalareanum)
			continue;
		//FIXME: is this a good thing? added this for items that never spawned into the game (f.i. CTF flags in obelisk)
		if (!li->entitynum && !(li->flags & IFL_ROAM))
			continue;
		//get the fuzzy weight function for this item
		iteminfo = &ic->iteminfo[li->iteminfo];
		weightnum = gs->itemweightindex[iteminfo->number];
		if (weightnum < 0)
			continue;

#ifdef UNDECIDEDFUZZY
		weight = FuzzyWeightUndecided(inventory, gs->itemweightconfig, weightnum);
#else
		weight = FuzzyWeight(inventory, gs->itemweightconfig, weightnum);
#endif //UNDECIDEDFUZZY
#ifdef DROPPEDWEIGHT
		//HACK: to make dropped items more attractive
		if (li->timeout)
			weight += GetValueFromLibVar(droppedweight);
#endif //DROPPEDWEIGHT
		//use weight scale for item_botroam
		if (li->flags & IFL_ROAM) weight *= li->weight;
		//
		if (weight > 0)
		{
			//get the travel time towards the goal area
// jmarshall
			//t = AAS_AreaTravelTimeToGoalArea(areanum, origin, li->goalareanum, travelflags);
			t = BotTravelTime(origin, li->goalorigin);
// jmarshall end

			//if the goal is reachable
			if (t > 0)
			{
				//if this item won't respawn before we get there
				avoidtime = BotAvoidGoalTime(goalstate, li->number);
				if (avoidtime - t * 0.009 > 0)
					continue;
				
				weight /= (float)t * TRAVELTIME_SCALE;
				
				if (weight > bestweight)
				{
					bestweight = weight;
					bestitem = li;
				}
			}
		}
	}

	//if no goal item found
	if (!bestitem)
	{
		/*
		//if not in lava or slime
		if (!AAS_AreaLava(areanum) && !AAS_AreaSlime(areanum))
		{
			if (AAS_RandomGoalArea(areanum, travelflags, &goal.areanum, goal.origin))
			{
				VectorSet(goal.mins, -15, -15, -15);
				VectorSet(goal.maxs, 15, 15, 15);
				goal.entitynum = 0;
				goal.number = 0;
				goal.flags = GFL_ROAM;
				goal.iteminfo = 0;
				//push the goal on the stack
				BotPushGoal(goalstate, &goal);
				//
#ifdef DEBUG
				G_Printf( "chosen roam goal area %d\n", goal.areanum);
#endif //DEBUG
				return qtrue;
			} //end if
		} //end if
		*/
		return qfalse;
	}

	//create a bot goal for this item
	iteminfo = &ic->iteminfo[bestitem->iteminfo];
	VectorCopy(bestitem->goalorigin, goal.origin);
	VectorCopy(iteminfo->mins, goal.mins);
	VectorCopy(iteminfo->maxs, goal.maxs);
	goal.areanum = bestitem->goalareanum;
	goal.entitynum = bestitem->entitynum;
	goal.number = bestitem->number;
	goal.flags = GFL_ITEM;
	if (bestitem->timeout)
		goal.flags |= GFL_DROPPED;
	if (bestitem->flags & IFL_ROAM)
		goal.flags |= GFL_ROAM;
	goal.iteminfo = bestitem->iteminfo;
	//if it's a dropped item
	if (bestitem->timeout)
	{
		avoidtime = AVOID_DROPPED_TIME;
	} 
	else
	{
		avoidtime = iteminfo->respawntime;
		if (!avoidtime)
			avoidtime = AVOID_DEFAULT_TIME;
		if (avoidtime < AVOID_MINIMUM_TIME)
			avoidtime = AVOID_MINIMUM_TIME;
	} 

	//add the chosen goal to the goals to avoid for a while
	BotAddToAvoidGoals(gs, bestitem->number, avoidtime);
	
	//push the goal on the stack
	BotPushGoal(goalstate, &goal);
	
	return qtrue;
}

/*
====================
BotChooseNBGItem
====================
*/
int BotChooseNBGItem(int goalstate, vec3_t origin, int* inventory, int travelflags, bot_goal_t* ltg, float maxtime)
{
	int areanum, t, weightnum, ltg_time;
	float weight, bestweight, avoidtime;
	iteminfo_t* iteminfo;
	itemconfig_t* ic;
	levelitem_t* li, * bestitem;
	bot_goal_t goal;
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs)
		return qfalse;
	if (!gs->itemweightconfig)
		return qfalse;
	//get the area the bot is in
// jmarshall
	areanum = 1; // BotReachabilityArea(origin, gs->client);
// jmarshall end
	//if the bot is in solid or if the area the bot is in has no reachability links
	//if (!areanum || !AAS_AreaReachability(areanum))
	//{
	//	//use the last valid area the bot was in
	//	areanum = gs->lastreachabilityarea;
	//} //end if
	//remember the last area with reachabilities the bot was in
	gs->lastreachabilityarea = areanum;
	//if still in solid
	if (!areanum)
		return qfalse;
// jmarshall
	if (ltg) {
		//ltg_time = AAS_AreaTravelTimeToGoalArea(areanum, origin, ltg->areanum, travelflags);
		ltg_time = BotTravelTime(origin, ltg->origin);
	}
	else {
		ltg_time = 99999;
	}
// jmarshall end
	//the item configuration
	ic = itemconfig;
	if (!itemconfig)
		return qfalse;
	//best weight and item so far
	bestweight = 0;
	bestitem = NULL;
	memset(&goal, 0, sizeof(bot_goal_t));
	//go through the items in the level
	for (li = levelitems; li; li = li->next)
	{
		if (g_gametype.integer == GT_SINGLE_PLAYER) {
			if (li->flags & IFL_NOTSINGLE)
				continue;
		}
		else if (g_gametype.integer >= GT_TEAM) {
			if (li->flags & IFL_NOTTEAM)
				continue;
		}
		else {
			if (li->flags & IFL_NOTFREE)
				continue;
		}
		if (li->flags & IFL_NOTBOT)
			continue;
		//if the item is in a possible goal area
		if (!li->goalareanum)
			continue;
		//FIXME: is this a good thing? added this for items that never spawned into the game (f.i. CTF flags in obelisk)
		if (!li->entitynum && !(li->flags & IFL_ROAM))
			continue;
		//get the fuzzy weight function for this item
		iteminfo = &ic->iteminfo[li->iteminfo];
		weightnum = gs->itemweightindex[iteminfo->number];
		if (weightnum < 0)
			continue;
		//
#ifdef UNDECIDEDFUZZY
		weight = FuzzyWeightUndecided(inventory, gs->itemweightconfig, weightnum);
#else
		weight = FuzzyWeight(inventory, gs->itemweightconfig, weightnum);
#endif //UNDECIDEDFUZZY
#ifdef DROPPEDWEIGHT
		//HACK: to make dropped items more attractive
		if (li->timeout)
			weight += GetValueFromLibVar(droppedweight);
#endif //DROPPEDWEIGHT
		//use weight scale for item_botroam
		if (li->flags & IFL_ROAM) weight *= li->weight;
		//
		if (weight > 0)
		{
			//get the travel time towards the goal area
			//t = AAS_AreaTravelTimeToGoalArea(areanum, origin, li->goalareanum, travelflags);
			t = BotTravelTime(origin, li->goalorigin);

			//if the goal is reachable
			if (t > 0 && t < maxtime)
			{
				//if this item won't respawn before we get there
				avoidtime = BotAvoidGoalTime(goalstate, li->number);
				if (avoidtime - t * 0.009 > 0)
					continue;
				//
				weight /= (float)t * TRAVELTIME_SCALE;
				//
				if (weight > bestweight)
				{
					t = 0;
					if (ltg && !li->timeout)
					{
						//get the travel time from the goal to the long term goal
// jmarshall
						//t = AAS_AreaTravelTimeToGoalArea(li->goalareanum, li->goalorigin, ltg->areanum, travelflags);
						t = BotTravelTime(li->goalorigin, ltg->origin);
// jmarshall end
					} //end if
					//if the travel back is possible and doesn't take too long
					if (t <= ltg_time)
					{
						bestweight = weight;
						bestitem = li;
					} //end if
				} //end if
			} //end if
		} //end if
	} //end for
	//if no goal item found
	if (!bestitem)
		return qfalse;
	//create a bot goal for this item
	iteminfo = &ic->iteminfo[bestitem->iteminfo];
	VectorCopy(bestitem->goalorigin, goal.origin);
	VectorCopy(iteminfo->mins, goal.mins);
	VectorCopy(iteminfo->maxs, goal.maxs);
	goal.areanum = bestitem->goalareanum;
	goal.entitynum = bestitem->entitynum;
	goal.number = bestitem->number;
	goal.flags = GFL_ITEM;
	if (bestitem->timeout)
		goal.flags |= GFL_DROPPED;
	if (bestitem->flags & IFL_ROAM)
		goal.flags |= GFL_ROAM;
	goal.iteminfo = bestitem->iteminfo;
	//if it's a dropped item
	if (bestitem->timeout)
	{
		avoidtime = AVOID_DROPPED_TIME;
	} //end if
	else
	{
		avoidtime = iteminfo->respawntime;
		if (!avoidtime)
			avoidtime = AVOID_DEFAULT_TIME;
		if (avoidtime < AVOID_MINIMUM_TIME)
			avoidtime = AVOID_MINIMUM_TIME;
	} //end else
	//add the chosen goal to the goals to avoid for a while
	BotAddToAvoidGoals(gs, bestitem->number, avoidtime);
	//push the goal on the stack
	BotPushGoal(goalstate, &goal);
	//
	return qtrue;
} //end of the function BotChooseNBGItem


/*
====================
BotTouchingGoal
====================
*/
int BotTouchingGoal(vec3_t origin, bot_goal_t* goal)
{
	int i;
	vec3_t boxmins, boxmaxs;
	vec3_t absmins, absmaxs;
	vec3_t safety_maxs = { 0, 0, 0 }; //{4, 4, 10};
	vec3_t safety_mins = { 0, 0, 0 }; //{-4, -4, 0};

	NAV_PresenceTypeBoundingBox(PRESENCE_NORMAL, boxmins, boxmaxs);
	VectorSubtract(goal->mins, boxmaxs, absmins);
	VectorSubtract(goal->maxs, boxmins, absmaxs);
	VectorAdd(absmins, goal->origin, absmins);
	VectorAdd(absmaxs, goal->origin, absmaxs);
	
	//make the box a little smaller for safety
	VectorSubtract(absmaxs, safety_maxs, absmaxs);
	VectorSubtract(absmins, safety_mins, absmins);

	for (i = 0; i < 3; i++)
	{
		if (origin[i] < absmins[i] || origin[i] > absmaxs[i]) 
			return qfalse;
	}
	return qtrue;
}

/*
========================
BotItemGoalInVisButNotVisible
========================
*/
int BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, bot_goal_t* goal)
{
	//aas_entityinfo_t entinfo;
	trace_t trace;
	vec3_t middle;

	if (!(goal->flags & GFL_ITEM)) 
		return qfalse;

	VectorAdd(goal->mins, goal->mins, middle);
	VectorScale(middle, 0.5, middle);
	VectorAdd(goal->origin, middle, middle);

	trap_Trace(&trace, eye, NULL, NULL, middle, viewer, CONTENTS_SOLID);

	//if the goal middle point is visible
	if (trace.fraction >= 1)
	{
		//the goal entity number doesn't have to be valid
		//just assume it's valid
		if (goal->entitynum <= 0)
			return qfalse;
// jmarshall - time delay thing here?
		//
		//if the entity data isn't valid
		//AAS_EntityInfo(goal->entitynum, &entinfo);
		//NOTE: for some wacko reason entities are sometimes
		// not updated
		//if (!entinfo.valid) return qtrue;
		//if (entinfo.ltime < AAS_Time() - 0.5)
		//	return qtrue;
// jmarshall end
	} 
	return qfalse;
}

/*
========================
BotResetGoalState
========================
*/
void BotResetGoalState(int goalstate)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	memset(gs->goalstack, 0, MAX_GOALSTACK * sizeof(bot_goal_t));
	gs->goalstacktop = 0;
	BotResetAvoidGoals(goalstate);
}

/*
========================
BotResetGoalState
========================
*/
int BotLoadItemWeights(int goalstate, char* filename)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return BLERR_CANNOTLOADITEMWEIGHTS;
	
	//load the weight configuration
	gs->itemweightconfig = ReadWeightConfig(filename);
	if (!gs->itemweightconfig)
	{
		G_Error( "couldn't load weights\n");
		return BLERR_CANNOTLOADITEMWEIGHTS;
	}

	//if there's no item configuration
	if (!itemconfig) 
		return BLERR_CANNOTLOADITEMWEIGHTS;
	
	//create the item weight index
	gs->itemweightindex = ItemWeightIndex(gs->itemweightconfig, itemconfig);
	
	//everything went ok
	return BLERR_NOERROR;
}

/*
========================
BotResetGoalState
========================
*/
void BotFreeItemWeights(int goalstate)
{
	bot_goalstate_t* gs;

	gs = BotGoalStateFromHandle(goalstate);
	if (!gs) 
		return;
	if (gs->itemweightconfig) 
		FreeWeightConfig(gs->itemweightconfig);
// jmarshall - eval
	//if (gs->itemweightindex) 
	//	FreeMemory(gs->itemweightindex);
// jmarshall end
}

/*
========================
BotAllocGoalState
========================
*/
int BotAllocGoalState(int client)
{
	int i;

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (!botgoalstates[i])
		{
			botgoalstates[i] = G_AllocClearedMemory(sizeof(bot_goalstate_t));
			botgoalstates[i]->client = client;
			return i;
		}
	}
	return 0;
}

/*
========================
BotFreeGoalState
========================
*/
void BotFreeGoalState(int handle)
{
	if (handle <= 0 || handle > MAX_CLIENTS)
	{
		G_Error( "goal state handle %d out of range\n", handle);
		return;
	} 
	
	if (!botgoalstates[handle])
	{
		G_Error( "invalid goal state handle %d\n", handle);
		return;
	} 

	BotFreeItemWeights(handle);
// jmarshall
	//FreeMemory(botgoalstates[handle]);
// jmarshall end
	botgoalstates[handle] = NULL;
}

/*
========================
BotSetupGoalAI
========================
*/
int BotSetupGoalAI(void)
{
	char* filename;

	//check if teamplay is on
//	g_gametype.integer = LibVarValue("g_gametype.integer", "0");
	
	//item configuration file
	filename = LibVarString("itemconfig", "items.c");
	
	//load the item configuration
	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	itemconfig = LoadItemConfig(filename);
	if (!itemconfig)
	{
		G_Error( "couldn't load item config\n");
		return BLERR_CANNOTLOADITEMCONFIG;
	}
	
	droppedweight = LibVar("droppedweight", "1000");
	
	//everything went ok
	return BLERR_NOERROR;
}

/*
========================
BotSetupGoalAI
========================
*/
void BotShutdownGoalAI(void)
{
	int i;
// jmarshall
	//if (itemconfig) 
	//	FreeMemory(itemconfig);
// jmarshall end
	itemconfig = NULL;
// jmarshall
	//if (levelitemheap) 
	//	FreeMemory(levelitemheap);
// jmarshall end
	levelitemheap = NULL;
	freelevelitems = NULL;
	levelitems = NULL;
	numlevelitems = 0;

	BotFreeInfoEntities();

	for (i = 1; i <= MAX_CLIENTS; i++)
	{
		if (botgoalstates[i])
		{
			BotFreeGoalState(i);
		}
	}
}
