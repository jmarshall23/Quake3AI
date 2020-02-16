// botai_dmq3.c
//

#include "../q_shared.h"
#include "BotAI.h"

#define MAX_NODESWITCHES	50

/*
==================
BotDeathmatchAI
==================
*/
void BotDeathmatchAI(bot_state_t* bs, float thinktime) {
	//char gender[144], name[144], buf[144];
	//char userinfo[MAX_INFO_STRING];
	//int i;
	//
	////if the bot has just been setup
	//if (bs->setupcount > 0) {
	//	bs->setupcount--;
	//	if (bs->setupcount > 0) return;
	//	//get the gender characteristic
	//	trap_Characteristic_String(bs->character, CHARACTERISTIC_GENDER, gender, sizeof(gender));
	//	//set the bot gender
	//	trap_GetUserinfo(bs->client, userinfo, sizeof(userinfo));
	//	Info_SetValueForKey(userinfo, "sex", gender);
	//	trap_SetUserinfo(bs->client, userinfo);
	//	//set the team
	//	if (!bs->map_restart && g_gametype.integer != GT_TOURNAMENT) {
	//		Com_sprintf(buf, sizeof(buf), "team %s", bs->settings.team);
	//		trap_EA_Command(bs->client, buf);
	//	}
	//	//set the chat gender
	//	if (gender[0] == 'm') trap_BotSetChatGender(bs->cs, CHAT_GENDERMALE);
	//	else if (gender[0] == 'f')  trap_BotSetChatGender(bs->cs, CHAT_GENDERFEMALE);
	//	else  trap_BotSetChatGender(bs->cs, CHAT_GENDERLESS);
	//	//set the chat name
	//	ClientName(bs->client, name, sizeof(name));
	//	trap_BotSetChatName(bs->cs, name, bs->client);
	//	//
	//	bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
	//	bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
	//	//
	//	bs->setupcount = 0;
	//	//
	//	BotSetupAlternativeRouteGoals();
	//}
	////no ideal view set
	//bs->flags &= ~BFL_IDEALVIEWSET;
	////
	//if (!BotIntermission(bs)) {
	//	//set the teleport time
	//	BotSetTeleportTime(bs);
	//	//update some inventory values
	//	BotUpdateInventory(bs);
	//	//check out the snapshot
	//	BotCheckSnapshot(bs);
	//	//check for air
	//	BotCheckAir(bs);
	//}
	////check the console messages
	//BotCheckConsoleMessages(bs);
	////if not in the intermission and not in observer mode
	//if (!BotIntermission(bs) && !BotIsObserver(bs)) {
		//do team AI
	//	BotTeamAI(bs);
	//}
	
	//if the bot has no ai node
	if (!bs->ainode) {
		AIEnter_Seek_LTG(bs, "BotDeathmatchAI: no ai node");
	}
	//if the bot entered the game less than 8 seconds ago
	//if (!bs->entergamechat && bs->entergame_time > FloatTime() - 8) {
	//	if (BotChat_EnterGame(bs)) {
	//		bs->stand_time = FloatTime() + BotChatTime(bs);
	//		AIEnter_Stand(bs, "BotDeathmatchAI: chat enter game");
	//	}
	//	bs->entergamechat = qtrue;
	//}
	////reset the node switches from the previous frame
	//BotResetNodeSwitches();
	////execute AI nodes
	for (int i = 0; i < MAX_NODESWITCHES; i++) {
		if (bs->ainode(bs)) 
			break;
	}
	////if the bot removed itself :)
	if (!bs->inuse) 
		return;
	
	////if the bot executed too many AI nodes
	//if (i >= MAX_NODESWITCHES) {
	//	trap_BotDumpGoalStack(bs->gs);
	//	trap_BotDumpAvoidGoals(bs->gs);
	//	BotDumpNodeSwitches(bs);
	//	ClientName(bs->client, name, sizeof(name));
	//	BotAI_Print(PRT_ERROR, "%s at %1.1f switched more than %d AI nodes\n", name, FloatTime(), MAX_NODESWITCHES);
	//}
	////
	//bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
	//bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
}