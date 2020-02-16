// bot_route.c
//


#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

vec4_t color_white = { 1.00f, 1.00f, 1.00f, 1.00f };

#define DISTANCEFACTOR_CROUCH		1.3f		//crouch speed = 100
#define DISTANCEFACTOR_SWIM			1		//should be 0.66, swim speed = 150
#define DISTANCEFACTOR_WALK			0.33f	//walk speed = 300

/*
=======================
BotDrawRoute
=======================
*/
void BotDrawRoute(bot_state_t *bs) {
	for(int i = 0; i < bs->numMovementWaypoints; i++)
	{
		box_t box;
		InitBox(&box);
		AddPointToBox(&box, bs->movement_waypoints[i]);
		ExpandBox(&box, 10.0f);
		trap_Debug_DrawDebugBox(color_white, &box, 0);
	}
}

/*
=======================
BotFindRouteToGoal
=======================
*/
qboolean BotFindRouteToGoal(bot_state_t* bs, bot_goal_t *goal) {
	gentity_t* ent = &level.gentities[bs->client];
	
	if (VectorCompare(bs->currentMoveGoal, goal->origin))
		return qtrue;

	bs->currentgoal = *goal;
	VectorCopy(goal->origin, bs->currentMoveGoal);
	bs->currentWaypoint = 0;
	bs->numMovementWaypoints = trap_Nav_GetPathBetweenPoints(ent->r.currentOrigin, goal->origin, bs->movement_waypoints);

	if (bs->numMovementWaypoints <= 0)
		return qfalse;

	return qtrue;
}

/*
=======================
BotMoveToGoal
=======================
*/
void BotMoveToGoal(bot_state_t *bs, bot_goal_t *goal) {
	bot_input_t* bi;
	gentity_t* ent = &level.gentities[bs->client];
	
	bi = &bs->input;

	if (!BotFindRouteToGoal(bs, goal))
		return;

	float distToGoal = VectorDistanceSquared(bs->movement_waypoints[bs->currentWaypoint], ent->r.currentOrigin);
	if (distToGoal <= 70) {
		bs->currentWaypoint++;
	}

	BotDrawRoute(bs);

	if (bs->currentWaypoint >= bs->numMovementWaypoints)
		return;

	VectorSubtract(bs->movement_waypoints[bs->currentWaypoint], ent->r.currentOrigin, bi->dir);
	VectorNormalize(bi->dir);
	bi->dir[2] = 0;

	bi->speed = 200; // 200 = walk, 400 = run.
	vectoangles(bi->dir, bi->viewangles);
}

/*
=================
BotTravelTime
=================
*/
unsigned short int BotTravelTime(vec3_t start, vec3_t end)
{
	int intdist;
	float dist;
	vec3_t dir;

	VectorSubtract(start, end, dir);
	dist = VectorLength(dir);
	
	//if crouch only area
	dist *= DISTANCEFACTOR_WALK;
	
	intdist = (int)dist;
	//make sure the distance isn't zero
	if (intdist <= 0) intdist = 1;
	return intdist;
}