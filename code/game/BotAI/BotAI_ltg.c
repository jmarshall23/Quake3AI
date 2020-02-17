// BotAI_ltg.c
//

#include "../q_shared.h"
#include "../g_local.h"

#include "BotAI.h"


/*
==================
BotReachedGoal
==================
*/
int BotReachedGoal(bot_state_t* bs, bot_goal_t* goal) {
	if (goal->flags & GFL_ITEM) {
		//if touching the goal
		if (BotTouchingGoal(bs->origin, goal)) {
			if (!(goal->flags & GFL_DROPPED)) {
				BotSetAvoidGoalTime(bs->gs, goal->number, -1);
			}
			return qtrue;
		}
		//if the goal isn't there
		if (BotItemGoalInVisButNotVisible(bs->entitynum, bs->eye, bs->viewangles, goal)) {
			/*
			float avoidtime;
			int t;

			avoidtime = trap_BotAvoidGoalTime(bs->gs, goal->number);
			if (avoidtime > 0) {
				t = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, goal->areanum, bs->tfl);
				if ((float) t * 0.009 < avoidtime)
					return qtrue;
			}
			*/
			return qtrue;
		}
		//if in the goal area and below or above the goal and not swimming
		//if (bs->areanum == goal->areanum) {
		//	if (bs->origin[0] > goal->origin[0] + goal->mins[0] && bs->origin[0] < goal->origin[0] + goal->maxs[0]) {
		//		if (bs->origin[1] > goal->origin[1] + goal->mins[1] && bs->origin[1] < goal->origin[1] + goal->maxs[1]) {
		//			if (!trap_AAS_Swimming(bs->origin)) {
		//				return qtrue;
		//			}
		//		}
		//	}
		//}
	}
	//else if (goal->flags & GFL_AIR) {
	//	//if touching the goal
	//	if (trap_BotTouchingGoal(bs->origin, goal)) return qtrue;
	//	//if the bot got air
	//	if (bs->lastair_time > FloatTime() - 1) return qtrue;
	//}
	else {
		//if touching the goal
		if (BotTouchingGoal(bs->origin, goal)) 
			return qtrue;
	}
	return qfalse;
}

/*
==================
BotGetItemLongTermGoal
==================
*/
int BotGetItemLongTermGoal(bot_state_t* bs, int tfl, bot_goal_t* goal) {
	//if the bot has no goal
	if (!BotGetTopGoal(bs->gs, goal)) {
		//BotAI_Print(PRT_MESSAGE, "no ltg on stack\n");
		bs->ltg_time = 0;
	}
	//if the bot touches the current goal
	else if (BotReachedGoal(bs, goal)) {
		BotChooseWeapon(bs);
		bs->ltg_time = 0;
	}

	// Check to see that we can get to our goal, if not get a new goal.
	if (bs->numMovementWaypoints > 0)
	{
		trace_t tr;
		gentity_t* ent = &g_entities[bs->client];
		trap_Trace(&tr, ent->r.currentOrigin, NULL, NULL, bs->movement_waypoints[bs->currentWaypoint], bs->client, CONTENTS_SOLID);

		if(tr.fraction  < 0.9f)
		{
			BotChooseWeapon(bs);
			bs->ltg_time = 0;
		}
	}

	//if it is time to find a new long term goal
	if (bs->ltg_time < FloatTime()) {
		//pop the current goal from the stack
		BotPopGoal(bs->gs);
		//BotAI_Print(PRT_MESSAGE, "%s: choosing new ltg\n", ClientName(bs->client, netname, sizeof(netname)));
		//choose a new goal
		//BotAI_Print(PRT_MESSAGE, "%6.1f client %d: BotChooseLTGItem\n", FloatTime(), bs->client);
		if (BotChooseLTGItem(bs->gs, bs->origin, bs->inventory, tfl)) {
			/*
			char buf[128];
			//get the goal at the top of the stack
			trap_BotGetTopGoal(bs->gs, goal);
			trap_BotGoalName(goal->number, buf, sizeof(buf));
			BotAI_Print(PRT_MESSAGE, "%1.1f: new long term goal %s\n", FloatTime(), buf);
			*/
			bs->ltg_time = FloatTime() + 20;
		}
		else {//the bot gets sorta stuck with all the avoid timings, shouldn't happen though
			//
			//trap_BotDumpAvoidGoals(bs->gs);
			//reset the avoid goals and the avoid reach
			BotResetAvoidGoals(bs->gs);
			//BotResetAvoidReach(bs->ms);
		}
		//get the goal at the top of the stack
		return BotGetTopGoal(bs->gs, goal);
	}
	return qtrue;
}

/*
==================
BotGetLongTermGoal

we could also create a seperate AI node for every long term goal type
however this saves us a lot of code
==================
*/
int BotGetLongTermGoal(bot_state_t* bs, int tfl, int retreat, bot_goal_t* goal) {
	vec3_t target, dir, dir2;
	//char netname[MAX_NETNAME];
	//char buf[MAX_MESSAGE_SIZE];
	int areanum;
	float croucher;
	//aas_entityinfo_t entinfo, botinfo;
	bot_waypoint_t* wp;
// jmarshall
//	if (bs->ltgtype == LTG_TEAMHELP && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			BotAI_BotInitialChat(bs, "help_start", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_YES);
//			trap_EA_Action(bs->client, ACTION_AFFIRMATIVE);
//			bs->teammessage_time = 0;
//		}
//		//if trying to help the team mate for more than a minute
//		if (bs->teamgoal_time < FloatTime())
//			bs->ltgtype = 0;
//		//if the team mate IS visible for quite some time
//		if (bs->teammatevisible_time < FloatTime() - 10) bs->ltgtype = 0;
//		//get entity information of the companion
//		BotEntityInfo(bs->teammate, &entinfo);
//		//if the team mate is visible
//		if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->teammate)) {
//			//if close just stand still there
//			VectorSubtract(entinfo.origin, bs->origin, dir);
//			if (VectorLengthSquared(dir) < Square(100)) {
//				trap_BotResetAvoidReach(bs->ms);
//				return qfalse;
//			}
//		}
//		else {
//			//last time the bot was NOT visible
//			bs->teammatevisible_time = FloatTime();
//		}
//		//if the entity information is valid (entity in PVS)
//		if (entinfo.valid) {
//			areanum = BotPointAreaNum(entinfo.origin);
//			if (areanum && trap_AAS_AreaReachability(areanum)) {
//				//update team goal
//				bs->teamgoal.entitynum = bs->teammate;
//				bs->teamgoal.areanum = areanum;
//				VectorCopy(entinfo.origin, bs->teamgoal.origin);
//				VectorSet(bs->teamgoal.mins, -8, -8, -8);
//				VectorSet(bs->teamgoal.maxs, 8, 8, 8);
//			}
//		}
//		memcpy(goal, &bs->teamgoal, sizeof(bot_goal_t));
//		return qtrue;
//	}
//
//	//if the bot accompanies someone
//	if (bs->ltgtype == LTG_TEAMACCOMPANY && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			BotAI_BotInitialChat(bs, "accompany_start", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_YES);
//			trap_EA_Action(bs->client, ACTION_AFFIRMATIVE);
//			bs->teammessage_time = 0;
//		}
//		//if accompanying the companion for 3 minutes
//		if (bs->teamgoal_time < FloatTime()) {
//			BotAI_BotInitialChat(bs, "accompany_stop", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//			trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
//			bs->ltgtype = 0;
//		}
//		//get entity information of the companion
//		BotEntityInfo(bs->teammate, &entinfo);
//		//if the companion is visible
//		if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->teammate)) {
//			//update visible time
//			bs->teammatevisible_time = FloatTime();
//			VectorSubtract(entinfo.origin, bs->origin, dir);
//			if (VectorLengthSquared(dir) < Square(bs->formation_dist)) {
//				//
//				// if the client being followed bumps into this bot then
//				// the bot should back up
//				BotEntityInfo(bs->entitynum, &botinfo);
//				// if the followed client is not standing ontop of the bot
//				if (botinfo.origin[2] + botinfo.maxs[2] > entinfo.origin[2] + entinfo.mins[2]) {
//					// if the bounding boxes touch each other
//					if (botinfo.origin[0] + botinfo.maxs[0] > entinfo.origin[0] + entinfo.mins[0] - 4 &&
//						botinfo.origin[0] + botinfo.mins[0] < entinfo.origin[0] + entinfo.maxs[0] + 4) {
//						if (botinfo.origin[1] + botinfo.maxs[1] > entinfo.origin[1] + entinfo.mins[1] - 4 &&
//							botinfo.origin[1] + botinfo.mins[1] < entinfo.origin[1] + entinfo.maxs[1] + 4) {
//							if (botinfo.origin[2] + botinfo.maxs[2] > entinfo.origin[2] + entinfo.mins[2] - 4 &&
//								botinfo.origin[2] + botinfo.mins[2] < entinfo.origin[2] + entinfo.maxs[2] + 4) {
//								// if the followed client looks in the direction of this bot
//								AngleVectors(entinfo.angles, dir, NULL, NULL);
//								dir[2] = 0;
//								VectorNormalize(dir);
//								//VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
//								VectorSubtract(bs->origin, entinfo.origin, dir2);
//								VectorNormalize(dir2);
//								if (DotProduct(dir, dir2) > 0.7) {
//									// back up
//									BotSetupForMovement(bs);
//									trap_BotMoveInDirection(bs->ms, dir2, 400, MOVE_WALK);
//								}
//							}
//						}
//					}
//				}
//				//check if the bot wants to crouch
//				//don't crouch if crouched less than 5 seconds ago
//				if (bs->attackcrouch_time < FloatTime() - 5) {
//					croucher = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CROUCHER, 0, 1);
//					if (random() < bs->thinktime * croucher) {
//						bs->attackcrouch_time = FloatTime() + 5 + croucher * 15;
//					}
//				}
//				//don't crouch when swimming
//				if (trap_AAS_Swimming(bs->origin)) bs->attackcrouch_time = FloatTime() - 1;
//				//if not arrived yet or arived some time ago
//				if (bs->arrive_time < FloatTime() - 2) {
//					//if not arrived yet
//					if (!bs->arrive_time) {
//						trap_EA_Gesture(bs->client);
//						BotAI_BotInitialChat(bs, "accompany_arrive", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//						trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
//						bs->arrive_time = FloatTime();
//					}
//					//if the bot wants to crouch
//					else if (bs->attackcrouch_time > FloatTime()) {
//						trap_EA_Crouch(bs->client);
//					}
//					//else do some model taunts
//					else if (random() < bs->thinktime * 0.05) {
//						//do a gesture :)
//						trap_EA_Gesture(bs->client);
//					}
//				}
//				//if just arrived look at the companion
//				if (bs->arrive_time > FloatTime() - 2) {
//					VectorSubtract(entinfo.origin, bs->origin, dir);
//					vectoangles(dir, bs->ideal_viewangles);
//					bs->ideal_viewangles[2] *= 0.5;
//				}
//				//else look strategically around for enemies
//				else if (random() < bs->thinktime * 0.8) {
//					BotRoamGoal(bs, target);
//					VectorSubtract(target, bs->origin, dir);
//					vectoangles(dir, bs->ideal_viewangles);
//					bs->ideal_viewangles[2] *= 0.5;
//				}
//				//check if the bot wants to go for air
//				if (BotGoForAir(bs, bs->tfl, &bs->teamgoal, 400)) {
//					trap_BotResetLastAvoidReach(bs->ms);
//					//get the goal at the top of the stack
//					//trap_BotGetTopGoal(bs->gs, &tmpgoal);
//					//trap_BotGoalName(tmpgoal.number, buf, 144);
//					//BotAI_Print(PRT_MESSAGE, "new nearby goal %s\n", buf);
//					//time the bot gets to pick up the nearby goal item
//					bs->nbg_time = FloatTime() + 8;
//					AIEnter_Seek_NBG(bs, "BotLongTermGoal: go for air");
//					return qfalse;
//				}
//				//
//				trap_BotResetAvoidReach(bs->ms);
//				return qfalse;
//			}
//		}
//		//if the entity information is valid (entity in PVS)
//		if (entinfo.valid) {
//			areanum = BotPointAreaNum(entinfo.origin);
//			if (areanum && trap_AAS_AreaReachability(areanum)) {
//				//update team goal
//				bs->teamgoal.entitynum = bs->teammate;
//				bs->teamgoal.areanum = areanum;
//				VectorCopy(entinfo.origin, bs->teamgoal.origin);
//				VectorSet(bs->teamgoal.mins, -8, -8, -8);
//				VectorSet(bs->teamgoal.maxs, 8, 8, 8);
//			}
//		}
//		//the goal the bot should go for
//		memcpy(goal, &bs->teamgoal, sizeof(bot_goal_t));
//		//if the companion is NOT visible for too long
//		if (bs->teammatevisible_time < FloatTime() - 60) {
//			BotAI_BotInitialChat(bs, "accompany_cannotfind", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//			trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
//			bs->ltgtype = 0;
//			// just to make sure the bot won't spam this message
//			bs->teammatevisible_time = FloatTime();
//		}
//		return qtrue;
//	}
//	//
//	if (bs->ltgtype == LTG_DEFENDKEYAREA) {
//		if (trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin,
//			bs->teamgoal.areanum, TFL_DEFAULT) > bs->defendaway_range) {
//			bs->defendaway_time = 0;
//		}
//	}
//	//if defending a key area
//	if (bs->ltgtype == LTG_DEFENDKEYAREA && !retreat &&
//		bs->defendaway_time < FloatTime()) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			trap_BotGoalName(bs->teamgoal.number, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "defend_start", buf, NULL);
//			trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//			BotVoiceChatOnly(bs, -1, VOICECHAT_ONDEFENSE);
//			bs->teammessage_time = 0;
//		}
//		//set the bot goal
//		memcpy(goal, &bs->teamgoal, sizeof(bot_goal_t));
//		//stop after 2 minutes
//		if (bs->teamgoal_time < FloatTime()) {
//			trap_BotGoalName(bs->teamgoal.number, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "defend_stop", buf, NULL);
//			trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//			bs->ltgtype = 0;
//		}
//		//if very close... go away for some time
//		VectorSubtract(goal->origin, bs->origin, dir);
//		if (VectorLengthSquared(dir) < Square(70)) {
//			trap_BotResetAvoidReach(bs->ms);
//			bs->defendaway_time = FloatTime() + 3 + 3 * random();
//			if (BotHasPersistantPowerupAndWeapon(bs)) {
//				bs->defendaway_range = 100;
//			}
//			else {
//				bs->defendaway_range = 350;
//			}
//		}
//		return qtrue;
//	}
//	//going to kill someone
//	if (bs->ltgtype == LTG_KILL && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			EasyClientName(bs->teamgoal.entitynum, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "kill_start", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			bs->teammessage_time = 0;
//		}
//		//
//		if (bs->lastkilledplayer == bs->teamgoal.entitynum) {
//			EasyClientName(bs->teamgoal.entitynum, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "kill_done", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			bs->lastkilledplayer = -1;
//			bs->ltgtype = 0;
//		}
//		//
//		if (bs->teamgoal_time < FloatTime()) {
//			bs->ltgtype = 0;
//		}
//		//just roam around
//		return BotGetItemLongTermGoal(bs, tfl, goal);
//	}
//	//get an item
//	if (bs->ltgtype == LTG_GETITEM && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			trap_BotGoalName(bs->teamgoal.number, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "getitem_start", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_YES);
//			trap_EA_Action(bs->client, ACTION_AFFIRMATIVE);
//			bs->teammessage_time = 0;
//		}
//		//set the bot goal
//		memcpy(goal, &bs->teamgoal, sizeof(bot_goal_t));
//		//stop after some time
//		if (bs->teamgoal_time < FloatTime()) {
//			bs->ltgtype = 0;
//		}
//		//
//		if (trap_BotItemGoalInVisButNotVisible(bs->entitynum, bs->eye, bs->viewangles, goal)) {
//			trap_BotGoalName(bs->teamgoal.number, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "getitem_notthere", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			bs->ltgtype = 0;
//		}
//		else if (BotReachedGoal(bs, goal)) {
//			trap_BotGoalName(bs->teamgoal.number, buf, sizeof(buf));
//			BotAI_BotInitialChat(bs, "getitem_gotit", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			bs->ltgtype = 0;
//		}
//		return qtrue;
//	}
//	//if camping somewhere
//	if ((bs->ltgtype == LTG_CAMP || bs->ltgtype == LTG_CAMPORDER) && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			if (bs->ltgtype == LTG_CAMPORDER) {
//				BotAI_BotInitialChat(bs, "camp_start", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//				trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//				BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_YES);
//				trap_EA_Action(bs->client, ACTION_AFFIRMATIVE);
//			}
//			bs->teammessage_time = 0;
//		}
//		//set the bot goal
//		memcpy(goal, &bs->teamgoal, sizeof(bot_goal_t));
//		//
//		if (bs->teamgoal_time < FloatTime()) {
//			if (bs->ltgtype == LTG_CAMPORDER) {
//				BotAI_BotInitialChat(bs, "camp_stop", NULL);
//				trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			}
//			bs->ltgtype = 0;
//		}
//		//if really near the camp spot
//		VectorSubtract(goal->origin, bs->origin, dir);
//		if (VectorLengthSquared(dir) < Square(60))
//		{
//			//if not arrived yet
//			if (!bs->arrive_time) {
//				if (bs->ltgtype == LTG_CAMPORDER) {
//					BotAI_BotInitialChat(bs, "camp_arrive", EasyClientName(bs->teammate, netname, sizeof(netname)), NULL);
//					trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//					BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_INPOSITION);
//				}
//				bs->arrive_time = FloatTime();
//			}
//			//look strategically around for enemies
//			if (random() < bs->thinktime * 0.8) {
//				BotRoamGoal(bs, target);
//				VectorSubtract(target, bs->origin, dir);
//				vectoangles(dir, bs->ideal_viewangles);
//				bs->ideal_viewangles[2] *= 0.5;
//			}
//			//check if the bot wants to crouch
//			//don't crouch if crouched less than 5 seconds ago
//			if (bs->attackcrouch_time < FloatTime() - 5) {
//				croucher = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CROUCHER, 0, 1);
//				if (random() < bs->thinktime * croucher) {
//					bs->attackcrouch_time = FloatTime() + 5 + croucher * 15;
//				}
//			}
//			//if the bot wants to crouch
//			if (bs->attackcrouch_time > FloatTime()) {
//				trap_EA_Crouch(bs->client);
//			}
//			//don't crouch when swimming
//			if (trap_AAS_Swimming(bs->origin)) bs->attackcrouch_time = FloatTime() - 1;
//			//make sure the bot is not gonna drown
//			if (trap_PointContents(bs->eye, bs->entitynum) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)) {
//				if (bs->ltgtype == LTG_CAMPORDER) {
//					BotAI_BotInitialChat(bs, "camp_stop", NULL);
//					trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//					//
//					if (bs->lastgoal_ltgtype == LTG_CAMPORDER) {
//						bs->lastgoal_ltgtype = 0;
//					}
//				}
//				bs->ltgtype = 0;
//			}
//			//
//			if (bs->camp_range > 0) {
//				//FIXME: move around a bit
//			}
//			//
//			trap_BotResetAvoidReach(bs->ms);
//			return qfalse;
//		}
//		return qtrue;
//	}
//	//patrolling along several waypoints
//	if (bs->ltgtype == LTG_PATROL && !retreat) {
//		//check for bot typing status message
//		if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//			strcpy(buf, "");
//			for (wp = bs->patrolpoints; wp; wp = wp->next) {
//				strcat(buf, wp->name);
//				if (wp->next) strcat(buf, " to ");
//			}
//			BotAI_BotInitialChat(bs, "patrol_start", buf, NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			BotVoiceChatOnly(bs, bs->decisionmaker, VOICECHAT_YES);
//			trap_EA_Action(bs->client, ACTION_AFFIRMATIVE);
//			bs->teammessage_time = 0;
//		}
//		//
//		if (!bs->curpatrolpoint) {
//			bs->ltgtype = 0;
//			return qfalse;
//		}
//		//if the bot touches the current goal
//		if (trap_BotTouchingGoal(bs->origin, &bs->curpatrolpoint->goal)) {
//			if (bs->patrolflags & PATROL_BACK) {
//				if (bs->curpatrolpoint->prev) {
//					bs->curpatrolpoint = bs->curpatrolpoint->prev;
//				}
//				else {
//					bs->curpatrolpoint = bs->curpatrolpoint->next;
//					bs->patrolflags &= ~PATROL_BACK;
//				}
//			}
//			else {
//				if (bs->curpatrolpoint->next) {
//					bs->curpatrolpoint = bs->curpatrolpoint->next;
//				}
//				else {
//					bs->curpatrolpoint = bs->curpatrolpoint->prev;
//					bs->patrolflags |= PATROL_BACK;
//				}
//			}
//		}
//		//stop after 5 minutes
//		if (bs->teamgoal_time < FloatTime()) {
//			BotAI_BotInitialChat(bs, "patrol_stop", NULL);
//			trap_BotEnterChat(bs->cs, bs->decisionmaker, CHAT_TELL);
//			bs->ltgtype = 0;
//		}
//		if (!bs->curpatrolpoint) {
//			bs->ltgtype = 0;
//			return qfalse;
//		}
//		memcpy(goal, &bs->curpatrolpoint->goal, sizeof(bot_goal_t));
//		return qtrue;
//	}
//#ifdef CTF
//	if (gametype == GT_CTF) {
//		//if going for enemy flag
//		if (bs->ltgtype == LTG_GETFLAG) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "captureflag_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONGETFLAG);
//				bs->teammessage_time = 0;
//			}
//			//
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &ctf_blueflag, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &ctf_redflag, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//if touching the flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				// make sure the bot knows the flag isn't there anymore
//				switch (BotTeam(bs)) {
//				case TEAM_RED: bs->blueflagstatus = 1; break;
//				case TEAM_BLUE: bs->redflagstatus = 1; break;
//				}
//				bs->ltgtype = 0;
//			}
//			//stop after 3 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			BotAlternateRoute(bs, goal);
//			return qtrue;
//		}
//		//if rushing to the base
//		if (bs->ltgtype == LTG_RUSHBASE && bs->rushbaseaway_time < FloatTime()) {
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &ctf_redflag, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &ctf_blueflag, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//if not carrying the flag anymore
//			if (!BotCTFCarryingFlag(bs)) bs->ltgtype = 0;
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) bs->ltgtype = 0;
//			//if touching the base flag the bot should loose the enemy flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				//if the bot is still carrying the enemy flag then the
//				//base flag is gone, now just walk near the base a bit
//				if (BotCTFCarryingFlag(bs)) {
//					trap_BotResetAvoidReach(bs->ms);
//					bs->rushbaseaway_time = FloatTime() + 5 + 10 * random();
//					//FIXME: add chat to tell the others to get back the flag
//				}
//				else {
//					bs->ltgtype = 0;
//				}
//			}
//			BotAlternateRoute(bs, goal);
//			return qtrue;
//		}
//		//returning flag
//		if (bs->ltgtype == LTG_RETURNFLAG) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "returnflag_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONRETURNFLAG);
//				bs->teammessage_time = 0;
//			}
//			//
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &ctf_blueflag, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &ctf_redflag, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//if touching the flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) bs->ltgtype = 0;
//			//stop after 3 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			BotAlternateRoute(bs, goal);
//			return qtrue;
//		}
//	}
//#endif //CTF
//#ifdef MISSIONPACK
//	else if (gametype == GT_1FCTF) {
//		if (bs->ltgtype == LTG_GETFLAG) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "captureflag_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONGETFLAG);
//				bs->teammessage_time = 0;
//			}
//			memcpy(goal, &ctf_neutralflag, sizeof(bot_goal_t));
//			//if touching the flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->ltgtype = 0;
//			}
//			//stop after 3 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			return qtrue;
//		}
//		//if rushing to the base
//		if (bs->ltgtype == LTG_RUSHBASE) {
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &ctf_blueflag, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &ctf_redflag, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//if not carrying the flag anymore
//			if (!Bot1FCTFCarryingFlag(bs)) {
//				bs->ltgtype = 0;
//			}
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			//if touching the base flag the bot should loose the enemy flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->ltgtype = 0;
//			}
//			BotAlternateRoute(bs, goal);
//			return qtrue;
//		}
//		//attack the enemy base
//		if (bs->ltgtype == LTG_ATTACKENEMYBASE &&
//			bs->attackaway_time < FloatTime()) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "attackenemybase_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONOFFENSE);
//				bs->teammessage_time = 0;
//			}
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &ctf_blueflag, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &ctf_redflag, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			//if touching the base flag the bot should loose the enemy flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->attackaway_time = FloatTime() + 2 + 5 * random();
//			}
//			return qtrue;
//		}
//		//returning flag
//		if (bs->ltgtype == LTG_RETURNFLAG) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "returnflag_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONRETURNFLAG);
//				bs->teammessage_time = 0;
//			}
//			//
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			//just roam around
//			return BotGetItemLongTermGoal(bs, tfl, goal);
//		}
//	}
//	else if (gametype == GT_OBELISK) {
//		if (bs->ltgtype == LTG_ATTACKENEMYBASE &&
//			bs->attackaway_time < FloatTime()) {
//
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "attackenemybase_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONOFFENSE);
//				bs->teammessage_time = 0;
//			}
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &blueobelisk, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &redobelisk, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//if the bot no longer wants to attack the obelisk
//			if (BotFeelingBad(bs) > 50) {
//				return BotGetItemLongTermGoal(bs, tfl, goal);
//			}
//			//if touching the obelisk
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->attackaway_time = FloatTime() + 3 + 5 * random();
//			}
//			// or very close to the obelisk
//			VectorSubtract(bs->origin, goal->origin, dir);
//			if (VectorLengthSquared(dir) < Square(60)) {
//				bs->attackaway_time = FloatTime() + 3 + 5 * random();
//			}
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			BotAlternateRoute(bs, goal);
//			//just move towards the obelisk
//			return qtrue;
//		}
//	}
//	else if (gametype == GT_HARVESTER) {
//		//if rushing to the base
//		if (bs->ltgtype == LTG_RUSHBASE) {
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &blueobelisk, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &redobelisk, sizeof(bot_goal_t)); break;
//			default: BotGoHarvest(bs); return qfalse;
//			}
//			//if not carrying any cubes
//			if (!BotHarvesterCarryingCubes(bs)) {
//				BotGoHarvest(bs);
//				return qfalse;
//			}
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				BotGoHarvest(bs);
//				return qfalse;
//			}
//			//if touching the base flag the bot should loose the enemy flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				BotGoHarvest(bs);
//				return qfalse;
//			}
//			BotAlternateRoute(bs, goal);
//			return qtrue;
//		}
//		//attack the enemy base
//		if (bs->ltgtype == LTG_ATTACKENEMYBASE &&
//			bs->attackaway_time < FloatTime()) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "attackenemybase_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONOFFENSE);
//				bs->teammessage_time = 0;
//			}
//			switch (BotTeam(bs)) {
//			case TEAM_RED: memcpy(goal, &blueobelisk, sizeof(bot_goal_t)); break;
//			case TEAM_BLUE: memcpy(goal, &redobelisk, sizeof(bot_goal_t)); break;
//			default: bs->ltgtype = 0; return qfalse;
//			}
//			//quit rushing after 2 minutes
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			//if touching the base flag the bot should loose the enemy flag
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->attackaway_time = FloatTime() + 2 + 5 * random();
//			}
//			return qtrue;
//		}
//		//harvest cubes
//		if (bs->ltgtype == LTG_HARVEST &&
//			bs->harvestaway_time < FloatTime()) {
//			//check for bot typing status message
//			if (bs->teammessage_time && bs->teammessage_time < FloatTime()) {
//				BotAI_BotInitialChat(bs, "harvest_start", NULL);
//				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
//				BotVoiceChatOnly(bs, -1, VOICECHAT_ONOFFENSE);
//				bs->teammessage_time = 0;
//			}
//			memcpy(goal, &neutralobelisk, sizeof(bot_goal_t));
//			//
//			if (bs->teamgoal_time < FloatTime()) {
//				bs->ltgtype = 0;
//			}
//			//
//			if (trap_BotTouchingGoal(bs->origin, goal)) {
//				bs->harvestaway_time = FloatTime() + 4 + 3 * random();
//			}
//			return qtrue;
//		}
//	}
//#endif
// jmarshall end

	//normal goal stuff
	return BotGetItemLongTermGoal(bs, tfl, goal);
}

/*
==================
BotLongTermGoal
==================
*/
int BotLongTermGoal(bot_state_t* bs, int tfl, int retreat, bot_goal_t* goal) {
	//aas_entityinfo_t entinfo;
	//char teammate[MAX_MESSAGE_SIZE];
	float squaredist;
	int areanum;
	vec3_t dir;

// jmarshall
	//FIXME: also have air long term goals?
	//
	//if the bot is leading someone and not retreating
	//if (bs->lead_time > 0 && !retreat) {
	//	if (bs->lead_time < FloatTime()) {
	//		BotAI_BotInitialChat(bs, "lead_stop", EasyClientName(bs->lead_teammate, teammate, sizeof(teammate)), NULL);
	//		trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
	//		bs->lead_time = 0;
	//		return BotGetLongTermGoal(bs, tfl, retreat, goal);
	//	}
	//	//
	//	if (bs->leadmessage_time < 0 && -bs->leadmessage_time < FloatTime()) {
	//		BotAI_BotInitialChat(bs, "followme", EasyClientName(bs->lead_teammate, teammate, sizeof(teammate)), NULL);
	//		trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
	//		bs->leadmessage_time = FloatTime();
	//	}
	//	//get entity information of the companion
	//	BotEntityInfo(bs->lead_teammate, &entinfo);
	//	//
	//	if (entinfo.valid) {
	//		areanum = BotPointAreaNum(entinfo.origin);
	//		if (areanum && trap_AAS_AreaReachability(areanum)) {
	//			//update team goal
	//			bs->lead_teamgoal.entitynum = bs->lead_teammate;
	//			bs->lead_teamgoal.areanum = areanum;
	//			VectorCopy(entinfo.origin, bs->lead_teamgoal.origin);
	//			VectorSet(bs->lead_teamgoal.mins, -8, -8, -8);
	//			VectorSet(bs->lead_teamgoal.maxs, 8, 8, 8);
	//		}
	//	}
	//	//if the team mate is visible
	//	if (BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->lead_teammate)) {
	//		bs->leadvisible_time = FloatTime();
	//	}
	//	//if the team mate is not visible for 1 seconds
	//	if (bs->leadvisible_time < FloatTime() - 1) {
	//		bs->leadbackup_time = FloatTime() + 2;
	//	}
	//	//distance towards the team mate
	//	VectorSubtract(bs->origin, bs->lead_teamgoal.origin, dir);
	//	squaredist = VectorLengthSquared(dir);
	//	//if backing up towards the team mate
	//	if (bs->leadbackup_time > FloatTime()) {
	//		if (bs->leadmessage_time < FloatTime() - 20) {
	//			BotAI_BotInitialChat(bs, "followme", EasyClientName(bs->lead_teammate, teammate, sizeof(teammate)), NULL);
	//			trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
	//			bs->leadmessage_time = FloatTime();
	//		}
	//		//if very close to the team mate
	//		if (squaredist < Square(100)) {
	//			bs->leadbackup_time = 0;
	//		}
	//		//the bot should go back to the team mate
	//		memcpy(goal, &bs->lead_teamgoal, sizeof(bot_goal_t));
	//		return qtrue;
	//	}
	//	else {
	//		//if quite distant from the team mate
	//		if (squaredist > Square(500)) {
	//			if (bs->leadmessage_time < FloatTime() - 20) {
	//				BotAI_BotInitialChat(bs, "followme", EasyClientName(bs->lead_teammate, teammate, sizeof(teammate)), NULL);
	//				trap_BotEnterChat(bs->cs, bs->teammate, CHAT_TELL);
	//				bs->leadmessage_time = FloatTime();
	//			}
	//			//look at the team mate
	//			VectorSubtract(entinfo.origin, bs->origin, dir);
	//			vectoangles(dir, bs->ideal_viewangles);
	//			bs->ideal_viewangles[2] *= 0.5;
	//			//just wait for the team mate
	//			return qfalse;
	//		}
	//	}
	//}
// jmarshall end
	return BotGetLongTermGoal(bs, tfl, retreat, goal);
}

/*
==================
AINode_Respawn
==================
*/
int AINode_Respawn(bot_state_t* bs) {
	bs->input.actionflags |= ACTION_RESPAWN;

	if(!BotIsDead(bs))
	{
		bs->ainode = NULL;
	}
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
	//if (BotFindEnemy(bs, -1)) {
	//	if (BotWantsToRetreat(bs)) {
	//		//keep the current long term goal and retreat
	//		AIEnter_Battle_Retreat(bs, "seek ltg: found enemy");
	//		return qfalse;
	//	}
	//	else {
	//		trap_BotResetLastAvoidReach(bs->ms);
	//		//empty the goal stack
	//		trap_BotEmptyGoalStack(bs->gs);
	//		//go fight
	//		AIEnter_Battle_Fight(bs, "seek ltg: found enemy");
	//		return qfalse;
	//	}
	//}
//	//
//	BotTeamGoals(bs, qfalse);
//	//get the current long term goal
	if (!BotLongTermGoal(bs, bs->tfl, qfalse, &goal)) {
		return qtrue;
	}
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

// jmarshall
	//move towards the goal
	BotMoveToGoal(bs, &goal); //  &moveresult, bs->ms, & goal, bs->tfl);
// jmarshall end
										
										
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

