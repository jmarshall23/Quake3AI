// nav_query.cpp
//

#include "../game/q_shared.hpp"
#include "../qcommon/qcommon.hpp"
#include "nav_local.h"

/*
===================
Nav_GetRandomPointNearPosition
===================
*/
void Nav_GetRandomPointNearPosition(vec3_t point, vec3_t randomPoint, float radius) {
	vec3_t extents = { 280, 480, 280 };
	dtQueryFilter filter;
	dtPolyRef polyRef;
	vec3_t nearestPoint;
	vec3_t randomPointRaw;
	vec3_t pointConv;
	QuakeCoordsToNav(point, pointConv);

	navMeshGlobals.navMeshFile->navQuery->findNearestPoly(pointConv, extents, &filter, &polyRef, nearestPoint);

	dtPolyRef randomPolyRef;
	navMeshGlobals.navMeshFile->navQuery->findRandomPointAroundCircle(polyRef, point, radius, &filter, FRand, &randomPolyRef, randomPointRaw);
	NavToQuakeCoords(randomPointRaw, randomPoint);
}

/*
===================
rvmNavFileLocal::GetPathBetweenPoints
===================
*/
int Nav_GetPathBetweenPoints(vec3_t p1, vec3_t p2, vec3_t *waypoints) {
	vec3_t extents = { 280, 480, 280 };

	dtQueryFilter filter;
	dtPolyRef startPolyRef, endPolyRef;
	vec3_t startNearestPt, endNearestPt;

	vec3_t p1Conv, p2Conv;

	QuakeCoordsToNav(p1, p1Conv);
	QuakeCoordsToNav(p2, p2Conv);

		
	navMeshGlobals.navMeshFile->navQuery->findNearestPoly(p1Conv, extents, &filter, &startPolyRef, startNearestPt);
	navMeshGlobals.navMeshFile->navQuery->findNearestPoly(p2Conv, extents, &filter, &endPolyRef, endNearestPt);


	dtPolyRef path[NAV_MAX_PATHSTEPS];
	int numPaths = -1;
	navMeshGlobals.navMeshFile->navQuery->findPath(startPolyRef, endPolyRef, p1Conv, p2Conv, &filter, &path[0], &numPaths, NAV_MAX_PATHSTEPS);

	if (numPaths <= 0)
		return -1;

	if(numPaths >= NAV_MAX_PATHSTEPS-1) {		
		Com_Printf("WARNING: Nav_GetPathBetweenPoints: Too many pathsteps!\n");
		numPaths = NAV_MAX_PATHSTEPS-1;
	}

	for (int i = 0; i < numPaths; i++)
	{
		vec3_t origin;
		bool ignored;

		const dtMeshTile* meshTile;
		const dtPoly* meshPoly;

		navMeshGlobals.navMeshFile->navMesh->getTileAndPolyByRef(path[i], &meshTile, &meshPoly);

		//idBounds polyBounds;
		//polyBounds.Clear();

		vec3_t polyBoundsMins, polyBoundsMaxs;
		ClearBounds(polyBoundsMins, polyBoundsMaxs);

		for (int i = 0; i < meshPoly->vertCount; i++)
		{
			vec3_t vertex = { meshTile->verts[(meshPoly->verts[i] * 3) + 0], meshTile->verts[(meshPoly->verts[i] * 3) + 1], meshTile->verts[(meshPoly->verts[i] * 3) + 2] };
			//polyBounds.AddPoint(vertex);
			AddPointToBounds(vertex, polyBoundsMins, polyBoundsMaxs);
		}

		vec3_t polyBoundsCenter;
		GetCenterOfBounds(polyBoundsMins, polyBoundsMaxs, polyBoundsCenter);

		polyBoundsCenter[0] += FRandRange(-50.0f, 50.0f);
		polyBoundsCenter[1] += FRandRange(-50.0f, 50.0f);

		navMeshGlobals.navMeshFile->navQuery->closestPointOnPoly(path[i], polyBoundsCenter, origin, &ignored);
		NavToQuakeCoords(origin, waypoints[i]);
	}

	VectorCopy(p2, waypoints[numPaths]);

	return numPaths + 1;
}