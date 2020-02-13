// main.cpp
//

#include "../game/q_shared.h"
#include "../tool_lib/bspfile.h"
#include "../tool_lib/cmdlib.h"
#include "../external/recast/Include/Recast.h"
#include "../external/detour/Include/DetourNavMesh.h"
#include "../external/detour/Include/DetourNavMeshBuilder.h"
#include "../splines/util_str.h"

#include <vector>

std::vector<drawVert_t> mapGeometry;

const float m_cellSize = 2.0f;
const float m_cellHeight = 5.0f;
const float m_agentHeight = 2.0f;
const float m_agentRadius = 20.6f;
const float m_agentMaxClimb = 20.9f;
const float m_agentMaxSlope = 45.0f;
const float m_regionMinSize = 8.0f;
const float m_regionMergeSize = 20.0f;
const float m_edgeMaxLen = 12.0f;
const float m_edgeMaxError = 1.3f;
const float m_vertsPerPoly = 6.0f;
const float m_detailSampleDist = 3.0f;
const float m_detailSampleMaxError = 1.0f;

#define BOT_GET_VALUE(x) x

class idFileWrite {
public:
	idFileWrite(const char *filePath) {
		file = fopen(filePath, "wb");
	}
	~idFileWrite() {
		fclose(file);
	}
	void Write(void* buffer, int length) {
		fwrite(buffer, 1, length, file);
	}
	void Write(const void* buffer, int length) {
		fwrite(buffer, 1, length, file);
	}
private:
	FILE* file;
};

/*
============
WriteMapGeometry
============
*/
void WriteMapGeometry(rcPolyMeshDetail* mesh) {
	FILE* objFile = fopen("navmesh.obj", "w");

	std::vector<float> debugVertexes;
	for (int meshId = 0; meshId < mesh->nmeshes; meshId++)
	{
		int startVertex = mesh->meshes[(meshId * 4) + 0];
		int numVertexes = mesh->meshes[(meshId * 4) + 1];
		int startTris = mesh->meshes[(meshId * 4) + 2];
		int numTris = mesh->meshes[(meshId * 4) + 3];

		for (int i = 0; i < numTris; i++)
		{
			for (int d = 0; d < 3; d++)
			{
				int vertexId = startVertex + mesh->tris[((startTris + i) * 4) + d];
				debugVertexes.push_back(mesh->verts[(vertexId * 3) + 0]);
				debugVertexes.push_back(mesh->verts[(vertexId * 3) + 1]);
				debugVertexes.push_back(mesh->verts[(vertexId * 3) + 2]);
			}
		}
#if 0
		for (int i = 0; i < numVertexes; i++)
		{

			int vertexId = startVertex + i;
			debugVertexes.push_back(mesh->verts[(vertexId * 3) + 0]);
			debugVertexes.push_back(mesh->verts[(vertexId * 3) + 1]);
			debugVertexes.push_back(mesh->verts[(vertexId * 3) + 2]);
		}
#endif
	}

	for (int i = 0; i < debugVertexes.size() / 3; i++)
	{
		float* vertex = &debugVertexes[(i * 3)];
		fprintf(objFile, "v %f %f %f\n", vertex[0], vertex[2], -vertex[1]);
	}

	for (int i = 0; i < debugVertexes.size() / 3; i += 3)
	{
		fprintf(objFile, "f %d %d %d\n", i + 1, i + 2, i + 3);
	}

	fclose(objFile);
}

/*
============
WriteMapGeometry
============
*/
void WriteMapGeometry(void) {
	FILE* objFile = fopen("map.obj", "w");

	for (int i = 0; i < mapGeometry.size(); i++)
	{
		drawVert_t* vert = &mapGeometry[i];
		fprintf(objFile, "v %f %f %f\n", vert->xyz[0], vert->xyz[1], vert->xyz[2]);
	}

	for (int i = 1; i < mapGeometry.size() + 1; i += 3)
	{
		fprintf(objFile, "f %d %d %d\n", i + 0, i + 1, i + 2);
	}

	fclose(objFile);
}

/*
===================
rvmNavFileLocal::WriteNavFile
===================
*/
void WriteNavFile(const char* name, rcPolyMesh* mesh, rcPolyMeshDetail* detailMesh, int mapCRC) {
	idStr navFileName;
	navFileName = "maps/";
	navFileName += name;
	navFileHeader_t header;

	navFileName.SetFileExtension(NAV_FILE_EXTENSION);

	// Open the nav file for writing.
	idFileWrite *file = new idFileWrite(navFileName);

	// Fill in the header.
	header.version = NAV_FILE_VERSION;
	header.mapCRC = mapCRC;

	// Write out the navmesh data.
	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = mesh->verts;
	params.vertCount = mesh->nverts;
	params.polys = mesh->polys;
	params.polyAreas = mesh->areas;
	params.polyFlags = mesh->flags;
	params.polyCount = mesh->npolys;
	params.nvp = mesh->nvp;
	params.detailMeshes = detailMesh->meshes;
	params.detailVerts = detailMesh->verts;
	params.detailVertsCount = detailMesh->nverts;
	params.detailTris = detailMesh->tris;
	params.detailTriCount = detailMesh->ntris;
	//params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
	//params.offMeshConRad = m_geom->getOffMeshConnectionRads();
	//params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
	//params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
	//params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
	//params.offMeshConUserID = m_geom->getOffMeshConnectionId();
	//params.offMeshConCount = m_geom->getOffMeshConnectionCount();
	params.walkableHeight = BOT_GET_VALUE(m_agentHeight);
	params.walkableRadius = BOT_GET_VALUE(m_agentRadius);
	params.walkableClimb = BOT_GET_VALUE(m_agentMaxClimb);
	rcVcopy(params.bmin, mesh->bmin);
	rcVcopy(params.bmax, mesh->bmax);
	params.cs = BOT_GET_VALUE(m_cellSize);
	params.ch = BOT_GET_VALUE(m_cellHeight);
	params.buildBvTree = false;

	unsigned char* navData = 0;
	int navDataSize = 0;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		printf("ERROR: Could not build Detour navmesh.");
		return;
	}

	dtNavMesh* dtmesh = dtAllocNavMesh();
	if (!mesh)
	{
		printf("ERROR: Could not create Detour navmesh");
		return;
	}

	dtStatus status;

	status = dtmesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
	{
		printf("ERROR: Could not init Detour navmesh");
		return;
	}

	header.numTiles = 0;
	for (int i = 0; i < dtmesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = dtmesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}

	// Write out the header.
	file->Write(&header, sizeof(navFileHeader_t));

	file->Write(dtmesh->getParams(), sizeof(dtNavMeshParams));

	// Store tiles.
	for (int i = 0; i < dtmesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = dtmesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader_t tileHeader;
		tileHeader.tileRef = dtmesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		file->Write(&tileHeader, sizeof(tileHeader));

		file->Write(tile->data, tile->dataSize);
	};
}

/*
============
BuildMapGeometry
============
*/
void BuildMapGeometry(void) {
	for (int i = 0; i < numDrawSurfaces; i++)
	{
		dsurface_t* surface = &drawSurfaces[i];

		if (surface->surfaceType == MST_PATCH || surface->surfaceType == MST_FLARE)
		{
			continue;
		}

		for (int d = 0; d < surface->numIndexes; d++)
		{
			int vertexId = surface->firstVert + drawIndexes[surface->firstIndex + d];
			mapGeometry.push_back(drawVerts[vertexId]);
		}
	}

	_printf("...Generated %d vertexes\n", mapGeometry.size());
}

void BuildNavMesh(const char * mapName)
{
	float bmin[3];
	float bmax[3];
	rcConfig m_cfg;
	rcContext* m_ctx = new rcContext();

	float* verts = new float[mapGeometry.size() * 3];
	for (int n = 0; n < mapGeometry.size(); n++)
	{
		vec3_t xyz;
		xyz[0] = mapGeometry[n].xyz[0];
		xyz[1] = mapGeometry[n].xyz[1];
		xyz[2] = mapGeometry[n].xyz[2];

		vec3_t conv_xyz;
		QuakeCoordsToNav(xyz, conv_xyz);

		verts[(n * 3) + 0] = conv_xyz[0];
		verts[(n * 3) + 1] = conv_xyz[1];
		verts[(n * 3) + 2] = conv_xyz[2];
	}

	//int numTris = mapGeometry.indexes.Num() / 3;
	//
	//int* indexes = new int[mapGeometry.indexes.Num()];
	//memcpy(indexes, mapGeometry.indexes.Ptr(), sizeof(int) * mapGeometry.indexes.Num());

	int numTris = mapGeometry.size() / 3;

	int* indexes = new int[mapGeometry.size()];
	for (int n = 0; n < mapGeometry.size(); n++) {
		indexes[n] = n;
	}

	rcCalcBounds(verts, mapGeometry.size(), bmin, bmax);

	printf("...RcCalcBounds (%f %f %f) (%f %f %f)\n", bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]);

	//
	// Step 1. Initialize build config.
	//

	printf("1/7: Initialize build config.\n");

	// Grab the navmesh options decl and grab the options from it. 
	//const idDeclEntityDef* botNavDecl = (const idDeclEntityDef*)declManager->FindType(DECL_ENTITYDEF, "navBot");

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = BOT_GET_VALUE(m_cellSize);
	m_cfg.ch = BOT_GET_VALUE(m_cellHeight);
	m_cfg.walkableSlopeAngle = BOT_GET_VALUE(m_agentMaxSlope);
	m_cfg.walkableHeight = (int)ceilf(BOT_GET_VALUE(m_agentHeight) / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(BOT_GET_VALUE(m_agentMaxClimb) / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(BOT_GET_VALUE(m_agentRadius) / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(BOT_GET_VALUE(m_edgeMaxLen) / BOT_GET_VALUE(m_cellSize));
	m_cfg.maxSimplificationError = BOT_GET_VALUE(m_edgeMaxError);
	m_cfg.minRegionArea = (int)rcSqr(BOT_GET_VALUE(m_regionMinSize));		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(BOT_GET_VALUE(m_regionMergeSize));	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)BOT_GET_VALUE(m_vertsPerPoly);
	m_cfg.detailSampleDist = BOT_GET_VALUE(m_detailSampleDist) < 0.9f ? 0 : BOT_GET_VALUE(m_cellSize) * BOT_GET_VALUE(m_detailSampleDist);
	m_cfg.detailSampleMaxError = BOT_GET_VALUE(m_cellHeight) * BOT_GET_VALUE(m_detailSampleMaxError);

	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	printf("...Grid Size %dx%d\n", m_cfg.width, m_cfg.height);

	//
	// Step 2. Rasterize input polygon soup.
	//
	printf("2/7: Rasterize input polygon soup..\n");
	rcHeightfield* m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		printf("buildNavigation: Out of memory 'solid'.");
		return;
	}

	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		printf("buildNavigation: Could not create solid heightfield.");
		return;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	unsigned char* m_triareas = new unsigned char[numTris];
	if (!m_triareas)
	{
		printf("buildNavigation: Out of memory 'm_triareas' (%d).", numTris);
		return;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, numTris * sizeof(unsigned char));
	rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, verts, mapGeometry.size(), indexes, numTris, m_triareas);
	rcRasterizeTriangles(m_ctx, verts, mapGeometry.size(), indexes, m_triareas, numTris, *m_solid, m_cfg.walkableClimb);

	delete[] m_triareas;
	m_triareas = 0;

	//
	// Step 3. Filter walkables surfaces.
	//
	printf("3/7: Filter walkables surfaces...\n");
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	// Step 4. Partition walkable surface to simple regions.
	//
	printf("4/7: Partition walkable surface to simple regions...\n");
	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		printf("buildNavigation: Out of memory 'chf'.");
	}
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		printf("buildNavigation: Could not build compact data.");
		return;
	}

	rcFreeHeightField(m_solid);
	m_solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		printf("buildNavigation: Could not erode.");
	}

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(m_ctx, *m_chf))
	{
		printf("buildNavigation: Could not build distance field.");
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	{
		printf("buildNavigation: Could not build regions.");
	}

	rcContourSet* m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		printf("buildNavigation: Out of memory 'cset'.");
	}

	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		printf("buildNavigation: Could not create contours.");
	}

	printf("m_cset->nconts=%i\n", m_cset->nconts);

	//
	// Step 6. Build polygons mesh from contours.
	//
	printf("6/7: Build polygons mesh from contours..\n");
	// Build polygon navmesh from the contours.
	rcPolyMesh* m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		printf("buildNavigation: Out of memory 'pmesh'.");
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		printf("buildNavigation: Could not triangulate contours.");
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	printf("7/7: Create detail mesh..\n");
	rcPolyMeshDetail* m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		printf("buildNavigation: Out of memory 'pmdtl'.");
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		printf("buildNavigation: Could not build detail mesh.");
	}

	WriteMapGeometry(m_dmesh);

	WriteNavFile(mapName, m_pmesh, m_dmesh, 0);
}

int main(int argc, char **argv) {
	printf("bspnav (c) 2020 Blackenmace Studios LLC\n");

	if (argc != 2)
	{
		_printf("usage: bsp2nav <map_name>\n");
		return -1;
	}

	const char* bspMapName = argv[1];

	_printf("Loading Map %s\n", bspMapName);
	LoadBSPFile(bspMapName);

	_printf("...Building Map Geometry\n");
	BuildMapGeometry();

	_printf("...Writing Map Geometry\n");
	WriteMapGeometry();

	_printf("Building Nav Mesh...\n");
	BuildNavMesh(bspMapName);

	return 0;
}