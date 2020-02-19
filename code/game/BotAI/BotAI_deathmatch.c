// botai_dmq3.c
//

#include "../q_shared.h"
#include "../inv.h"
#include "../g_local.h"
#include "BotAI.h"


#define MAX_NODESWITCHES	50


/*
==================
AINode_Seek_NBG
==================
*/
int AINode_Seek_NBG(bot_state_t* bs) {
	bot_goal_t goal;
	vec3_t target, dir;
	//bot_moveresult_t moveresult;

	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "seek nbg: observer");
	//	return qfalse;
	//}
	////if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "seek nbg: intermision");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "seek nbg: bot dead");
		return qfalse;
	}
	
	//bs->tfl = TFL_DEFAULT;
	//if (bot_grapple.integer) bs->tfl |= TFL_GRAPPLEHOOK;
	////if in lava or slime the bot should be able to get out
	//if (BotInLavaOrSlime(bs)) bs->tfl |= TFL_LAVA | TFL_SLIME;
	////
	//if (BotCanAndWantsToRocketJump(bs)) {
	//	bs->tfl |= TFL_ROCKETJUMP;
	//}
	////map specific code
	//BotMapScripts(bs);
	//no enemy
	bs->enemy = -1;
	//if the bot has no goal
	if (!BotGetTopGoal(bs->gs, &goal)) 
		bs->nbg_time = 0;
	//if the bot touches the current goal
	else if (BotReachedGoal(bs, &goal)) {
		BotChooseWeapon(bs);
		bs->nbg_time = 0;
	}
	
	if (bs->nbg_time < FloatTime()) {
		//pop the current goal from the stack
		BotPopGoal(bs->gs);
		//check for new nearby items right away
		//NOTE: we canNOT reset the check_time to zero because it would create an endless loop of node switches
		bs->check_time = FloatTime() + 0.05;
		//go back to seek ltg
		AIEnter_Seek_LTG(bs, "seek nbg: time out");
		return qfalse;
	}

	//predict obstacles
	//if (BotAIPredictObstacles(bs, &goal))
	//	return qfalse;
	////initialize the movement state
	//BotSetupForMovement(bs);
	////move towards the goal
	//trap_BotMoveToGoal(&moveresult, bs->ms, &goal, bs->tfl);
	////if the movement failed
	//if (moveresult.failure) {
	//	//reset the avoid reach, otherwise bot is stuck in current area
	//	trap_BotResetAvoidReach(bs->ms);
	//	bs->nbg_time = 0;
	//}
	BotMoveToGoal(bs, &goal);

	//check if the bot is blocked
	//BotAIBlocked(bs, &moveresult, qtrue);
	////
	//BotClearPath(bs, &moveresult);

// jmarshall - fix look at code.
	//if the viewangles are used for the movement
	//if (moveresult.flags & (MOVERESULT_MOVEMENTVIEWSET | MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW)) {
	//	VectorCopy(moveresult.ideal_viewangles, bs->ideal_viewangles);
	//}
	////if waiting for something
	//else if (moveresult.flags & MOVERESULT_WAITING) {
	//	if (random() < bs->thinktime * 0.8) {
	//		BotRoamGoal(bs, target);
	//		VectorSubtract(target, bs->origin, dir);
	//		vectoangles(dir, bs->ideal_viewangles);
	//		bs->ideal_viewangles[2] *= 0.5;
	//	}
	//}
	//else if (!(bs->flags & BFL_IDEALVIEWSET)) {
	//	if (!trap_BotGetSecondGoal(bs->gs, &goal)) trap_BotGetTopGoal(bs->gs, &goal);
	//	if (trap_BotMovementViewTarget(bs->ms, &goal, bs->tfl, 300, target)) {
	//		VectorSubtract(target, bs->origin, dir);
	//		vectoangles(dir, bs->ideal_viewangles);
	//	}
	//	//FIXME: look at cluster portals?
	//	else vectoangles(moveresult.movedir, bs->ideal_viewangles);
	//	bs->ideal_viewangles[2] *= 0.5;
	//}
	////if the weapon is used for the bot movement
	//if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) bs->weaponnum = moveresult.weapon;
// jmarshall end
	//if there is an enemy
	if (BotFindEnemy(bs, -1)) {
		if (BotWantsToRetreat(bs)) {
			//keep the current long term goal and retreat
			AIEnter_Battle_NBG(bs, "seek nbg: found enemy");
		}
		else {
			//trap_BotResetLastAvoidReach(bs->ms);
			//empty the goal stack
			BotEmptyGoalStack(bs->gs);
			//go fight
			AIEnter_Battle_Fight(bs, "seek nbg: found enemy");
		}
	}
	return qtrue;
}

/*
==================
AIEnter_Seek_NBG
==================
*/
void AIEnter_Seek_NBG(bot_state_t* bs, char* s) {
	//bot_goal_t goal;
	//char buf[144];
	//
	//if (trap_BotGetTopGoal(bs->gs, &goal)) {
	//	trap_BotGoalName(goal.number, buf, 144);
	//	BotRecordNodeSwitch(bs, "seek NBG", buf, s);
	//}
	//else {
	//	BotRecordNodeSwitch(bs, "seek NBG", "no goal", s);
	//}
	bs->ainode = AINode_Seek_NBG;
}

/*
==================
BotAggression
==================
*/
float BotAggression(bot_state_t* bs) {
	//if the bot has quad
	if (bs->inventory[INVENTORY_QUAD]) {
		//if the bot is not holding the gauntlet or the enemy is really nearby
		if (bs->weaponnum != WP_GAUNTLET ||
			bs->inventory[ENEMY_HORIZONTAL_DIST] < 80) {
			return 70;
		}
	}
	//if the enemy is located way higher than the bot
	if (bs->inventory[ENEMY_HEIGHT] > 200) return 0;
	//if the bot is very low on health
	if (bs->inventory[INVENTORY_HEALTH] < 60) return 0;
	//if the bot is low on health
	if (bs->inventory[INVENTORY_HEALTH] < 80) {
		//if the bot has insufficient armor
		if (bs->inventory[INVENTORY_ARMOR] < 40) return 0;
	}
	//if the bot can use the bfg
	if (bs->inventory[INVENTORY_BFG10K] > 0 &&
		bs->inventory[INVENTORY_BFGAMMO] > 7) return 100;
	//if the bot can use the railgun
	if (bs->inventory[INVENTORY_RAILGUN] > 0 &&
		bs->inventory[INVENTORY_SLUGS] > 5) return 95;
	//if the bot can use the lightning gun
	if (bs->inventory[INVENTORY_LIGHTNING] > 0 &&
		bs->inventory[INVENTORY_LIGHTNINGAMMO] > 50) return 90;
	//if the bot can use the rocketlauncher
	if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 &&
		bs->inventory[INVENTORY_ROCKETS] > 5) return 90;
	//if the bot can use the plasmagun
	if (bs->inventory[INVENTORY_PLASMAGUN] > 0 &&
		bs->inventory[INVENTORY_CELLS] > 40) return 85;
	//if the bot can use the grenade launcher
	if (bs->inventory[INVENTORY_GRENADELAUNCHER] > 0 &&
		bs->inventory[INVENTORY_GRENADES] > 10) return 80;
	//if the bot can use the shotgun
	if (bs->inventory[INVENTORY_SHOTGUN] > 0 &&
		bs->inventory[INVENTORY_SHELLS] > 10) return 50;
	//otherwise the bot is not feeling too good
	return 0;
}


/*
==================
BotWantsToRetreat
==================
*/
int BotWantsToRetreat(bot_state_t* bs) {
//	aas_entityinfo_t entinfo;
//
//	if (gametype == GT_CTF) {
//		//always retreat when carrying a CTF flag
//		if (BotCTFCarryingFlag(bs))
//			return qtrue;
//	}
//#ifdef MISSIONPACK
//	else if (gametype == GT_1FCTF) {
//		//if carrying the flag then always retreat
//		if (Bot1FCTFCarryingFlag(bs))
//			return qtrue;
//	}
//	else if (gametype == GT_OBELISK) {
//		//the bots should be dedicated to attacking the enemy obelisk
//		if (bs->ltgtype == LTG_ATTACKENEMYBASE) {
//			if (bs->enemy != redobelisk.entitynum ||
//				bs->enemy != blueobelisk.entitynum) {
//				return qtrue;
//			}
//		}
//		if (BotFeelingBad(bs) > 50) {
//			return qtrue;
//		}
//		return qfalse;
//	}
//	else if (gametype == GT_HARVESTER) {
//		//if carrying cubes then always retreat
//		if (BotHarvesterCarryingCubes(bs)) return qtrue;
//	}
//#endif
//	//
//	if (bs->enemy >= 0) {
//		//if the enemy is carrying a flag
//		BotEntityInfo(bs->enemy, &entinfo);
//		if (EntityCarriesFlag(&entinfo))
//			return qfalse;
//	}
//	//if the bot is getting the flag
//	if (bs->ltgtype == LTG_GETFLAG)
//		return qtrue;
//	//
	if (BotAggression(bs) < 50)
		return qtrue;
	return qfalse;
}

/*
==================
BotWantsToChase
==================
*/
int BotWantsToChase(bot_state_t* bs) {
//	aas_entityinfo_t entinfo;
//
//	if (gametype == GT_CTF) {
//		//never chase when carrying a CTF flag
//		if (BotCTFCarryingFlag(bs))
//			return qfalse;
//		//always chase if the enemy is carrying a flag
//		BotEntityInfo(bs->enemy, &entinfo);
//		if (EntityCarriesFlag(&entinfo))
//			return qtrue;
//	}
//#ifdef MISSIONPACK
//	else if (gametype == GT_1FCTF) {
//		//never chase if carrying the flag
//		if (Bot1FCTFCarryingFlag(bs))
//			return qfalse;
//		//always chase if the enemy is carrying a flag
//		BotEntityInfo(bs->enemy, &entinfo);
//		if (EntityCarriesFlag(&entinfo))
//			return qtrue;
//	}
//	else if (gametype == GT_OBELISK) {
//		//the bots should be dedicated to attacking the enemy obelisk
//		if (bs->ltgtype == LTG_ATTACKENEMYBASE) {
//			if (bs->enemy != redobelisk.entitynum ||
//				bs->enemy != blueobelisk.entitynum) {
//				return qfalse;
//			}
//		}
//	}
//	else if (gametype == GT_HARVESTER) {
//		//never chase if carrying cubes
//		if (BotHarvesterCarryingCubes(bs))
//			return qfalse;
//	}
//#endif
//	//if the bot is getting the flag
//	if (bs->ltgtype == LTG_GETFLAG)
//		return qfalse;
	//
	if (BotAggression(bs) > 50)
		return qtrue;
	return qfalse;
}

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
		BotUpdateInventory(bs);
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

	gentity_t* entinfo = &g_entities[bs->client];
	VectorCopy(entinfo->s.angles, bs->viewangles);
	
	// if the bot has no ai node
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
// jmarshall - keep the last visible position
		if (bs->enemy != -1)
		{
			if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)) {
				gentity_t* ent = &g_entities[bs->enemy];
				VectorCopy(ent->r.currentOrigin, bs->last_enemy_visible_position);
			}
		}
// jmarshall end

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