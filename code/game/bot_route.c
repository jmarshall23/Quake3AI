// bot_route.c
//


#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

#define DISTANCEFACTOR_CROUCH		1.3f		//crouch speed = 100
#define DISTANCEFACTOR_SWIM			1		//should be 0.66, swim speed = 150
#define DISTANCEFACTOR_WALK			0.33f	//walk speed = 300


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
} //end of the function AAS_AreaTravelTime