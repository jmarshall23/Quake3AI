// nav_local.h
//

#pragma once

#include "../external/detour/Include/DetourNavMesh.h"
#include "../external/detour/Include/DetourNavMeshQuery.h"

struct NavMeshFile_t
{
	NavMeshFile_t()
	{
		navMesh = NULL;
		navQuery = NULL;
	}

	~NavMeshFile_t()
	{
		dtFreeNavMesh(navMesh);
		navMesh = NULL;

		dtFreeNavMeshQuery(navQuery);
		navQuery = NULL;
	}

	dtNavMesh* navMesh;
	dtNavMeshQuery* navQuery;
};

struct NavMeshGlobals_t {
	NavMeshGlobals_t()
	{
		navMeshFile = NULL;
	}

	NavMeshFile_t* navMeshFile;
};

extern NavMeshGlobals_t navMeshGlobals;