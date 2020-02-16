// bot_route.c
//


#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

//vec4_t color_white = { 1.00f, 1.00f, 1.00f, 1.00f };

vec4_t color_waypoints[8] = {
	{ 1.00f, 1.00f, 1.00f, 1.00f },
	{ 1.00f, 1.00f, 0.00f, 1.00f },
	{ 1.00f, 0.00f, 1.00f, 1.00f },
	{ 0.00f, 1.00f, 1.00f, 1.00f },
	{ 1.00f, 0.00f, 1.00f, 1.00f },
	{ 0.00f, 1.00f, 0.00f, 1.00f },
	{ 0.00f, 1.00f, 0.00f, 1.00f },
};

vec4_t color_next_waypoint = { 1.0f, 0.0f, 0.0f, 1.0f };

#define DISTANCEFACTOR_CROUCH		1.3f		//crouch speed = 100
#define DISTANCEFACTOR_SWIM			1		//should be 0.66, swim speed = 150
#define DISTANCEFACTOR_WALK			0.33f	//walk speed = 300

/*
=======================
BotDrawRoute
=======================
*/
void BotDrawRoute(bot_state_t *bs) {
	{
		box_t box;
		InitBox(&box);
		AddPointToBox(&box, g_entities[bs->client].r.currentOrigin);
		ExpandBox(&box, 12.0f);

		trap_Debug_DrawDebugBox(color_waypoints[bs->client % 8], &box, 0);
	}

	for(int i = bs->currentWaypoint; i < bs->numMovementWaypoints; i++)
	{
		box_t box;
		InitBox(&box);
		AddPointToBox(&box, bs->movement_waypoints[i]);
		ExpandBox(&box, 3.0f);

		if(i == bs->currentWaypoint)
			trap_Debug_DrawDebugBox(color_next_waypoint, &box, 0);
		else
			trap_Debug_DrawDebugBox(color_waypoints[bs->client % 8], &box, 0);
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
	vec3_t move_goal;
	
	bi = &bs->input;

	vec3_t movedelta;
	VectorSubtract(ent->r.currentOrigin, bs->last_origin, movedelta);
	float currentMoveSpeed = VectorLength(movedelta);
	if (currentMoveSpeed <= 0.05f) {
		bs->stuck_time++;
	}
	else {
		bs->stuck_time = 0;
	}

	if(bs->stuck_time > 30) {
		if (VectorLength(bs->very_short_term_origin) <= 0)
		{
			trap_Nav_GetRandomPointNearPosition(ent->r.currentOrigin, bs->very_short_term_origin, 50.0f);			
		}

		float distToGoal = VectorDistanceSquared(move_goal, ent->r.currentOrigin);
		distToGoal = sqrt(distToGoal);
		if(distToGoal > 60.0f) {
			VectorClear(bs->very_short_term_origin);
		}
		else
		{
			if (distToGoal <= 10) {
				VectorClear(bs->very_short_term_origin);
			}
		}

		VectorCopy(bs->very_short_term_origin, move_goal);
	}
	else 
	{
		VectorClear(bs->very_short_term_origin);
		VectorCopy(ent->r.currentOrigin, bs->last_origin);

		if (!BotFindRouteToGoal(bs, goal))
			return;

		float distToGoal = VectorDistanceSquared(bs->movement_waypoints[bs->currentWaypoint], ent->r.currentOrigin);
		distToGoal = sqrt(distToGoal);
		if (distToGoal <= 50) {
			bs->currentWaypoint++;
		}

		BotDrawRoute(bs);

		if (bs->currentWaypoint >= bs->numMovementWaypoints)
			return;

		VectorCopy(bs->movement_waypoints[bs->currentWaypoint], move_goal);
	}

	VectorSubtract(move_goal, ent->r.currentOrigin, bi->dir);
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