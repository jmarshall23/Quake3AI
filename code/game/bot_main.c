// bot_main.c
//

#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

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
BotAIRespawn
==================
*/
int BotAIRespawn(bot_state_t* bs) {
	bs->input.actionflags |= ACTION_RESPAWN;

	//if(bs->respawn_time == 0) {
	//	bs->respawn_time = FloatTime() + 3.0f;
	//}
	//
	//if (bs->respawn_time < FloatTime()) {
	//	// wait until respawned
	//	bs->respawn_wait = qtrue;
	//	// elementary action respawn
	//	//trap_EA_Respawn(bs->client);
	//	bs->input.actionflags |= ACTION_RESPAWN;
	//	//
	//	if (bs->respawnchat_time) {
	//		//trap_BotEnterChat(bs->cs, 0, bs->chatto);
	//		bs->enemy = -1;
	//	}
	//
	//	bs->respawn_time = 0;
	//}
	//if (bs->respawnchat_time && bs->respawnchat_time < FloatTime() - 0.5) {
	//	//trap_EA_Talk(bs->client);
	//}
	//
	return qtrue;
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
	if (BotIsDead(bs))
	{
		BotAIRespawn(bs);
	}
	else
	{
	//	BotDeathmatchAI(bs, thinktime);
	}
//	//set the weapon selection every AI frame
	BotAI_SelectWeapon(bs, bs->weaponnum);

	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
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
	if (level.navMeshFile <= 0)
		return -1;

	if(botstates[client] != NULL) {
		G_Error("BotAISetupClient: BotState not cleaned up!\n");
	}

	botstates[client] = (bot_state_t *)malloc(sizeof(bot_state_t));
	memset(botstates[client], 0, sizeof(bot_state_t));
	botstates[client]->inuse = qtrue;

	botstates[client]->client = client;

	//BotScheduleBotThink();
	return 1;
}

/*
==============
BotAISetup
==============
*/
int BotAISetup(int restart) {
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