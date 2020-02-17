// BotAI.c
//

#include "../q_shared.h"
#include "../g_local.h"
#include "../chars.h"
#include "../inv.h"
#include "BotAI.h"

/*
==================
BotUpdateInventory
==================
*/
void BotUpdateInventory(bot_state_t* bs) {
	int oldinventory[MAX_ITEMS];

	memcpy(oldinventory, bs->inventory, sizeof(oldinventory));
	//armor
	bs->inventory[INVENTORY_ARMOR] = bs->cur_ps.stats[STAT_ARMOR];
	//weapons
	bs->inventory[INVENTORY_GAUNTLET] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_GAUNTLET)) != 0;
	bs->inventory[INVENTORY_SHOTGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_SHOTGUN)) != 0;
	bs->inventory[INVENTORY_MACHINEGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_MACHINEGUN)) != 0;
	bs->inventory[INVENTORY_GRENADELAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_GRENADE_LAUNCHER)) != 0;
	bs->inventory[INVENTORY_ROCKETLAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER)) != 0;
	bs->inventory[INVENTORY_LIGHTNING] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_LIGHTNING)) != 0;
	bs->inventory[INVENTORY_RAILGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN)) != 0;
	bs->inventory[INVENTORY_PLASMAGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_PLASMAGUN)) != 0;
	bs->inventory[INVENTORY_BFG10K] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_BFG)) != 0;
	bs->inventory[INVENTORY_GRAPPLINGHOOK] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_GRAPPLING_HOOK)) != 0;
#ifdef MISSIONPACK
	bs->inventory[INVENTORY_NAILGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_NAILGUN)) != 0;;
	bs->inventory[INVENTORY_PROXLAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_PROX_LAUNCHER)) != 0;;
	bs->inventory[INVENTORY_CHAINGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_CHAINGUN)) != 0;;
#endif
	//ammo
	bs->inventory[INVENTORY_SHELLS] = bs->cur_ps.ammo[WP_SHOTGUN];
	bs->inventory[INVENTORY_BULLETS] = bs->cur_ps.ammo[WP_MACHINEGUN];
	bs->inventory[INVENTORY_GRENADES] = bs->cur_ps.ammo[WP_GRENADE_LAUNCHER];
	bs->inventory[INVENTORY_CELLS] = bs->cur_ps.ammo[WP_PLASMAGUN];
	bs->inventory[INVENTORY_LIGHTNINGAMMO] = bs->cur_ps.ammo[WP_LIGHTNING];
	bs->inventory[INVENTORY_ROCKETS] = bs->cur_ps.ammo[WP_ROCKET_LAUNCHER];
	bs->inventory[INVENTORY_SLUGS] = bs->cur_ps.ammo[WP_RAILGUN];
	bs->inventory[INVENTORY_BFGAMMO] = bs->cur_ps.ammo[WP_BFG];
#ifdef MISSIONPACK
	bs->inventory[INVENTORY_NAILS] = bs->cur_ps.ammo[WP_NAILGUN];
	bs->inventory[INVENTORY_MINES] = bs->cur_ps.ammo[WP_PROX_LAUNCHER];
	bs->inventory[INVENTORY_BELT] = bs->cur_ps.ammo[WP_CHAINGUN];
#endif
	//powerups
	bs->inventory[INVENTORY_HEALTH] = bs->cur_ps.stats[STAT_HEALTH];
	bs->inventory[INVENTORY_TELEPORTER] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_TELEPORTER;
	bs->inventory[INVENTORY_MEDKIT] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_MEDKIT;
#ifdef MISSIONPACK
	bs->inventory[INVENTORY_KAMIKAZE] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_KAMIKAZE;
	bs->inventory[INVENTORY_PORTAL] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_PORTAL;
	bs->inventory[INVENTORY_INVULNERABILITY] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_INVULNERABILITY;
#endif
	bs->inventory[INVENTORY_QUAD] = bs->cur_ps.powerups[PW_QUAD] != 0;
	bs->inventory[INVENTORY_ENVIRONMENTSUIT] = bs->cur_ps.powerups[PW_BATTLESUIT] != 0;
	bs->inventory[INVENTORY_HASTE] = bs->cur_ps.powerups[PW_HASTE] != 0;
	bs->inventory[INVENTORY_INVISIBILITY] = bs->cur_ps.powerups[PW_INVIS] != 0;
	bs->inventory[INVENTORY_REGEN] = bs->cur_ps.powerups[PW_REGEN] != 0;
	bs->inventory[INVENTORY_FLIGHT] = bs->cur_ps.powerups[PW_FLIGHT] != 0;
#ifdef MISSIONPACK
	bs->inventory[INVENTORY_SCOUT] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_SCOUT;
	bs->inventory[INVENTORY_GUARD] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_GUARD;
	bs->inventory[INVENTORY_DOUBLER] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_DOUBLER;
	bs->inventory[INVENTORY_AMMOREGEN] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_AMMOREGEN;
#endif
	bs->inventory[INVENTORY_REDFLAG] = bs->cur_ps.powerups[PW_REDFLAG] != 0;
	bs->inventory[INVENTORY_BLUEFLAG] = bs->cur_ps.powerups[PW_BLUEFLAG] != 0;
#ifdef MISSIONPACK
	bs->inventory[INVENTORY_NEUTRALFLAG] = bs->cur_ps.powerups[PW_NEUTRALFLAG] != 0;
	if (BotTeam(bs) == TEAM_RED) {
		bs->inventory[INVENTORY_REDCUBE] = bs->cur_ps.generic1;
		bs->inventory[INVENTORY_BLUECUBE] = 0;
	}
	else {
		bs->inventory[INVENTORY_REDCUBE] = 0;
		bs->inventory[INVENTORY_BLUECUBE] = bs->cur_ps.generic1;
	}
	BotCheckItemPickup(bs, oldinventory);
#endif
}

/*
==================
BotNearbyGoal
==================
*/
int BotNearbyGoal(bot_state_t * bs, int tfl, bot_goal_t * ltg, float range) {
	int ret;

// jmarshall - check for air.
	//check if the bot should go for air
	//if (BotGoForAir(bs, tfl, ltg, range)) return qtrue;
	////if the bot is carrying the enemy flag
	//if (BotCTFCarryingFlag(bs)) {
	//	//if the bot is just a few secs away from the base 
	//	if (trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin,
	//		bs->teamgoal.areanum, TFL_DEFAULT) < 300) {
	//		//make the range really small
	//		range = 50;
	//	}
	//}
// jmarshall end

	//
	ret = BotChooseNBGItem(bs->gs, bs->origin, bs->inventory, tfl, ltg, range);
	/*
	if (ret)
	{
		char buf[128];
		//get the goal at the top of the stack
		trap_BotGetTopGoal(bs->gs, &goal);
		trap_BotGoalName(goal.number, buf, sizeof(buf));
		BotAI_Print(PRT_MESSAGE, "%1.1f: new nearby goal %s\n", FloatTime(), buf);
	}
	*/
	return ret;
}


/*
==================
BotCheckAttack
==================
*/
void BotCheckAttack(bot_state_t* bs) {
	float points, reactiontime, fov, firethrottle;
	int attackentity;
	trace_t bsptrace;
	//float selfpreservation;
	vec3_t forward, right, start, end, dir, angles;
	weaponinfo_t wi;
	trace_t trace;
	//aas_entityinfo_t entinfo;
	gentity_t* entinfo;
	vec3_t mins = { -8, -8, -8 }, maxs = { 8, 8, 8 };

	attackentity = bs->enemy;
	//
	//BotEntityInfo(attackentity, &entinfo);
	entinfo = &g_entities[attackentity];

	// if not attacking a player
	if (attackentity >= MAX_CLIENTS) {
#ifdef MISSIONPACK
		// if attacking an obelisk
		if (entinfo.number == redobelisk.entitynum ||
			entinfo.number == blueobelisk.entitynum) {
			// if obelisk is respawning return
			if (g_entities[entinfo.number].activator &&
				g_entities[entinfo.number].activator->s.frame == 2) {
				return;
			}
		}
#endif
	}
	//
	reactiontime = Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);
	if (bs->enemysight_time > FloatTime() - reactiontime) 
		return;
	if (bs->teleport_time > FloatTime() - reactiontime) 
		return;
	
	//if changing weapons
	if (bs->weaponchange_time > FloatTime() - 0.1) 
		return;

	//check fire throttle characteristic
	if (bs->firethrottlewait_time > FloatTime()) 
		return;

	firethrottle = Characteristic_BFloat(bs->character, CHARACTERISTIC_FIRETHROTTLE, 0, 1);
	if (bs->firethrottleshoot_time < FloatTime()) {
		if (random() > firethrottle) {
			bs->firethrottlewait_time = FloatTime() + firethrottle;
			bs->firethrottleshoot_time = 0;
		}
		else {
			bs->firethrottleshoot_time = FloatTime() + 1 - firethrottle;
			bs->firethrottlewait_time = 0;
		}
	}
	
	VectorSubtract(bs->aimtarget, bs->eye, dir);
	
	if (bs->weaponnum == WP_GAUNTLET) {
		if (VectorLengthSquared(dir) > Square(60)) {
			return;
		}
	}
	if (VectorLengthSquared(dir) < Square(100))
		fov = 120;
	else
		fov = 50;
	
	vectoangles(dir, angles);
	if (!InFieldOfVision(bs->viewangles, fov, angles))
		return;

	trap_Trace(&bsptrace, bs->eye, NULL, NULL, bs->aimtarget, bs->client, CONTENTS_SOLID | CONTENTS_PLAYERCLIP);
	if (bsptrace.fraction < 1 && bsptrace.entityNum != attackentity)
		return;

	//get the weapon info
	BotGetWeaponInfo(bs->ws, bs->weaponnum, &wi);

	//get the start point shooting from
	VectorCopy(bs->origin, start);
	start[2] += bs->cur_ps.viewheight;
	AngleVectors(bs->viewangles, forward, right, NULL);
	start[0] += forward[0] * wi.offset[0] + right[0] * wi.offset[1];
	start[1] += forward[1] * wi.offset[0] + right[1] * wi.offset[1];
	start[2] += forward[2] * wi.offset[0] + right[2] * wi.offset[1] + wi.offset[2];
	//end point aiming at
	VectorMA(start, 1000, forward, end);
	//a little back to make sure not inside a very close enemy
	VectorMA(start, -12, forward, start);
	trap_Trace(&trace, start, mins, maxs, end, bs->entitynum, MASK_SHOT);

	//if the entity is a client
	if (trace.entityNum > 0 && trace.entityNum <= MAX_CLIENTS) {
		if (trace.entityNum != attackentity) {
// jmarshall - teams
			//if a teammate is hit
			//if (BotSameTeam(bs, trace.entityNum))
			//	return;
// jmarshall end
		}
	}
	//if won't hit the enemy or not attacking a player (obelisk)
	if (trace.entityNum != attackentity || attackentity >= MAX_CLIENTS) {
		//if the projectile does radial damage
		if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
			if (trace.fraction * 1000 < wi.proj.radius) {
				points = (wi.proj.damage - 0.5 * trace.fraction * 1000) * 0.5;
				if (points > 0) {
					return;
				}
			}
			//FIXME: check if a teammate gets radial damage
		}
	}
	//if fire has to be release to activate weapon
	if (wi.flags & WFL_FIRERELEASED) {
		if (bs->flags & BFL_ATTACKED) {
			//trap_EA_Attack(bs->client);
			bs->input.actionflags |= ACTION_ATTACK;
		}
	}
	else {
		//trap_EA_Attack(bs->client);
		bs->input.actionflags |= ACTION_ATTACK;
	}
	bs->flags ^= BFL_ATTACKED;
}

/*
==================
BotAimAtEnemy
==================
*/
void BotAimAtEnemy(bot_state_t* bs) {
	int i, enemyvisible;
	float dist, f, aim_skill, aim_accuracy, speed, reactiontime;
	vec3_t dir, bestorigin, end, start, groundtarget, cmdmove, enemyvelocity;
	vec3_t mins = { -4,-4,-4 }, maxs = { 4, 4, 4 };
	weaponinfo_t wi;
	//aas_entityinfo_t entinfo;
	gentity_t* entinfo;
	bot_goal_t goal;
	trace_t trace;
	vec3_t target;

	//if the bot has no enemy
	if (bs->enemy < 0) {
		return;
	}

	//get the enemy entity information
	//BotEntityInfo(bs->enemy, &entinfo);
	entinfo = &g_entities[bs->enemy];

	//if this is not a player (should be an obelisk)
	if (bs->enemy >= MAX_CLIENTS) {
		//if the obelisk is visible
		VectorCopy(entinfo->r.currentOrigin, target);
#ifdef MISSIONPACK
		// if attacking an obelisk
		if (bs->enemy == redobelisk.entitynum ||
			bs->enemy == blueobelisk.entitynum) {
			target[2] += 32;
		}
#endif
		//aim at the obelisk
		VectorSubtract(target, bs->eye, dir);
		vectoangles(dir, bs->ideal_viewangles);
		//set the aim target before trying to attack
		VectorCopy(target, bs->aimtarget);
		return;
	}

	//
	//BotAI_Print(PRT_MESSAGE, "client %d: aiming at client %d\n", bs->entitynum, bs->enemy);
	//
	aim_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
	aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
	//
	if (aim_skill > 0.95) {
		//don't aim too early
		reactiontime = 0.5 * Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);
		if (bs->enemysight_time > FloatTime() - reactiontime) 
			return;
		if (bs->teleport_time > FloatTime() - reactiontime) 
			return;
	}

	//get the weapon information
	BotGetWeaponInfo(bs->ws, bs->weaponnum, &wi);

	//get the weapon specific aim accuracy and or aim skill
	if (wi.number == WP_MACHINEGUN) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MACHINEGUN, 0, 1);
	}
	else if (wi.number == WP_SHOTGUN) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_SHOTGUN, 0, 1);
	}
	else if (wi.number == WP_GRENADE_LAUNCHER) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_GRENADELAUNCHER, 0, 1);
		aim_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_GRENADELAUNCHER, 0, 1);
	}
	else if (wi.number == WP_ROCKET_LAUNCHER) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_ROCKETLAUNCHER, 0, 1);
		aim_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_ROCKETLAUNCHER, 0, 1);
	}
	else if (wi.number == WP_LIGHTNING) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_LIGHTNING, 0, 1);
	}
	else if (wi.number == WP_RAILGUN) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_RAILGUN, 0, 1);
	}
	else if (wi.number == WP_PLASMAGUN) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PLASMAGUN, 0, 1);
		aim_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_PLASMAGUN, 0, 1);
	}
	else if (wi.number == WP_BFG) {
		aim_accuracy = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BFG10K, 0, 1);
		aim_skill = Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_BFG10K, 0, 1);
	}
	//
	if (aim_accuracy <= 0) 
		aim_accuracy = 0.0001f;
	//get the enemy entity information
	//BotEntityInfo(bs->enemy, &entinfo);
	entinfo = &g_entities[bs->enemy];

	//if the enemy is invisible then shoot crappy most of the time
	if (EntityIsInvisible(entinfo)) {
		if (random() > 0.1) 
			aim_accuracy *= 0.4f;
	}
// jmarshall - fix aim accuracy.
	//VectorSubtract(entinfo->r.currentOrigin, entinfo.lastvisorigin, enemyvelocity);
	//VectorScale(enemyvelocity, 1 / entinfo.update_time, enemyvelocity);
	////enemy origin and velocity is remembered every 0.5 seconds
	//if (bs->enemyposition_time < FloatTime()) {
	//	//
	//	bs->enemyposition_time = FloatTime() + 0.5;
	//	VectorCopy(enemyvelocity, bs->enemyvelocity);
	//	VectorCopy(entinfo.origin, bs->enemyorigin);
	//}
	////if not extremely skilled
	//if (aim_skill < 0.9) {
	//	VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
	//	//if the enemy moved a bit
	//	if (VectorLengthSquared(dir) > Square(48)) {
	//		//if the enemy changed direction
	//		if (DotProduct(bs->enemyvelocity, enemyvelocity) < 0) {
	//			//aim accuracy should be worse now
	//			aim_accuracy *= 0.7f;
	//		}
	//	}
	//}
// jmarshall end
	
	//check visibility of enemy
	enemyvisible = BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy);
	
	//if the enemy is visible
	if (enemyvisible) {
		//
		VectorCopy(entinfo->r.currentOrigin, bestorigin);
		bestorigin[2] += 8;
		//get the start point shooting from
		//NOTE: the x and y projectile start offsets are ignored
		VectorCopy(bs->origin, start);
		start[2] += bs->cur_ps.viewheight;
		start[2] += wi.offset[2];
		//
		trap_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, MASK_SHOT);
		//if the enemy is NOT hit
		if (trace.fraction <= 1 && trace.entityNum != bs->enemy) {
			bestorigin[2] += 16;
		}
		//if it is not an instant hit weapon the bot might want to predict the enemy
		if (wi.speed) {
			//
			VectorSubtract(bestorigin, bs->origin, dir);
			dist = VectorLength(dir);
			VectorSubtract(entinfo->r.currentOrigin, bs->enemyorigin, dir);
			//if the enemy is NOT pretty far away and strafing just small steps left and right
			if (!(dist > 100 && VectorLengthSquared(dir) < Square(32))) {
				//if skilled anough do exact prediction
				if (aim_skill > 0.8 &&
					//if the weapon is ready to fire
					bs->cur_ps.weaponstate == WEAPON_READY) {
					//aas_clientmove_t move;
					//vec3_t origin;
					//
					//VectorSubtract(entinfo.origin, bs->origin, dir);
					////distance towards the enemy
					//dist = VectorLength(dir);
					////direction the enemy is moving in
					//VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					////
					//VectorScale(dir, 1 / entinfo.update_time, dir);
					////
					//VectorCopy(entinfo.origin, origin);
					//origin[2] += 1;
					////
					//VectorClear(cmdmove);
					////AAS_ClearShownDebugLines();
					//trap_AAS_PredictClientMovement(&move, bs->enemy, origin,
					//	PRESENCE_CROUCH, qfalse,
					//	dir, cmdmove, 0,
					//	dist * 10 / wi.speed, 0.1f, 0, 0, qfalse);
					//VectorCopy(move.endpos, bestorigin);
					//BotAI_Print(PRT_MESSAGE, "%1.1f predicted speed = %f, frames = %f\n", FloatTime(), VectorLength(dir), dist * 10 / wi.speed);
					
					bot_goal_t goal;
					VectorCopy(entinfo->r.currentOrigin, goal.origin);
					BotMoveToGoal(bs, &goal);
				}
				//if not that skilled do linear prediction
				else if (aim_skill > 0.4) {
// jmarshall - fix linear prediction.
					//VectorSubtract(entinfo->r.currentOrigin, bs->origin, dir);
					////distance towards the enemy
					//dist = VectorLength(dir);
					////direction the enemy is moving in
					////VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					//dir[2] = 0;
					////
					//speed = VectorNormalize(dir) / entinfo.update_time;
					////botimport.Print(PRT_MESSAGE, "speed = %f, wi->speed = %f\n", speed, wi->speed);
					////best spot to aim at
					//VectorMA(entinfo.origin, (dist / wi.speed) * speed, dir, bestorigin);
					bot_goal_t goal;
					VectorCopy(entinfo->r.currentOrigin, goal.origin);
					BotMoveToGoal(bs, &goal);
// jmarshall end
				}
			}
		}
		//if the projectile does radial damage
		if (aim_skill > 0.6 && wi.proj.damagetype & DAMAGETYPE_RADIAL) {
			//if the enemy isn't standing significantly higher than the bot
			if (entinfo->r.currentOrigin[2] < bs->origin[2] + 16) {
				//try to aim at the ground in front of the enemy
				VectorCopy(entinfo->r.currentOrigin, end);
				end[2] -= 64;
				trap_Trace(&trace, entinfo->r.currentOrigin, NULL, NULL, end, bs->enemy, MASK_SHOT);
				//
				VectorCopy(bestorigin, groundtarget);
				if (trace.startsolid) 
					groundtarget[2] = entinfo->r.currentOrigin[2] - 16;
				else 
					groundtarget[2] = trace.endpos[2] - 8;
				//trace a line from projectile start to ground target
				trap_Trace(&trace, start, NULL, NULL, groundtarget, bs->entitynum, MASK_SHOT);
				//if hitpoint is not vertically too far from the ground target
				if (fabs(trace.endpos[2] - groundtarget[2]) < 50) {
					VectorSubtract(trace.endpos, groundtarget, dir);
					//if the hitpoint is near anough the ground target
					if (VectorLengthSquared(dir) < Square(60)) {
						VectorSubtract(trace.endpos, start, dir);
						//if the hitpoint is far anough from the bot
						if (VectorLengthSquared(dir) > Square(100)) {
							//check if the bot is visible from the ground target
							trace.endpos[2] += 1;
							trap_Trace(&trace, trace.endpos, NULL, NULL, entinfo->r.currentOrigin, bs->enemy, MASK_SHOT);
							if (trace.fraction >= 1) {
								//botimport.Print(PRT_MESSAGE, "%1.1f aiming at ground\n", AAS_Time());
								VectorCopy(groundtarget, bestorigin);
							}
						}
					}
				}
			}
		}
		bestorigin[0] += 20 * crandom() * (1 - aim_accuracy);
		bestorigin[1] += 20 * crandom() * (1 - aim_accuracy);
		bestorigin[2] += 10 * crandom() * (1 - aim_accuracy);
	}
	else {
		//
		VectorCopy(bs->lastenemyorigin, bestorigin);
		bestorigin[2] += 8;
// jmarshall - fix this.
		//if the bot is skilled anough
		//if (aim_skill > 0.5) {
		//	//do prediction shots around corners
		//	if (wi.number == WP_BFG ||
		//		wi.number == WP_ROCKET_LAUNCHER ||
		//		wi.number == WP_GRENADE_LAUNCHER) {
		//		//create the chase goal
		//		goal.entitynum = bs->client;
		//		goal.areanum = bs->areanum;
		//		VectorCopy(bs->eye, goal.origin);
		//		VectorSet(goal.mins, -8, -8, -8);
		//		VectorSet(goal.maxs, 8, 8, 8);
		//		//
		//		if (trap_BotPredictVisiblePosition(bs->lastenemyorigin, bs->lastenemyareanum, &goal, TFL_DEFAULT, target)) {
		//			VectorSubtract(target, bs->eye, dir);
		//			if (VectorLengthSquared(dir) > Square(80)) {
		//				VectorCopy(target, bestorigin);
		//				bestorigin[2] -= 20;
		//			}
		//		}
		//		aim_accuracy = 1;
		//	}
		//}
// jmarshall end
	}
	//
	if (enemyvisible) {
		trap_Trace(&trace, bs->eye, NULL, NULL, bestorigin, bs->entitynum, MASK_SHOT);
		VectorCopy(trace.endpos, bs->aimtarget);
	}
	else {
		VectorCopy(bestorigin, bs->aimtarget);
	}
	//get aim direction
	VectorSubtract(bestorigin, bs->eye, dir);
	//
	if (wi.number == WP_MACHINEGUN ||
		wi.number == WP_SHOTGUN ||
		wi.number == WP_LIGHTNING ||
		wi.number == WP_RAILGUN) {
		//distance towards the enemy
		dist = VectorLength(dir);
		if (dist > 150) dist = 150;
		f = 0.6 + dist / 150 * 0.4;
		aim_accuracy *= f;
	}
	//add some random stuff to the aim direction depending on the aim accuracy
	if (aim_accuracy < 0.8) {
		VectorNormalize(dir);
		for (i = 0; i < 3; i++) dir[i] += 0.3 * crandom() * (1 - aim_accuracy);
	}
	//set the ideal view angles
	vectoangles(dir, bs->ideal_viewangles);
	//take the weapon spread into account for lower skilled bots
	bs->ideal_viewangles[PITCH] += 6 * wi.vspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[PITCH] = AngleMod(bs->ideal_viewangles[PITCH]);
	bs->ideal_viewangles[YAW] += 6 * wi.hspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[YAW] = AngleMod(bs->ideal_viewangles[YAW]);
// jmarshall - add bot_challenge.
	//if the bots should be really challenging
	//if (bot_challenge.integer) {
	//	//if the bot is really accurate and has the enemy in view for some time
	//	if (aim_accuracy > 0.9 && bs->enemysight_time < FloatTime() - 1) {
	//		//set the view angles directly
	//		if (bs->ideal_viewangles[PITCH] > 180) bs->ideal_viewangles[PITCH] -= 360;
	//		VectorCopy(bs->ideal_viewangles, bs->viewangles);
	//		trap_EA_View(bs->client, bs->viewangles);
	//	}
	//}
// jmarshall end
}


/*
==================
BotUpdateBattleInventory
==================
*/
void BotUpdateBattleInventory(bot_state_t* bs, int enemy) {
	vec3_t dir;
	gentity_t *entinfo;

	entinfo = &g_entities[enemy];

	VectorSubtract(entinfo->r.currentOrigin, bs->origin, dir);
	bs->inventory[ENEMY_HEIGHT] = (int)dir[2];
	dir[2] = 0;
	bs->inventory[ENEMY_HORIZONTAL_DIST] = (int)VectorLength(dir);
	//FIXME: add num visible enemies and num visible team mates to the inventory
}

/*
==================
InFieldOfVision
==================
*/
qboolean InFieldOfVision(vec3_t viewangles, float fov, vec3_t angles)
{
	int i;
	float diff, angle;

	for (i = 0; i < 2; i++) {
		angle = AngleMod(viewangles[i]);
		angles[i] = AngleMod(angles[i]);
		diff = angles[i] - angle;
		if (angles[i] > angle) {
			if (diff > 180.0) diff -= 360.0;
		}
		else {
			if (diff < -180.0) diff += 360.0;
		}
		if (diff > 0) {
			if (diff > fov * 0.5) 
				return qfalse;
		}
		else {
			if (diff < -fov * 0.5) 
				return qfalse;
		}
	}
	return qtrue;
}


/*
==================
EntityIsInvisible
==================
*/
qboolean EntityIsInvisible(gentity_t* ent) {
	// the flag is always visible
	//if (EntityCarriesFlag(entinfo)) {
	//	return qfalse;
	//}

	if (ent->client->ps.powerups[PW_INVIS]) {
		return qtrue;
	}

	return qfalse;
}

/*
==================
EntityIsDead
==================
*/
qboolean EntityIsDead(gentity_t *ent) {
	playerState_t ps;

	if(ent->inuse && ent->client != NULL)
	{
		if (ent->client->ps.pm_type != PM_NORMAL) 
			return qtrue;
	}
	
	return qfalse;
}


/*
==================
EntityIsShooting
==================
*/
qboolean EntityIsShooting(gentity_t *ent) {
	if (ent->inuse && ent->client != NULL)
	{
		if (ent->client->ps.eFlags & EF_FIRING) {
			return qtrue;
		}
	}
	return qfalse;
}


/*
==================
BotEntityVisible

returns visibility in the range [0, 1] taking fog and water surfaces into account
==================
*/
float BotEntityVisible(int viewer, vec3_t eye, vec3_t viewangles, float fov, int ent) {
	int i, contents_mask, passent, hitent, infog, inwater, otherinfog, pc;
	float squaredfogdist, waterfactor, vis, bestvis;
	trace_t trace;
	gentity_t *entinfo;
	vec3_t dir, entangles, start, end, middle;

	//calculate middle of bounding box
	//BotEntityInfo(ent, &entinfo);
	entinfo = &g_entities[ent];

	VectorAdd(entinfo->r.mins, entinfo->r.maxs, middle);
	VectorScale(middle, 0.5, middle);
	VectorAdd(entinfo->r.currentOrigin, middle, middle);
	
	//check if entity is within field of vision
	VectorSubtract(middle, eye, dir);
	vectoangles(dir, entangles);
	if (!InFieldOfVision(viewangles, fov, entangles)) 
		return 0;

	pc = trap_PointContents(eye, ent);
	infog = (pc & CONTENTS_FOG);
	inwater = (pc & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER));

	bestvis = 0;
	for (i = 0; i < 3; i++) {
		//if the point is not in potential visible sight
		//if (!AAS_inPVS(eye, middle)) continue;
		//
		contents_mask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
		passent = viewer;
		hitent = ent;
		VectorCopy(eye, start);
		VectorCopy(middle, end);
		//if the entity is in water, lava or slime
		if (trap_PointContents(middle, ent) & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER)) {
			contents_mask |= (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER);
		}
		//if eye is in water, lava or slime
		if (inwater) {
			if (!(contents_mask & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER))) {
				passent = ent;
				hitent = viewer;
				VectorCopy(middle, start);
				VectorCopy(eye, end);
			}
			contents_mask ^= (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER);
		}

		//trace from start to end
		trap_Trace(&trace, start, NULL, NULL, end, passent, contents_mask);
		
		//if water was hit
		waterfactor = 1.0;
		if (trace.contents & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER)) {
			//if the water surface is translucent
			if (1) {
				//trace through the water
				contents_mask &= ~(CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER);
				trap_Trace(&trace, trace.endpos, NULL, NULL, end, passent, contents_mask);
				waterfactor = 0.5;
			}
		}

		//if a full trace or the hitent was hit
		if (trace.fraction >= 1 || trace.entityNum == hitent) {
			//check for fog, assuming there's only one fog brush where
			//either the viewer or the entity is in or both are in
			otherinfog = (trap_PointContents(middle, ent) & CONTENTS_FOG);
			if (infog && otherinfog) {
				VectorSubtract(trace.endpos, eye, dir);
				squaredfogdist = VectorLengthSquared(dir);
			}
			else if (infog) {
				VectorCopy(trace.endpos, start);
				trap_Trace(&trace, start, NULL, NULL, eye, viewer, CONTENTS_FOG);
				VectorSubtract(eye, trace.endpos, dir);
				squaredfogdist = VectorLengthSquared(dir);
			}
			else if (otherinfog) {
				VectorCopy(trace.endpos, end);
				trap_Trace(&trace, eye, NULL, NULL, end, viewer, CONTENTS_FOG);
				VectorSubtract(end, trace.endpos, dir);
				squaredfogdist = VectorLengthSquared(dir);
			}
			else {
				//if the entity and the viewer are not in fog assume there's no fog in between
				squaredfogdist = 0;
			}
			//decrease visibility with the view distance through fog
			vis = 1 / ((squaredfogdist * 0.001) < 1 ? 1 : (squaredfogdist * 0.001));
			
			//if entering water visibility is reduced
			vis *= waterfactor;
			
			if (vis > bestvis) bestvis = vis;
			
			//if pretty much no fog
			if (bestvis >= 0.95) 
				return bestvis;
		}
		//check bottom and top of bounding box as well
		if (i == 0) 
			middle[2] += entinfo->r.mins[2];
		else if (i == 1) 
			middle[2] += entinfo->r.maxs[2] - entinfo->r.mins[2];
	}
	return bestvis;
}


/*
==================
BotFindEnemy
==================
*/
int BotFindEnemy(bot_state_t* bs, int curenemy) {
	int i, healthdecrease;
	float f, alertness, easyfragger, vis;
	float squaredist, cursquaredist;
	gentity_t *entinfo, *curenemyinfo;
	vec3_t dir, angles;

	alertness = Characteristic_BFloat(bs->character, CHARACTERISTIC_ALERTNESS, 0, 1);
	easyfragger = Characteristic_BFloat(bs->character, CHARACTERISTIC_EASY_FRAGGER, 0, 1);
	
	//check if the health decreased
	healthdecrease = bs->lasthealth > bs->inventory[INVENTORY_HEALTH];
	
	//remember the current health value
	bs->lasthealth = bs->inventory[INVENTORY_HEALTH];
	//
	if (curenemy >= 0) {
		//BotEntityInfo(curenemy, &curenemyinfo);
		curenemyinfo = &g_entities[curenemy];
// jmarshall - add flag support.
		//if (EntityCarriesFlag(&curenemyinfo)) return qfalse;
// jmarshall end
		VectorSubtract(curenemyinfo->r.currentOrigin, bs->origin, dir);
		cursquaredist = VectorLengthSquared(dir);
	}
	else {
		cursquaredist = 0;
	}

	for (i = 0; i < MAX_CLIENTS; i++) {

		if (i == bs->client) 
			continue;

		//if it's the current enemy
		if (i == curenemy) 
			continue;

		entinfo = &g_entities[i];
		if (!entinfo->inuse)
			continue;
		
		//if the enemy isn't dead and the enemy isn't the bot self
		if (EntityIsDead(entinfo) || i == bs->entitynum) 
			continue;

		//if the enemy is invisible and not shooting
// jmarshall - add invis
		//if (EntityIsInvisible(&entinfo) && !EntityIsShooting(&entinfo)) {
		//	continue;
		//}
// jmarshall end

// jmarshall - eval, looks like code to not shoot chatting players or players that just spawned in.
//			   do we care about this?
		//if not an easy fragger don't shoot at chatting players
		//if (easyfragger < 0.5 && EntityIsChatting(&entinfo)) 
		//	continue;
		
		// 
		//if (lastteleport_time > FloatTime() - 3) {
		//	VectorSubtract(entinfo.origin, lastteleport_origin, dir);
		//	if (VectorLengthSquared(dir) < Square(70)) 
		//		continue;
		//}
// jmarshall end

		//calculate the distance towards the enemy
		VectorSubtract(entinfo->r.currentOrigin, bs->origin, dir);

		squaredist = VectorLengthSquared(dir);

// jmarshall
		//if this entity is not carrying a flag
		//if (!EntityCarriesFlag(&entinfo))
		//{
			//if this enemy is further away than the current one
			if (curenemy >= 0 && squaredist > cursquaredist) 
				continue;
		//} 
// jmarshall end

		//if the bot has no
		if (squaredist > Square(900.0 + alertness * 4000.0)) 
			continue;

// jmarshall - teams!
		//if on the same team
		//if (BotSameTeam(bs, i)) 
		//	continue;
// jmarshall end
		//if the bot's health decreased or the enemy is shooting
		if (curenemy < 0 && (healthdecrease || EntityIsShooting(entinfo)))
			f = 360;
		else
			f = 90 + 90 - (90 - (squaredist > Square(810) ? Square(810) : squaredist) / (810 * 9));
		//check if the enemy is visible
		vis = BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, f, i);
		if (vis <= 0) 
			continue;
		
		//if the enemy is quite far away, not shooting and the bot is not damaged
		if (curenemy < 0 && squaredist > Square(100) && !healthdecrease && !EntityIsShooting(entinfo))
		{
			//check if we can avoid this enemy
			VectorSubtract(bs->origin, entinfo->r.currentOrigin, dir);
			vectoangles(dir, angles);
			
			//if the bot isn't in the fov of the enemy
			if (!InFieldOfVision(bs->viewangles, 90, angles)) {
				//update some stuff for this enemy
				BotUpdateBattleInventory(bs, i);

				//if the bot doesn't really want to fight
				if (BotWantsToRetreat(bs)) 
					continue;
			}
		}
		//found an enemy
		bs->enemy = i;//entinfo.number;
		if (curenemy >= 0) 
			bs->enemysight_time = FloatTime() - 2;
		else 
			bs->enemysight_time = FloatTime();
		bs->enemysuicide = qfalse;
		bs->enemydeath_time = 0;
		bs->enemyvisible_time = FloatTime();
		return qtrue;
	}
	return qfalse;
}
