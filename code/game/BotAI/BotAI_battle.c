// BotAI_battle.c
//

#include "../q_shared.h"
#include "../inv.h"
#include "../g_local.h"
#include "../chars.h"
#include "BotAI.h"

#define IDEAL_ATTACKDIST			140


/*
==================
AINode_Battle_NBG
==================
*/
int AINode_Battle_NBG(bot_state_t* bs) {
	int areanum;
	bot_goal_t goal;
	//aas_entityinfo_t entinfo;
	gentity_t* entinfo;
	//bot_moveresult_t moveresult;
	float attack_skill;
	vec3_t target, dir;
	
	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "battle nbg: observer");
	//	return qfalse;
	//}
	////if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "battle nbg: intermission");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "battle nbg: bot dead");
		return qfalse;
	}
	
	//if no enemy
	if (bs->enemy < 0) {
		AIEnter_Seek_NBG(bs, "battle nbg: no enemy");
		return qfalse;
	}
	
	//BotEntityInfo(bs->enemy, &entinfo);
	entinfo = &g_entities[bs->enemy];
	if (EntityIsDead(entinfo)) {
		AIEnter_Seek_NBG(bs, "battle nbg: enemy dead");
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

	//update the last time the enemy was visible
	if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)) {
		bs->enemyvisible_time = FloatTime();
		VectorCopy(entinfo->r.currentOrigin, target);
		// if not a player enemy
		if (bs->enemy >= MAX_CLIENTS) {
#ifdef MISSIONPACK
			// if attacking an obelisk
			if (bs->enemy == redobelisk.entitynum ||
				bs->enemy == blueobelisk.entitynum) {
				target[2] += 16;
			}
#endif
		}
		//update the reachability area and origin if possible
		//areanum = BotPointAreaNum(target);
		//if (areanum && trap_AAS_AreaReachability(areanum)) {
			VectorCopy(target, bs->lastenemyorigin);
		//	bs->lastenemyareanum = areanum;
		//}
	}
	
	//if the bot has no goal or touches the current goal
	if (!BotGetTopGoal(bs->gs, &goal)) {
		bs->nbg_time = 0;
	}
	else if (BotReachedGoal(bs, &goal)) {
		bs->nbg_time = 0;
	}
	//
	if (bs->nbg_time < FloatTime()) {
		//pop the current goal from the stack
		BotPopGoal(bs->gs);
		//if the bot still has a goal
		if (BotGetTopGoal(bs->gs, &goal))
			AIEnter_Battle_Retreat(bs, "battle nbg: time out");
		else
			AIEnter_Battle_Fight(bs, "battle nbg: time out");
		
		return qfalse;
	}

	//move towards the goal
	BotMoveToGoal(bs, &goal);

	//initialize the movement state
	//BotSetupForMovement(bs);
	////move towards the goal
	//trap_BotMoveToGoal(&moveresult, bs->ms, &goal, bs->tfl);
	////if the movement failed
	//if (moveresult.failure) {
	//	//reset the avoid reach, otherwise bot is stuck in current area
	//	trap_BotResetAvoidReach(bs->ms);
	//	//BotAI_Print(PRT_MESSAGE, "movement failure %d\n", moveresult.traveltype);
	//	bs->nbg_time = 0;
	//}
	////
	//BotAIBlocked(bs, &moveresult, qfalse);

	//update the attack inventory values
	BotUpdateBattleInventory(bs, bs->enemy);
	
	//choose the best weapon to fight with
	BotChooseWeapon(bs);
	
	//if the view is fixed for the movement
	//if (moveresult.flags & (MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW)) {
	//	VectorCopy(moveresult.ideal_viewangles, bs->ideal_viewangles);
	//}
	//else if (!(moveresult.flags & MOVERESULT_MOVEMENTVIEWSET)
	//	&& !(bs->flags & BFL_IDEALVIEWSET)) {
	//	attack_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ATTACK_SKILL, 0, 1);
	//	//if the bot is skilled anough and the enemy is visible
	//	if (attack_skill > 0.3) {
	//		//&& BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)
	//		BotAimAtEnemy(bs);
	//	}
	//	else {
	//		if (trap_BotMovementViewTarget(bs->ms, &goal, bs->tfl, 300, target)) {
	//			VectorSubtract(target, bs->origin, dir);
	//			vectoangles(dir, bs->ideal_viewangles);
	//		}
	//		else {
	//			vectoangles(moveresult.movedir, bs->ideal_viewangles);
	//		}
	//		bs->ideal_viewangles[2] *= 0.5;
	//	}
	//}
	//if (attack_skill > 0.3) {
		//&& BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)
		BotAimAtEnemy(bs);
	//}

	//if the weapon is used for the bot movement
	//if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) bs->weaponnum = moveresult.weapon;
	//attack the enemy if possible
	BotCheckAttack(bs);
	//
	return qtrue;
}

/*
==================
AIEnter_Battle_NBG
==================
*/
void AIEnter_Battle_NBG(bot_state_t* bs, char* s) {
	//BotRecordNodeSwitch(bs, "battle NBG", "", s);
	bs->ainode = AINode_Battle_NBG;
}


/*
==================
BotAttackMove
==================
*/
void BotAttackMove(bot_state_t* bs, int tfl) {
	botMoveFlags_t movetype;
	int i, attackentity;
	float attack_skill, jumper, croucher, dist, strafechange_time;
	float attack_dist, attack_range;
	vec3_t forward, backward, sideward, hordir, up = { 0, 0, 1 };
	gentity_t *entinfo;
	//bot_moveresult_t moveresult;
	bot_goal_t goal;

	attackentity = bs->enemy;
	
	if (bs->attackchase_time > FloatTime()) {
		//create the chase goal
		goal.entitynum = attackentity;
		goal.areanum = bs->lastenemyareanum;
		VectorCopy(bs->lastenemyorigin, goal.origin);
		VectorSet(goal.mins, -8, -8, -8);
		VectorSet(goal.maxs, 8, 8, 8);
		
		//initialize the movement state
		//BotSetupForMovement(bs);
		
		//move towards the goal
		//trap_BotMoveToGoal(&moveresult, bs->ms, &goal, tfl);
		BotMoveToGoal(bs, &goal);
		
		return;
	}
	
//	memset(&moveresult, 0, sizeof(bot_moveresult_t));
	
	attack_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_ATTACK_SKILL, 0, 1);
	jumper = Characteristic_BFloat(bs->character, CHARACTERISTIC_JUMPER, 0, 1);
	croucher = Characteristic_BFloat(bs->character, CHARACTERISTIC_CROUCHER, 0, 1);
	
	//if the bot is really stupid
	if (attack_skill < 0.2) 
		return;
	
	//initialize the movement state
	//BotSetupForMovement(bs);
	
	//get the enemy entity info
	//BotEntityInfo(attackentity, &entinfo);
	entinfo = &g_entities[attackentity];

	//direction towards the enemy
	VectorSubtract(entinfo->r.currentOrigin, bs->origin, forward);

	//the distance towards the enemy
	dist = VectorNormalize(forward);
	VectorNegate(forward, backward);
	//walk, crouch or jump
	movetype = MOVE_WALK;
	//
	if (bs->attackcrouch_time < FloatTime() - 1) {
		if (random() < jumper) {
			movetype = MOVE_JUMP;
		}
		//wait at least one second before crouching again
		else if (bs->attackcrouch_time < FloatTime() - 1 && random() < croucher) {
			bs->attackcrouch_time = FloatTime() + croucher * 5;
		}
	}
	if (bs->attackcrouch_time > FloatTime()) movetype = MOVE_CROUCH;
	//if the bot should jump
	if (movetype == MOVE_JUMP) {
		//if jumped last frame
		if (bs->attackjump_time > FloatTime()) {
			movetype = MOVE_WALK;
		}
		else {
			bs->attackjump_time = FloatTime() + 1;
		}
	}
	if (bs->cur_ps.weapon == WP_GAUNTLET) {
		attack_dist = 0;
		attack_range = 0;
	}
	else {
		attack_dist = IDEAL_ATTACKDIST;
		attack_range = 40;
	}
	//if the bot is stupid
	if (attack_skill <= 0.4) {
		//just walk to or away from the enemy
		if (dist > attack_dist + attack_range) {
			if (BotMoveInDirection(bs, forward, 400, movetype)) 
				return;
		}
		if (dist < attack_dist - attack_range) {
			if (BotMoveInDirection(bs, backward, 400, movetype)) 
				return;
		}
		return;
	}

	//increase the strafe time
	bs->attackstrafe_time += bs->thinktime;

	//get the strafe change time
	strafechange_time = 0.4 + (1 - attack_skill) * 0.2;
	if (attack_skill > 0.7) strafechange_time += crandom() * 0.2;
	
	//if the strafe direction should be changed
	if (bs->attackstrafe_time > strafechange_time) {
		//some magic number :)
		if (random() > 0.935) {
			//flip the strafe direction
			bs->flags ^= BFL_STRAFERIGHT;
			bs->attackstrafe_time = 0;
		}
	}
	
	for (i = 0; i < 2; i++) {
		hordir[0] = forward[0];
		hordir[1] = forward[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//get the sideward vector
		CrossProduct(hordir, up, sideward);
		//reverse the vector depending on the strafe direction
		if (bs->flags & BFL_STRAFERIGHT) VectorNegate(sideward, sideward);
		//randomly go back a little
		if (random() > 0.9) {
			VectorAdd(sideward, backward, sideward);
		}
		else {
			//walk forward or backward to get at the ideal attack distance
			if (dist > attack_dist + attack_range) {
				VectorAdd(sideward, forward, sideward);
			}
			else if (dist < attack_dist - attack_range) {
				VectorAdd(sideward, backward, sideward);
			}
		}
		//perform the movement
		if (BotMoveInDirection(bs, sideward, 400, movetype)) {
			return;
		}

		//movement failed, flip the strafe direction
		bs->flags ^= BFL_STRAFERIGHT;
		bs->attackstrafe_time = 0;
	}
	//bot couldn't do any usefull movement
//	bs->attackchase_time = AAS_Time() + 6;
	return;
}

/*
==================
BotBattleUseItems
==================
*/
void BotBattleUseItems(bot_state_t* bs) {
	if (bs->inventory[INVENTORY_HEALTH] < 40) {
		if (bs->inventory[INVENTORY_TELEPORTER] > 0) {
//			if (!BotCTFCarryingFlag(bs) // jmarshall flag
#ifdef MISSIONPACK
				&& !Bot1FCTFCarryingFlag(bs)
				&& !BotHarvesterCarryingCubes(bs)
#endif
//				) {
//				trap_EA_Use(bs->client);
				bs->input.actionflags |= ACTION_USE;
//			}
		}
	}
	if (bs->inventory[INVENTORY_HEALTH] < 60) {
		if (bs->inventory[INVENTORY_MEDKIT] > 0) {
			//trap_EA_Use(bs->client);
			bs->input.actionflags |= ACTION_USE;
		}
	}
#ifdef MISSIONPACK
	BotUseKamikaze(bs);
	BotUseInvulnerability(bs);
#endif
}

/*
==================
AINode_Battle_Fight
==================
*/
int AINode_Battle_Fight(bot_state_t* bs) {
	int areanum;
	vec3_t target;
	gentity_t *entinfo;
	
	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "battle fight: observer");
	//	return qfalse;
	//}

	//if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "battle fight: intermission");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "battle fight: bot dead");
		return qfalse;
	}
	//if there is another better enemy
	if (BotFindEnemy(bs, bs->enemy)) {
#ifdef DEBUG
		G_Printf("found new better enemy\n");
#endif
	}

	//if no enemy
	if (bs->enemy < 0) {
		AIEnter_Seek_LTG(bs, "battle fight: no enemy");
		return qfalse;
	}
	
	//BotEntityInfo(bs->enemy, &entinfo);
	entinfo = &g_entities[bs->enemy];

	//if the enemy is dead
	if (bs->enemydeath_time) {
		if (bs->enemydeath_time < FloatTime() - 1.0) {
			bs->enemydeath_time = 0;
			if (bs->enemysuicide) {
// jmarshall - bot chat
				//BotChat_EnemySuicide(bs);
// jmarshall end
			}
// jmarshall - bot chat stand
//			if (bs->lastkilledplayer == bs->enemy && BotChat_Kill(bs)) {
//				bs->stand_time = FloatTime() + BotChatTime(bs);
//				AIEnter_Stand(bs, "battle fight: enemy dead");
//			}
//			else {
				bs->ltg_time = 0;
				AIEnter_Seek_LTG(bs, "battle fight: enemy dead");
//			}
// jmarshall end
			return qfalse;
		}
	}
	else {
		if (EntityIsDead(entinfo)) {
			bs->enemydeath_time = FloatTime();
		}
	}
	//if the enemy is invisible and not shooting the bot looses track easily
	if (EntityIsInvisible(entinfo) && !EntityIsShooting(entinfo)) {
		if (random() < 0.2) {
			AIEnter_Seek_LTG(bs, "battle fight: invisible");
			return qfalse;
		}
	}
	//
	VectorCopy(entinfo->r.currentOrigin, target);
	
	// if not a player enemy
	if (bs->enemy >= MAX_CLIENTS) {
#ifdef MISSIONPACK
		// if attacking an obelisk
		if (bs->enemy == redobelisk.entitynum ||
			bs->enemy == blueobelisk.entitynum) {
			target[2] += 16;
		}
#endif
	}

	//update the reachability area and origin if possible
	//areanum = BotPointAreaNum(target);
	//if (areanum && trap_AAS_AreaReachability(areanum)) {
	//	VectorCopy(target, bs->lastenemyorigin);
	//	bs->lastenemyareanum = areanum;
	//}
//	bs->lastenemyareanum = areanum;

	//update the attack inventory values
	BotUpdateBattleInventory(bs, bs->enemy);

	//if the bot's health decreased
// jmarshall - bot chat
	//if (bs->lastframe_health > bs->inventory[INVENTORY_HEALTH]) {
	//	if (BotChat_HitNoDeath(bs)) {
	//		bs->stand_time = FloatTime() + BotChatTime(bs);
	//		AIEnter_Stand(bs, "battle fight: chat health decreased");
	//		return qfalse;
	//	}
	//}

	//if the bot hit someone
	//if (bs->cur_ps.persistant[PERS_HITS] > bs->lasthitcount) {
	//	if (BotChat_HitNoKill(bs)) {
	//		bs->stand_time = FloatTime() + BotChatTime(bs);
	//		AIEnter_Stand(bs, "battle fight: chat hit someone");
	//		return qfalse;
	//	}
	//}
// jmarshall end

	//if the enemy is not visible
	if (!BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)) {
		if (BotWantsToChase(bs)) {
			AIEnter_Battle_Chase(bs, "battle fight: enemy out of sight");
			return qfalse;
		}
		else {
			AIEnter_Seek_LTG(bs, "battle fight: enemy out of sight");
			return qfalse;
		}
	}
	//use holdable items
	BotBattleUseItems(bs);
	//
	//bs->tfl = TFL_DEFAULT;
	//if (bot_grapple.integer) bs->tfl |= TFL_GRAPPLEHOOK;
	////if in lava or slime the bot should be able to get out
	//if (BotInLavaOrSlime(bs)) bs->tfl |= TFL_LAVA | TFL_SLIME;
	////
	//if (BotCanAndWantsToRocketJump(bs)) {
	//	bs->tfl |= TFL_ROCKETJUMP;
	//}
	//choose the best weapon to fight with
	BotChooseWeapon(bs);
	//do attack movements
	BotAttackMove(bs, bs->tfl);
	
	//if the movement failed
	//if (moveresult.failure) {
	//	//reset the avoid reach, otherwise bot is stuck in current area
	//	trap_BotResetAvoidReach(bs->ms);
	//	//BotAI_Print(PRT_MESSAGE, "movement failure %d\n", moveresult.traveltype);
	//	bs->ltg_time = 0;
	//}
	//BotAIBlocked(bs, &moveresult, qfalse);

	//aim at the enemy
	BotAimAtEnemy(bs);
	
	//attack the enemy if possible
	BotCheckAttack(bs);
	
	//if the bot wants to retreat
	if (!(bs->flags & BFL_FIGHTSUICIDAL)) {
		if (BotWantsToRetreat(bs)) {
			AIEnter_Battle_Retreat(bs, "battle fight: wants to retreat");
			return qtrue;
		}
	}
	return qtrue;
}

/*
==================
AIEnter_Battle_Fight
==================
*/
void AIEnter_Battle_Fight(bot_state_t* bs, char* s) {
	//BotRecordNodeSwitch(bs, "battle fight", "", s);
	//trap_BotResetLastAvoidReach(bs->ms);
	bs->ainode = AINode_Battle_Fight;
}


/*
==================
AIEnter_Battle_Fight
==================
*/
void AIEnter_Battle_SuicidalFight(bot_state_t* bs, char* s) {
//	BotRecordNodeSwitch(bs, "battle fight", "", s);
//	trap_BotResetLastAvoidReach(bs->ms);
	bs->ainode = AINode_Battle_Fight;
	bs->flags |= BFL_FIGHTSUICIDAL;
}


/*
==================
AINode_Battle_Chase
==================
*/
int AINode_Battle_Chase(bot_state_t* bs)
{
	bot_goal_t goal;
	vec3_t target, dir;
	//bot_moveresult_t moveresult;
	float range;

	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "battle chase: observer");
	//	return qfalse;
	//}
	//
	////if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "battle chase: intermission");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "battle chase: bot dead");
		return qfalse;
	}

	//if no enemy
	if (bs->enemy < 0) {
		AIEnter_Seek_LTG(bs, "battle chase: no enemy");
		return qfalse;
	}

	//if the enemy is visible
	if (BotEntityVisibleTest(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy, qfalse)) {
		AIEnter_Battle_Fight(bs, "battle chase");
		return qfalse;
	}

	//if there is another enemy
	if (BotFindEnemy(bs, -1)) {
		AIEnter_Battle_Fight(bs, "battle chase: better enemy");
		return qfalse;
	}
	////there is no last enemy area
	//if (!bs->lastenemyareanum) {
	//	AIEnter_Seek_LTG(bs, "battle chase: no enemy area");
	//	return qfalse;
	//}
// jmarshall
	//
	//bs->tfl = TFL_DEFAULT;
	//if (bot_grapple.integer) bs->tfl |= TFL_GRAPPLEHOOK;
	////if in lava or slime the bot should be able to get out
	//if (BotInLavaOrSlime(bs)) bs->tfl |= TFL_LAVA | TFL_SLIME;
	////
	//if (BotCanAndWantsToRocketJump(bs)) {
	//	bs->tfl |= TFL_ROCKETJUMP;
	//}
	//map specific code
	//BotMapScripts(bs);
// jmarshall end

	//create the chase goal
	goal.entitynum = bs->enemy;
	goal.areanum = bs->lastenemyareanum;
	VectorCopy(bs->lastenemyorigin, goal.origin);
	VectorSet(goal.mins, -8, -8, -8);
	VectorSet(goal.maxs, 8, 8, 8);

// jmarshall - goal origin is last visible position
	{
		// Do a trace between the last_enemy_visible_position and the goal origin, 
		// if for some reason we don't have line of sight to it, switch to LTG.
		trace_t trace;

		trap_Trace(&trace, bs->last_enemy_visible_position, NULL, NULL, g_entities[bs->client].r.currentOrigin, 0, CONTENTS_SOLID);

		if (trace.fraction <= 0.9f)
		{
			AIEnter_Seek_LTG(bs, "can't see last enemy position");
			return qfalse;
		}
	}
	VectorCopy(bs->last_enemy_visible_position, goal.origin);
// jmarshall end

	//if the last seen enemy spot is reached the enemy could not be found
	if (BotTouchingGoal(bs->origin, &goal))
		bs->chase_time = 0;

	//if there's no chase time left
	if (!bs->chase_time || bs->chase_time < FloatTime() - 10) {
		AIEnter_Seek_LTG(bs, "battle chase: time out");
		return qfalse;
	}
	//check for nearby goals periodicly
	if (bs->check_time < FloatTime()) {
		bs->check_time = FloatTime() + 1;
		range = 150;
		//
		if (BotNearbyGoal(bs, bs->tfl, &goal, range)) {
			//the bot gets 5 seconds to pick up the nearby goal item
			bs->nbg_time = FloatTime() + 0.1 * range + 1;
			//BotResetLastAvoidReach(bs->ms);
			AIEnter_Battle_NBG(bs, "battle chase: nbg");
			return qfalse;
		}
	}

	BotUpdateBattleInventory(bs, bs->enemy);

	////initialize the movement state
	//BotSetupForMovement(bs);

	//move towards the goal
	//trap_BotMoveToGoal(&moveresult, bs->ms, &goal, bs->tfl);
	BotMoveToGoal(bs, &goal);

	//if the movement failed
	//if (moveresult.failure) {
	//	//reset the avoid reach, otherwise bot is stuck in current area
	//	trap_BotResetAvoidReach(bs->ms);
	//	//BotAI_Print(PRT_MESSAGE, "movement failure %d\n", moveresult.traveltype);
	//	bs->ltg_time = 0;
	//}
	////
	//BotAIBlocked(bs, &moveresult, qfalse);
	//
	//if (moveresult.flags & (MOVERESULT_MOVEMENTVIEWSET | MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW)) {
	//	VectorCopy(moveresult.ideal_viewangles, bs->ideal_viewangles);
	//}

	if (!(bs->flags & BFL_IDEALVIEWSET)) {
		BotAimAtEnemy(bs);
// jmarshall
		//if (bs->chase_time > FloatTime() - 2) {
		//	BotAimAtEnemy(bs);
		//}
		//else {
		//	if (BotMovementViewTarget(bs->ms, &goal, bs->tfl, 300, target)) {
		//		VectorSubtract(target, bs->origin, dir);
		//		vectoangles(dir, bs->viewangles);
		//	}
		//	//else {
		//	//	vectoangles(moveresult.movedir, bs->ideal_viewangles);
		//	//}
		//}
// jmarshall end
		bs->ideal_viewangles[2] *= 0.5;
	}
	//if the weapon is used for the bot movement
	//if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) bs->weaponnum = moveresult.weapon;
	//if the bot is in the area the enemy was last seen in
	//if (bs->areanum == bs->lastenemyareanum) bs->chase_time = 0;
	//if the bot wants to retreat (the bot could have been damage during the chase)
	if (BotWantsToRetreat(bs)) {
		AIEnter_Battle_Retreat(bs, "battle chase: wants to retreat");
		return qtrue;
	}
	return qtrue;
}

/*
==================
AIEnter_Battle_Chase
==================
*/
void AIEnter_Battle_Chase(bot_state_t* bs, char* s) {
	//BotRecordNodeSwitch(bs, "battle chase", "", s);
	bs->chase_time = FloatTime();
	bs->ainode = AINode_Battle_Chase;
}

/*
==================
AINode_Battle_Retreat
==================
*/
int AINode_Battle_Retreat(bot_state_t* bs) {
	bot_goal_t goal;
	//aas_entityinfo_t entinfo;
	gentity_t* entinfo;
	//bot_moveresult_t moveresult;
	vec3_t target, dir;
	float attack_skill, range;
	//int areanum;

	//if (BotIsObserver(bs)) {
	//	AIEnter_Observer(bs, "battle retreat: observer");
	//	return qfalse;
	//}

	//if in the intermission
	//if (BotIntermission(bs)) {
	//	AIEnter_Intermission(bs, "battle retreat: intermission");
	//	return qfalse;
	//}

	//respawn if dead
	if (BotIsDead(bs)) {
		AIEnter_Respawn(bs, "battle retreat: bot dead");
		return qfalse;
	}

	//if no enemy
	if (bs->enemy < 0) {
		AIEnter_Seek_LTG(bs, "battle retreat: no enemy");
		return qfalse;
	}

	//BotEntityInfo(bs->enemy, &entinfo);
	entinfo = &g_entities[bs->enemy];

	// If our enemy is dead, search for another LTG.
	if (EntityIsDead(entinfo)) {
		AIEnter_Seek_LTG(bs, "battle retreat: enemy dead");
		return qfalse;
	}

	//if there is another better enemy
	if (BotFindEnemy(bs, bs->enemy)) {
		G_Printf("found new better enemy\n");
	}

// jmarshall: if we are retreating try to jump around, become harder to hit. 
	if (FRandRange(0.0, 4.0) <= 2.0)
	{
		bs->input.actionflags |= ACTION_JUMP;
	}
// jmarshall end

	//
	//bs->tfl = TFL_DEFAULT;
	//if (bot_grapple.integer) bs->tfl |= TFL_GRAPPLEHOOK;
	////if in lava or slime the bot should be able to get out
	//if (BotInLavaOrSlime(bs)) bs->tfl |= TFL_LAVA | TFL_SLIME;
	////map specific code
	//BotMapScripts(bs);

	//update the attack inventory values
	BotUpdateBattleInventory(bs, bs->enemy);

	//if the bot doesn't want to retreat anymore... probably picked up some nice items
	if (BotWantsToChase(bs)) {
		//empty the goal stack, when chasing, only the enemy is the goal
		BotEmptyGoalStack(bs->gs);

		//go chase the enemy
		AIEnter_Battle_Chase(bs, "battle retreat: wants to chase");
		return qfalse;
	}

	//update the last time the enemy was visible
	if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy)) {
		bs->enemyvisible_time = FloatTime();
		VectorCopy(entinfo->r.currentOrigin, target);

		// if not a player enemy
		if (bs->enemy >= MAX_CLIENTS) {
#ifdef MISSIONPACK
			// if attacking an obelisk
			if (bs->enemy == redobelisk.entitynum ||
				bs->enemy == blueobelisk.entitynum) {
				target[2] += 16;
			}
#endif
		}

		VectorCopy(target, bs->lastenemyorigin);
	}

	//if the enemy is NOT visible for 4 seconds
	if (bs->enemyvisible_time < FloatTime() - 4) {
		AIEnter_Seek_LTG(bs, "battle retreat: lost enemy");
		return qfalse;
	}
	//else if the enemy is NOT visible
	else if (bs->enemyvisible_time < FloatTime()) {
		//if there is another enemy
		if (BotFindEnemy(bs, -1)) {
			AIEnter_Battle_Fight(bs, "battle retreat: another enemy");
			return qfalse;
		}
	}
	//
	//BotTeamGoals(bs, qtrue);
	
	//use holdable items
	BotBattleUseItems(bs);
	
	//get the current long term goal while retreating
	if (!BotLongTermGoal(bs, bs->tfl, qtrue, &goal)) {
		AIEnter_Battle_SuicidalFight(bs, "battle retreat: no way out");
		return qfalse;
	}
	//check for nearby goals periodicly
	if (bs->check_time < FloatTime()) {
		bs->check_time = FloatTime() + 1;
		range = 150;
#ifdef CTF
		//if (gametype == GT_CTF) {
		//	//if carrying a flag the bot shouldn't be distracted too much
		//	if (BotCTFCarryingFlag(bs))
		//		range = 50;
		//}
#endif //CTF
#ifdef MISSIONPACK
		else if (gametype == GT_1FCTF) {
			if (Bot1FCTFCarryingFlag(bs))
				range = 50;
		}
		else if (gametype == GT_HARVESTER) {
			if (BotHarvesterCarryingCubes(bs))
				range = 80;
		}
#endif
		//
		if (BotNearbyGoal(bs, bs->tfl, &goal, range)) {
			//trap_BotResetLastAvoidReach(bs->ms);
			//time the bot gets to pick up the nearby goal item
			bs->nbg_time = FloatTime() + range / 100 + 1;
			AIEnter_Battle_NBG(bs, "battle retreat: nbg");
			return qfalse;
		}
	}
	//initialize the movement state
	//BotSetupForMovement(bs);
	//move towards the goal
	//trap_BotMoveToGoal(&moveresult, bs->ms, &goal, bs->tfl);
	BotMoveToGoal(bs, &goal);

	//if the movement failed
	//if (moveresult.failure) {
	//	//reset the avoid reach, otherwise bot is stuck in current area
	//	trap_BotResetAvoidReach(bs->ms);
	//	//BotAI_Print(PRT_MESSAGE, "movement failure %d\n", moveresult.traveltype);
	//	bs->ltg_time = 0;
	//}
	//
	//BotAIBlocked(bs, &moveresult, qfalse);
	//choose the best weapon to fight with
	BotChooseWeapon(bs);
	//if the view is fixed for the movement
	//if (moveresult.flags & (MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW)) {
	//	VectorCopy(moveresult.ideal_viewangles, bs->ideal_viewangles);
	//}
	//else if (!(moveresult.flags & MOVERESULT_MOVEMENTVIEWSET)
	//	&& !(bs->flags & BFL_IDEALVIEWSET)) {
	//	attack_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ATTACK_SKILL, 0, 1);
	//	//if the bot is skilled anough
	//	if (attack_skill > 0.3) {
	//		BotAimAtEnemy(bs);
	//	}
	//	else {
	//		if (trap_BotMovementViewTarget(bs->ms, &goal, bs->tfl, 300, target)) {
	//			VectorSubtract(target, bs->origin, dir);
	//			vectoangles(dir, bs->ideal_viewangles);
	//		}
	//		else {
	//			vectoangles(moveresult.movedir, bs->ideal_viewangles);
	//		}
	//		bs->ideal_viewangles[2] *= 0.5;
	//	}
	//}
	////if the weapon is used for the bot movement
	//if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) bs->weaponnum = moveresult.weapon;
	BotAimAtEnemy(bs);

	//attack the enemy if possible
	BotCheckAttack(bs);
	//
	return qtrue;
}


/*
==================
AIEnter_Battle_Retreat
==================
*/
void AIEnter_Battle_Retreat(bot_state_t* bs, char* s) {
	//	BotRecordNodeSwitch(bs, "battle retreat", "", s);
	bs->ainode = AINode_Battle_Retreat;
}
