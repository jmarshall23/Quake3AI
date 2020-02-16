// BotAI_ltg.c
//

#include "../q_shared.h"
#include "BotAI.h"

/*
==================
AINode_Respawn
==================
*/
int AINode_Respawn(bot_state_t* bs) {
	bs->input.actionflags |= ACTION_RESPAWN;
	return qtrue;
}


/*
==================
AIEnter_Respawn
==================
*/
void AIEnter_Respawn(bot_state_t* bs, char* s) {
	//BotRecordNodeSwitch(bs, "respawn", "", s);
	////reset some states
	////trap_BotResetMoveState(bs->ms);
	//BotResetGoalState(bs->gs);
	////trap_BotResetAvoidGoals(bs->gs);
	////trap_BotResetAvoidReach(bs->ms);
	//
	////if the bot wants to chat
	//if (BotChat_Death(bs)) {
	//	bs->respawn_time = FloatTime() + BotChatTime(bs);
	//	bs->respawnchat_time = FloatTime();
	//}
	//else {
	//	bs->respawn_time = FloatTime() + 1 + random();
	//	bs->respawnchat_time = 0;
	//}
	//set respawn state
	bs->respawn_wait = qfalse;
	bs->ainode = AINode_Respawn;
}

/*
==================
AINode_Seek_LTG
==================
*/
int AINode_Seek_LTG(bot_state_t* bs)
{
	bot_goal_t goal;
	vec3_t target, dir;
	//bot_moveresult_t moveresult;
	int range;
	//char buf[128];
	//bot_goal_t tmpgoal;

	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "seek ltg: observer");
	//	return qfalse;
	//}
	//if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "seek ltg: intermission");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "seek ltg: bot dead");
		return qfalse;
	}
	//
//	if (BotChat_Random(bs)) {
//		bs->stand_time = FloatTime() + BotChatTime(bs);
//		AIEnter_Stand(bs, "seek ltg: random chat");
//		return qfalse;
//	}
//	//
//	bs->tfl = TFL_DEFAULT;
//	if (bot_grapple.integer) bs->tfl |= TFL_GRAPPLEHOOK;
//	//if in lava or slime the bot should be able to get out
//	if (BotInLavaOrSlime(bs)) bs->tfl |= TFL_LAVA | TFL_SLIME;
//	//
//	if (BotCanAndWantsToRocketJump(bs)) {
//		bs->tfl |= TFL_ROCKETJUMP;
//	}
//	//map specific code
//	BotMapScripts(bs);
//	//no enemy
//	bs->enemy = -1;
//	//
//	if (bs->killedenemy_time > FloatTime() - 2) {
//		if (random() < bs->thinktime * 1) {
//			trap_EA_Gesture(bs->client);
//		}
//	}
//	//if there is an enemy
//	if (BotFindEnemy(bs, -1)) {
//		if (BotWantsToRetreat(bs)) {
//			//keep the current long term goal and retreat
//			AIEnter_Battle_Retreat(bs, "seek ltg: found enemy");
//			return qfalse;
//		}
//		else {
//			trap_BotResetLastAvoidReach(bs->ms);
//			//empty the goal stack
//			trap_BotEmptyGoalStack(bs->gs);
//			//go fight
//			AIEnter_Battle_Fight(bs, "seek ltg: found enemy");
//			return qfalse;
//		}
//	}
//	//
//	BotTeamGoals(bs, qfalse);
//	//get the current long term goal
//	if (!BotLongTermGoal(bs, bs->tfl, qfalse, &goal)) {
//		return qtrue;
//	}
//	//check for nearby goals periodicly
//	if (bs->check_time < FloatTime()) {
//		bs->check_time = FloatTime() + 0.5;
//		//check if the bot wants to camp
//		BotWantsToCamp(bs);
//		//
//		if (bs->ltgtype == LTG_DEFENDKEYAREA) range = 400;
//		else range = 150;
//		//
//#ifdef CTF
//		if (gametype == GT_CTF) {
//			//if carrying a flag the bot shouldn't be distracted too much
//			if (BotCTFCarryingFlag(bs))
//				range = 50;
//		}
//#endif //CTF
//#ifdef MISSIONPACK
//		else if (gametype == GT_1FCTF) {
//			if (Bot1FCTFCarryingFlag(bs))
//				range = 50;
//		}
//		else if (gametype == GT_HARVESTER) {
//			if (BotHarvesterCarryingCubes(bs))
//				range = 80;
//		}
//#endif
//		//
//		if (BotNearbyGoal(bs, bs->tfl, &goal, range)) {
//			trap_BotResetLastAvoidReach(bs->ms);
//			//get the goal at the top of the stack
//			//trap_BotGetTopGoal(bs->gs, &tmpgoal);
//			//trap_BotGoalName(tmpgoal.number, buf, 144);
//			//BotAI_Print(PRT_MESSAGE, "new nearby goal %s\n", buf);
//			//time the bot gets to pick up the nearby goal item
//			bs->nbg_time = FloatTime() + 4 + range * 0.01;
//			AIEnter_Seek_NBG(bs, "ltg seek: nbg");
//			return qfalse;
//		}
//	}
//	//predict obstacles
//	if (BotAIPredictObstacles(bs, &goal))
//		return qfalse;
//	//initialize the movement state
//	BotSetupForMovement(bs);
//	//move towards the goal
//	trap_BotMoveToGoal(&moveresult, bs->ms, &goal, bs->tfl);
//	//if the movement failed
//	if (moveresult.failure) {
//		//reset the avoid reach, otherwise bot is stuck in current area
//		trap_BotResetAvoidReach(bs->ms);
//		//BotAI_Print(PRT_MESSAGE, "movement failure %d\n", moveresult.traveltype);
//		bs->ltg_time = 0;
//	}
//	//
//	BotAIBlocked(bs, &moveresult, qtrue);
//	//
//	BotClearPath(bs, &moveresult);
//	//if the viewangles are used for the movement
//	if (moveresult.flags & (MOVERESULT_MOVEMENTVIEWSET | MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW)) {
//		VectorCopy(moveresult.ideal_viewangles, bs->ideal_viewangles);
//	}
//	//if waiting for something
//	else if (moveresult.flags & MOVERESULT_WAITING) {
//		if (random() < bs->thinktime * 0.8) {
//			BotRoamGoal(bs, target);
//			VectorSubtract(target, bs->origin, dir);
//			vectoangles(dir, bs->ideal_viewangles);
//			bs->ideal_viewangles[2] *= 0.5;
//		}
//	}
//	else if (!(bs->flags & BFL_IDEALVIEWSET)) {
//		if (trap_BotMovementViewTarget(bs->ms, &goal, bs->tfl, 300, target)) {
//			VectorSubtract(target, bs->origin, dir);
//			vectoangles(dir, bs->ideal_viewangles);
//		}
//		//FIXME: look at cluster portals?
//		else if (VectorLengthSquared(moveresult.movedir)) {
//			vectoangles(moveresult.movedir, bs->ideal_viewangles);
//		}
//		else if (random() < bs->thinktime * 0.8) {
//			BotRoamGoal(bs, target);
//			VectorSubtract(target, bs->origin, dir);
//			vectoangles(dir, bs->ideal_viewangles);
//			bs->ideal_viewangles[2] *= 0.5;
//		}
//		bs->ideal_viewangles[2] *= 0.5;
//	}
//	//if the weapon is used for the bot movement
//	if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) bs->weaponnum = moveresult.weapon;
//	//
	return qtrue;
}

/*
==================
AIEnter_Seek_LTG
==================
*/
void AIEnter_Seek_LTG(bot_state_t* bs, char* s) {
	bot_goal_t goal;
	char buf[144];

	if (BotGetTopGoal(bs->gs, &goal)) {
		BotGoalName(goal.number, buf, 144);
//		BotRecordNodeSwitch(bs, "seek LTG", buf, s);
	}
	else {
//		BotRecordNodeSwitch(bs, "seek LTG", "no goal", s);
	}
	bs->ainode = AINode_Seek_LTG;
}

