// nav_file.cpp
//

#include "../game/q_shared.hpp"
#include "../qcommon/qcommon.hpp"
#include "../splines/util_str.h"
#include "nav_local.h"

NavMeshGlobals_t navMeshGlobals;

/*
======================
Nav_LoadMesh
======================
*/
qhandle_t Nav_LoadMesh(const char* bspFileName) {
	fileHandle_t navFile;
	navFileHeader_t	header;
	
	idStr navFileName = bspFileName;
	navFileName.SetFileExtension(NAV_FILE_EXTENSION);

	// Open the navmesh file for reading.
	if(FS_FOpenFileRead(navFileName, &navFile, qfalse) == -1) {
		Com_Printf("Nav_LoadMesh: Failed to load NavMesh %s\n", navFileName.c_str());
		return -1;
	}

	// Read and parse the header.
	FS_Read(&header, sizeof(navFileHeader_t), navFile);

	// Check the navmesh file version.
	if(header.version != NAV_FILE_VERSION) {
		Com_Printf("Nav_LoadMesh: File version invalid expected %d found %d\n", NAV_FILE_VERSION, header.version);
		return -1;
	}

	// Check to see if the previous navmesh was cleaned up.
	if(navMeshGlobals.navMeshFile != NULL) {
		delete navMeshGlobals.navMeshFile;
		navMeshGlobals.navMeshFile = NULL;
	}

	// Allocate a new nav mesh file.
	navMeshGlobals.navMeshFile = new NavMeshFile_t();
	navMeshGlobals.navMeshFile->navMesh = dtAllocNavMesh();

	dtNavMeshParams params;
	FS_Read(&params, sizeof(dtNavMeshParams), navFile);

	dtStatus status = navMeshGlobals.navMeshFile->navMesh->init(&params);
	if (dtStatusFailed(status)) {
		return -1;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader_t tileHeader;
		FS_Read(&tileHeader, sizeof(NavMeshTileHeader_t), navFile);

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		FS_Read(data, tileHeader.dataSize, navFile);
		navMeshGlobals.navMeshFile->navMesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	navMeshGlobals.navMeshFile->navQuery = dtAllocNavMeshQuery();
	status = navMeshGlobals.navMeshFile->navQuery->init(navMeshGlobals.navMeshFile->navMesh, 2048);
	if (dtStatusFailed(status))
	{
		Com_Error(ERR_DROP, "Could not init Detour navmesh query");
		return -1;
	}

	// Close the navmesh file.
	FS_FCloseFile(navFile);

	return 1;
}