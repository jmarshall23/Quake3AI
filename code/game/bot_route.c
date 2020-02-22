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

#define MAX_USERMOVE				400

/*
=======================
BotMoveInDirection
=======================
*/
// jmarshall - this doesn't match the original behavior, but basically we get a random point on the navmesh, close to the origin,
//			   and make that the new move to waypoint. The issue is, because this called gets called almost every frame during battle,
//			   it will cause performance issues. I need to take a look and figure out the best way forward for this function.
int BotMoveInDirection(bot_state_t *bs, vec3_t dir, float speed, int type) {
	gentity_t* ent = &g_entities[bs->client];
	bot_input_t* bi = &bs->input;
	vec3_t newMoveToGoal;
	trace_t trace;

	// Run a trace to see if we can just move to the requested location.
	newMoveToGoal[0] = ent->r.currentOrigin[0] + (dir[0] * 25.0f);
	newMoveToGoal[1] = ent->r.currentOrigin[1] + (dir[1] * 25.0f);
	newMoveToGoal[2] = ent->r.currentOrigin[2] + (dir[2] * 25.0f);

	trap_Trace(&trace, ent->r.currentOrigin, NULL, NULL, newMoveToGoal, 0, CONTENTS_SOLID);

	// If we can't directly move to the requested location, find a random point on the navmesh close to us, and just move there.
	// Again this doesn't match the original behavior. 
	if (trace.fraction < 0.9f)
	{
		trap_Nav_GetRandomPointNearPosition(ent->r.currentOrigin, newMoveToGoal, 50.0f);
	}

	VectorSubtract(newMoveToGoal, ent->r.currentOrigin, bi->dir);
	VectorNormalize(bi->dir);
	bi->dir[2] = 0;

	bi->speed = speed; // 200 = walk, 400 = run.

	return 0;
}
// jmarshall end

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
BotNearGoal
=======================
*/
qboolean BotNearGoal(vec3_t p1, vec3_t p2)
{
	vec3_t p1_z, p2_z;

	p1_z[0] = p1[0];
	p1_z[1] = p1[1];
	p1_z[2] = 0;

	p2_z[0] = p2[0];
	p2_z[1] = p2[1];
	p2_z[2] = 0;

	float distToGoal = VectorDistanceSquared(p1_z, p2_z);
	distToGoal = sqrt(distToGoal);
	if (distToGoal <= 50) {
		return qtrue;
	}

	return qfalse;
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


	VectorClear(bs->very_short_term_origin);
	VectorCopy(ent->r.currentOrigin, bs->last_origin);

	if (!BotFindRouteToGoal(bs, goal))
		return;

	if (BotNearGoal(bs->movement_waypoints[bs->currentWaypoint], ent->r.currentOrigin)) {
		bs->currentWaypoint++;
	}

	BotDrawRoute(bs);

	VectorCopy(bs->movement_waypoints[bs->currentWaypoint], move_goal);

	VectorSubtract(move_goal, ent->r.currentOrigin, bi->dir);
	VectorNormalize(bi->dir);
	bi->dir[2] = 0;

	bi->speed = 200; // 200 = walk, 400 = run.
	vectoangles(bi->dir, bs->viewangles);
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