// BotAI.c
//

#include "../q_shared.h"
#include "../g_local.h"
#include "../chars.h"
#include "BotAI.h"

///*
//==================
//EntityIsDead
//==================
//*/
//qboolean EntityIsDead(aas_entityinfo_t* entinfo) {
//	playerState_t ps;
//
//	if (entinfo->number >= 0 && entinfo->number < MAX_CLIENTS) {
//		//retrieve the current client state
//		BotAI_GetClientState(entinfo->number, &ps);
//		if (ps.pm_type != PM_NORMAL) return qtrue;
//	}
//	return qfalse;
//}
//
///*
//==================
//BotFindEnemy
//==================
//*/
//int BotFindEnemy(bot_state_t* bs, int curenemy) {
//	int i, healthdecrease;
//	float f, alertness, easyfragger, vis;
//	float squaredist, cursquaredist;
//	gentity_t *entinfo, *curenemyinfo;
//	vec3_t dir, angles;
//
//	alertness = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ALERTNESS, 0, 1);
//	easyfragger = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_EASY_FRAGGER, 0, 1);
//	//check if the health decreased
//	healthdecrease = bs->lasthealth > bs->inventory[INVENTORY_HEALTH];
//	//remember the current health value
//	bs->lasthealth = bs->inventory[INVENTORY_HEALTH];
//	//
//	if (curenemy >= 0) {
//		//BotEntityInfo(curenemy, &curenemyinfo);
//		curenemyinfo = &g_entities[curenemy];
//// jmarshall - add flag support.
//		//if (EntityCarriesFlag(&curenemyinfo)) return qfalse;
//// jmarshall end
//		VectorSubtract(curenemyinfo->r.currentOrigin, bs->origin, dir);
//		cursquaredist = VectorLengthSquared(dir);
//	}
//	else {
//		cursquaredist = 0;
//	}
//
//	for (i = 0; i < MAX_CLIENTS; i++) {
//
//		if (i == bs->client) 
//			continue;
//
//		//if it's the current enemy
//		if (i == curenemy) 
//			continue;
//
//		entinfo = &g_entities[i];
//		if (!entinfo->inuse)
//			continue;
//		
//		//if the enemy isn't dead and the enemy isn't the bot self
//		if (EntityIsDead(&entinfo) || entinfo.number == bs->entitynum) 
//			continue;
//
//		//if the enemy is invisible and not shooting
//		if (EntityIsInvisible(&entinfo) && !EntityIsShooting(&entinfo)) {
//			continue;
//		}
//		//if not an easy fragger don't shoot at chatting players
//		if (easyfragger < 0.5 && EntityIsChatting(&entinfo)) continue;
//		//
//		if (lastteleport_time > FloatTime() - 3) {
//			VectorSubtract(entinfo.origin, lastteleport_origin, dir);
//			if (VectorLengthSquared(dir) < Square(70)) continue;
//		}
//		//calculate the distance towards the enemy
//		VectorSubtract(entinfo.origin, bs->origin, dir);
//		squaredist = VectorLengthSquared(dir);
//		//if this entity is not carrying a flag
//		if (!EntityCarriesFlag(&entinfo))
//		{
//			//if this enemy is further away than the current one
//			if (curenemy >= 0 && squaredist > cursquaredist) continue;
//		} //end if
//		//if the bot has no
//		if (squaredist > Square(900.0 + alertness * 4000.0)) continue;
//		//if on the same team
//		if (BotSameTeam(bs, i)) continue;
//		//if the bot's health decreased or the enemy is shooting
//		if (curenemy < 0 && (healthdecrease || EntityIsShooting(&entinfo)))
//			f = 360;
//		else
//			f = 90 + 90 - (90 - (squaredist > Square(810) ? Square(810) : squaredist) / (810 * 9));
//		//check if the enemy is visible
//		vis = BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, f, i);
//		if (vis <= 0) continue;
//		//if the enemy is quite far away, not shooting and the bot is not damaged
//		if (curenemy < 0 && squaredist > Square(100) && !healthdecrease && !EntityIsShooting(&entinfo))
//		{
//			//check if we can avoid this enemy
//			VectorSubtract(bs->origin, entinfo.origin, dir);
//			vectoangles(dir, angles);
//			//if the bot isn't in the fov of the enemy
//			if (!InFieldOfVision(entinfo.angles, 90, angles)) {
//				//update some stuff for this enemy
//				BotUpdateBattleInventory(bs, i);
//				//if the bot doesn't really want to fight
//				if (BotWantsToRetreat(bs)) continue;
//			}
//		}
//		//found an enemy
//		bs->enemy = entinfo.number;
//		if (curenemy >= 0) bs->enemysight_time = FloatTime() - 2;
//		else bs->enemysight_time = FloatTime();
//		bs->enemysuicide = qfalse;
//		bs->enemydeath_time = 0;
//		bs->enemyvisible_time = FloatTime();
//		return qtrue;
//	}
//	return qfalse;
//}
