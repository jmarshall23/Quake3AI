// bot_main.c
//

#include "q_shared.h"
#include "g_local.h"

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
BotAIStartFrame
==================
*/
int BotAIStartFrame(int time) {
	G_CheckBotSpawn();
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
			//BotAIShutdownClient(botstates[i]->client, restart);
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