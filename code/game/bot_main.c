// bot_main.c
//

#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"
#include "chars.h"

bot_state_t *botstates[MAX_CLIENTS];
int local_time = 0;
float floattime;

/*
==================
BotIsDead
==================
*/
qboolean BotIsDead(bot_state_t* bs) {
	return (bs->cur_ps.pm_type == PM_DEAD);
}


/*
==================
BotAIInit
==================
*/
void BotAIInit(void) {
	memset(&botstates[0], 0, sizeof(botstates));
}

/*
==============
BotInterbreedEndMatch
add link back into ExitLevel?
==============
*/
void BotInterbreedEndMatch(void) {

}

/*
==================
BotAI_GetClientState
==================
*/
int BotAI_GetClientState(int clientNum, playerState_t* state) {
	gentity_t* ent;

	ent = &g_entities[clientNum];
	if (!ent->inuse) {
		return qfalse;
	}
	if (!ent->client) {
		return qfalse;
	}

	memcpy(state, &ent->client->ps, sizeof(playerState_t));
	return qtrue;
}

/*
==============
BotAI_SelectWeapon
==============
*/
void BotAI_SelectWeapon(bot_state_t *bs, int weaponNum) {
	bs->input.weapon = weaponNum;
}

/*
==============
BotAI
==============
*/
int BotAI(int client, float thinktime) {
	bot_state_t* bs;
	char buf[1024], * args;
	int j;

	//trap_EA_ResetInput(client);
	//
	bs = botstates[client];
	if (!bs || !bs->inuse) {
		//BotAI_Print(PRT_FATAL, "BotAI: client %d is not setup\n", client);
		return qfalse;
	}

	memset(&bs->input, 0, sizeof(bot_input_t));

	//retrieve the current client state
	BotAI_GetClientState(client, &bs->cur_ps);

	//retrieve any waiting server commands
//	while (trap_BotGetServerCommand(client, buf, sizeof(buf))) {
//		//have buf point to the command and args to the command arguments
//		args = strchr(buf, ' ');
//		if (!args) continue;
//		*args++ = '\0';
//
//		//remove color espace sequences from the arguments
//		RemoveColorEscapeSequences(args);
//
//		if (!Q_stricmp(buf, "cp "))
//		{ /*CenterPrintf*/
//		}
//		else if (!Q_stricmp(buf, "cs"))
//		{ /*ConfigStringModified*/
//		}
//		else if (!Q_stricmp(buf, "print")) {
//			//remove first and last quote from the chat message
//			memmove(args, args + 1, strlen(args));
//			args[strlen(args) - 1] = '\0';
//			trap_BotQueueConsoleMessage(bs->cs, CMS_NORMAL, args);
//		}
//		else if (!Q_stricmp(buf, "chat")) {
//			//remove first and last quote from the chat message
//			memmove(args, args + 1, strlen(args));
//			args[strlen(args) - 1] = '\0';
//			trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
//		}
//		else if (!Q_stricmp(buf, "tchat")) {
//			//remove first and last quote from the chat message
//			memmove(args, args + 1, strlen(args));
//			args[strlen(args) - 1] = '\0';
//			trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
//		}
//#ifdef MISSIONPACK
//		else if (!Q_stricmp(buf, "vchat")) {
//			BotVoiceChatCommand(bs, SAY_ALL, args);
//		}
//		else if (!Q_stricmp(buf, "vtchat")) {
//			BotVoiceChatCommand(bs, SAY_TEAM, args);
//		}
//		else if (!Q_stricmp(buf, "vtell")) {
//			BotVoiceChatCommand(bs, SAY_TELL, args);
//		}
//#endif
//		else if (!Q_stricmp(buf, "scores"))
//		{ /*FIXME: parse scores?*/
//		}
//		else if (!Q_stricmp(buf, "clientLevelShot"))
//		{ /*ignore*/
//		}
//	}

	//add the delta angles to the bot's current view angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}

//	//increase the local time of the bot
	bs->ltime += thinktime;

	bs->thinktime = thinktime;
//	//origin of the bot
	VectorCopy(bs->cur_ps.origin, bs->origin);

//	//eye coordinates of the bot
	VectorCopy(bs->cur_ps.origin, bs->eye);
	bs->eye[2] += bs->cur_ps.viewheight;
//	//get the area the bot is in
//	bs->areanum = BotPointAreaNum(bs->origin);

//	//the real AI
	BotDeathmatchAI(bs, thinktime);

//	//set the weapon selection every AI frame
	BotAI_SelectWeapon(bs, bs->weaponnum);

	//subtract the delta angles
	//for (j = 0; j < 3; j++) {
	//	bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	//}
	//everything was ok
	return qtrue;
}

/*
==================
BotAIStartFrame
==================
*/
int BotAIStartFrame(int time) {
	int elapsed_time;

	floattime = (float)time / 1000;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (!botstates[i] || !botstates[i]->inuse) {
			continue;
		}
		if (g_entities[i].client->pers.connected != CON_CONNECTED) {
			continue;
		}

		BotAI(i, floattime);
	}

	G_CheckBotSpawn();

	elapsed_time = time - local_time;
	local_time = time;

	// execute bot user commands every frame
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (!botstates[i] || !botstates[i]->inuse) {
			continue;
		}
		if (g_entities[i].client->pers.connected != CON_CONNECTED) {
			continue;
		}

		BotUpdateInput(botstates[i], time, elapsed_time);
		trap_SV_ClientThink(botstates[i]->client, &botstates[i]->lastucmd);
	}
	return 1;
}

/*
==============
BotAIShutdownClient
==============
*/
int BotAIShutdownClient(int client, qboolean restart) {
	return 1;
}

/*
==============
BotAIShutdown
==============
*/
int BotAIShutdown(int restart) {

	int i;

	//if the game is restarted for a tournament
	if (restart) {
		//shutdown all the bots in the botlib
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (botstates[i]) {
				BotAIShutdownClient(botstates[i]->client, restart);
				free(botstates[i]);
			}
		}
		//don't shutdown the bot library
	}
	else {
//		trap_BotLibShutdown();
	}
	return qtrue;
}


/*
==============
BotAISetupClient
==============
*/
int BotAISetupClient(int client, struct bot_settings_s* settings, qboolean restart) {
	bot_state_t* bs;
	int errnum;
	char filename[MAX_QPATH];
	char name[MAX_QPATH];
	char gender[MAX_QPATH];
	
	if (level.navMeshFile <= 0)
		return -1;

	if(botstates[client] != NULL) {
		G_Error("BotAISetupClient: BotState not cleaned up!\n");
	}

	botstates[client] = (bot_state_t *)malloc(sizeof(bot_state_t));
	bs = botstates[client];

	memset(bs, 0, sizeof(bot_state_t));
	bs->inuse = qtrue;

	//load the bot character
	bs->character = BotLoadCharacterFromFile(settings->characterfile, settings->skill);
	if (!bs->character) {
		// Try and load the bot with a skill level of 1.
		G_Printf("BotAISetupClient: Client %d has missing skill %d\n", client, settings->skill);

		settings->skill = 1;
		bs->character = BotLoadCharacterFromFile(settings->characterfile, settings->skill);
		if (bs->character == NULL)
		{
			G_Error("couldn't load skill %f from %s\n", settings->skill, settings->characterfile);
			return qfalse;
		}
	}

	//copy the settings
	memcpy(&bs->settings, settings, sizeof(bot_settings_t));

	//allocate a goal state
	bs->gs = BotAllocGoalState(client);

	//load the item weights
	Characteristic_String(bs->character, CHARACTERISTIC_ITEMWEIGHTS, filename, MAX_QPATH);

	errnum = BotLoadItemWeights(bs->gs, filename);
	if (errnum != BLERR_NOERROR) {
		BotFreeGoalState(bs->gs);
		return qfalse;
	}

	////allocate a weapon state
	bs->ws = BotAllocWeaponState();

	//load the weapon weights
	Characteristic_String(bs->character, CHARACTERISTIC_WEAPONWEIGHTS, filename, MAX_QPATH);
	errnum = BotLoadWeaponWeights(bs->ws, filename);
	if (errnum != BLERR_NOERROR) {
	//	trap_BotFreeGoalState(bs->gs);
		BotFreeWeaponState(bs->ws);
		return qfalse;
	}

	////allocate a chat state
	//bs->cs = trap_BotAllocChatState();
	////load the chat file
	//trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_FILE, filename, MAX_PATH);
	//trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_NAME, name, MAX_PATH);
	//errnum = trap_BotLoadChatFile(bs->cs, filename, name);
	//if (errnum != BLERR_NOERROR) {
	//	trap_BotFreeChatState(bs->cs);
	//	trap_BotFreeGoalState(bs->gs);
	//	trap_BotFreeWeaponState(bs->ws);
	//	return qfalse;
	//}
	////get the gender characteristic
	//trap_Characteristic_String(bs->character, CHARACTERISTIC_GENDER, gender, MAX_PATH);
	////set the chat gender
	//if (*gender == 'f' || *gender == 'F') trap_BotSetChatGender(bs->cs, CHAT_GENDERFEMALE);
	//else if (*gender == 'm' || *gender == 'M') trap_BotSetChatGender(bs->cs, CHAT_GENDERMALE);
	//else trap_BotSetChatGender(bs->cs, CHAT_GENDERLESS);
	//
	bs->inuse = qtrue;
	bs->client = client;
	bs->entitynum = client;
	bs->setupcount = 4;
	bs->entergame_time = FloatTime();
	//bs->ms = BotAllocMoveState();
	//bs->walker = Characteristic_BFloat(bs->character, CHARACTERISTIC_WALKER, 0, 1);
	//numbots++;
	//
	//if (trap_Cvar_VariableIntegerValue("bot_testichat")) {
	//	trap_BotLibVarSet("bot_testichat", "1");
	//	BotChatTest(bs);
	//}
	////NOTE: reschedule the bot thinking
	//BotScheduleBotThink();
	//
	////if interbreeding start with a mutation
	//if (bot_interbreed) {
	//	trap_BotMutateGoalFuzzyLogic(bs->gs, 1);
	//}
	//
	//// if we kept the bot client
	//if (restart) {
	//	BotReadSessionData(bs);
	//}
	//bot has been setup succesfully
	
	//BotScheduleBotThink();
	return 1;
}

/*
==============
BotAISetup
==============
*/
int BotAISetup(int restart) {
	InitCharacteristicSystem();
	return 1;
}

/*
==============
BotAILoadMap
==============
*/
int BotAILoadMap(int restart) {
	if (restart)
		return 1;

	// Try and load the navmesh.
	level.navMeshFile = trap_Nav_LoadMesh(va("maps/%s.bsp", g_mapName.string));
	if (level.navMeshFile <= 0)
		return -1;

	return 1;
}

/*
==============
BotAIPostSpawn
==============
*/
void BotAIPostSpawn(void) {
	BotSetupGoalAI();
	BotSetupWeaponAI();
	BotInitLevelItems();
}