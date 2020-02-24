// bg_splines.cpp
//


#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

/*
=============
BG_Find_PathCorner
=============
*/
pathCorner_t* BG_Find_PathCorner(const char* match) {
	int i;

	for (i = 0; i < numPathCorners; i++) {
		if (!Q_stricmp(pathCorners[i].name, match)) {
			return &pathCorners[i];
		}
	}

	return NULL;
}

/*
=============
BG_AddPathCorner
=============
*/
void BG_AddPathCorner(const char* name, vec3_t origin) {
	if (numPathCorners >= MAX_PATH_CORNERS) {
		Com_Error(ERR_DROP, "MAX PATH CORNERS (%i) hit", MAX_PATH_CORNERS);
	}

	VectorCopy(origin, pathCorners[numPathCorners].origin);
	Q_strncpyz(pathCorners[numPathCorners].name, name, 64);
	numPathCorners++;
}

/*
=============
BG_Find_Spline
=============
*/
splinePath_t* BG_Find_Spline(const char* match) {
	int i;

	for (i = 0; i < numSplinePaths; i++) {
		if (!Q_stricmp(splinePaths[i].point.name, match)) {
			return &splinePaths[i];
		}
	}

	return NULL;
}

splinePath_t* BG_AddSplinePath(const char* name, const char* target, vec3_t origin) {
	splinePath_t* spline;
	if (numSplinePaths >= MAX_SPLINE_PATHS) {
		Com_Error(ERR_DROP, "MAX SPLINES (%i) hit", MAX_SPLINE_PATHS);
	}

	spline = &splinePaths[numSplinePaths];

	memset(spline, 0, sizeof(splinePath_t));

	VectorCopy(origin, spline->point.origin);

	Q_strncpyz(spline->point.name, name, 64);
	Q_strncpyz(spline->strTarget, target ? target : "", 64);

	spline->numControls = 0;

	numSplinePaths++;

	return spline;
}

void BG_AddSplineControl(splinePath_t* spline, const char* name) {
	if (spline->numControls >= MAX_SPLINE_CONTROLS) {
		Com_Error(ERR_DROP, "MAX SPLINE CONTROLS (%i) hit", MAX_SPLINE_CONTROLS);
	}

	Q_strncpyz(spline->controls[spline->numControls].name, name, 64);

	spline->numControls++;
}

float BG_SplineLength(splinePath_t* pSpline) {
	float i;
	float granularity = 0.01f;
	float dist = 0;
	//	float tension;
	vec3_t vec[2];
	vec3_t lastPoint;
	vec3_t result;

	for (i = 0; i <= 1.f; i += granularity) {
		/*		if(pSpline->isStart) {
					tension = 1 - sin((1 - i) * M_PI * 0.5f);
				} else if(pSpline->isEnd) {
					tension = sin(i * M_PI * 0.5f);
				} else {
					tension = i;
				}*/

		BG_CalculateSpline_r(pSpline, vec[0], vec[1], i);
		VectorSubtract(vec[1], vec[0], result);
		VectorMA(vec[0], i, result, result);

		if (i != 0) {
			VectorSubtract(result, lastPoint, vec[0]);
			dist += VectorLength(vec[0]);
		}

		VectorCopy(result, lastPoint);
	}

	return dist;
}

void BG_BuildSplinePaths() {
	int i, j;
	pathCorner_t* pnt;
	splinePath_t* spline, * st;

	for (i = 0; i < numSplinePaths; i++) {
		spline = &splinePaths[i];

		if (*spline->strTarget) {
			for (j = 0; j < spline->numControls; j++) {
				pnt = BG_Find_PathCorner(spline->controls[j].name);

				if (!pnt) {
					Com_Printf("^1Cant find control point (%s) for spline (%s)\n", spline->controls[j].name, spline->point.name);
					// Gordon: Just changing to a warning for now, easier for region compiles...
					continue;

				}
				else {
					VectorCopy(pnt->origin, spline->controls[j].origin);
				}
			}

			st = BG_Find_Spline(spline->strTarget);
			if (!st) {
				Com_Printf("^1Cant find target point (%s) for spline (%s)\n", spline->strTarget, spline->point.name);
				// Gordon: Just changing to a warning for now, easier for region compiles...
				continue;
			}

			spline->next = st;

			spline->length = BG_SplineLength(spline);
			BG_ComputeSegments(spline);
		}
	}

	for (i = 0; i < numSplinePaths; i++) {
		spline = &splinePaths[i];

		if (spline->next) {
			spline->next->prev = spline;
		}
	}
}

splinePath_t* BG_GetSplineData(int number, qboolean* backwards) {
	if (number < 0) {
		*backwards = qtrue;
		number = -number;
	}
	else {
		*backwards = qfalse;
	}
	number--;

	if (number < 0 || number >= numSplinePaths) {
		return NULL;
	}

	return &splinePaths[number];
}


void BG_CalculateSpline_r(splinePath_t* spline, vec3_t out1, vec3_t out2, float tension) {
	vec3_t points[18];
	int i;
	int count = spline->numControls + 2;
	vec3_t dist;

	VectorCopy(spline->point.origin, points[0]);
	for (i = 0; i < spline->numControls; i++) {
		VectorCopy(spline->controls[i].origin, points[i + 1]);
	}
	if (!spline->next) {
		return;
		//		Com_Error( ERR_DROP, "Spline (%s) with no target referenced", spline->point.name );
	}
	VectorCopy(spline->next->point.origin, points[i + 1]);


	while (count > 2) {
		for (i = 0; i < count - 1; i++) {
			VectorSubtract(points[i + 1], points[i], dist);
			VectorMA(points[i], tension, dist, points[i]);
		}
		count--;
	}

	VectorCopy(points[0], out1);
	VectorCopy(points[1], out2);
}

qboolean BG_TraverseSpline(float* deltaTime, splinePath_t** pSpline) {
	float dist;

	while ((*deltaTime) > 1) {
		(*deltaTime) -= 1;
		dist = (*pSpline)->length * (*deltaTime);

		if (!(*pSpline)->next || !(*pSpline)->next->length) {
			return qfalse;
			//			Com_Error( ERR_DROP, "Spline path end passed (%s)", (*pSpline)->point.name );
		}

		(*pSpline) = (*pSpline)->next;
		*deltaTime = dist / (*pSpline)->length;
	}

	while ((*deltaTime) < 0) {
		dist = -((*pSpline)->length * (*deltaTime));

		if (!(*pSpline)->prev || !(*pSpline)->prev->length) {
			return qfalse;
			//			Com_Error( ERR_DROP, "Spline path end passed (%s)", (*pSpline)->point.name );
		}

		(*pSpline) = (*pSpline)->prev;
		(*deltaTime) = 1 - (dist / (*pSpline)->length);
	}

	return qtrue;
}


/*
================
BG_RaySphereIntersection

================
*/

qboolean BG_RaySphereIntersection(float radius, vec3_t origin, splineSegment_t* path, float* t0, float* t1) {
	vec3_t v;
	float b, c, d;

	VectorSubtract(path->start, origin, v);

	b = 2 * DotProduct(v, path->v_norm);
	c = DotProduct(v, v) - (radius * radius);

	d = (b * b) - (4 * c);
	if (d < 0) {
		return qfalse;
	}
	d = sqrt(d);

	*t0 = (-b + d) * 0.5f;
	*t1 = (-b - d) * 0.5f;

	return qtrue;
}

void BG_LinearPathOrigin2(float radius, splinePath_t** pSpline, float* deltaTime, vec3_t result, qboolean backwards) {
	qboolean first = qtrue;
	float t = 0.f;
	int i = floor((*deltaTime) * (MAX_SPLINE_SEGMENTS));
	float frac;
	//	int x = 0;
	//	splinePath_t* start = *pSpline;

	if (i >= MAX_SPLINE_SEGMENTS) {
		i = MAX_SPLINE_SEGMENTS - 1;
		frac = 1.f;
	}
	else {
		frac = (((*deltaTime) * (MAX_SPLINE_SEGMENTS)) - i);
	}

	while (qtrue) {
		float t0, t1;

		while (qtrue) {
			if (BG_RaySphereIntersection(radius, result, &(*pSpline)->segments[i], &t0, &t1)) {
				qboolean found = qfalse;

				t0 /= (*pSpline)->segments[i].length;
				t1 /= (*pSpline)->segments[i].length;

				if (first) {
					if (radius < 0) {
						if (t0 < frac && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						}
						else if (t1 < frac) {
							t = t1;
							found = qtrue;
						}
					}
					else {
						if (t0 > frac && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						}
						else if (t1 > frac) {
							t = t1;
							found = qtrue;
						}
					}
				}
				else {
					if (radius < 0) {
						if (t0 < t1 && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						}
						else {
							t = t1;
							found = qtrue;
						}
					}
					else {
						if (t0 > t1 && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						}
						else {
							t = t1;
							found = qtrue;
						}
					}
				}

				if (found) {
					if (t >= 0.f && t <= 1.f) {
						*deltaTime = (i / (float)(MAX_SPLINE_SEGMENTS)) + (t / (float)(MAX_SPLINE_SEGMENTS));
						VectorMA((*pSpline)->segments[i].start, t * (*pSpline)->segments[i].length, (*pSpline)->segments[i].v_norm, result);
						return;
					}
				}
				found = qfalse;
			}

			first = qfalse;
			if (radius < 0) {
				i--;
				if (i < 0) {
					i = MAX_SPLINE_SEGMENTS - 1;
					break;
				}
			}
			else {
				i++;
				if (i >= MAX_SPLINE_SEGMENTS) {
					i = 0;
					break;
				}
			}
		}

		if (radius < 0) {
			if (!(*pSpline)->prev) {
				return;
				//				Com_Error( ERR_DROP, "End of spline reached (%s)\n", start->point.name );
			}
			*pSpline = (*pSpline)->prev;
		}
		else {
			if (!(*pSpline)->next) {
				return;
				//				Com_Error( ERR_DROP, "End of spline reached (%s)\n", start->point.name );
			}
			*pSpline = (*pSpline)->next;
		}
	}
}

void BG_ComputeSegments(splinePath_t* pSpline) {
	int i;
	float granularity = 1 / ((float)(MAX_SPLINE_SEGMENTS));
	vec3_t vec[4];

	for (i = 0; i < MAX_SPLINE_SEGMENTS; i++) {
		BG_CalculateSpline_r(pSpline, vec[0], vec[1], i * granularity);
		VectorSubtract(vec[1], vec[0], pSpline->segments[i].start);
		VectorMA(vec[0], i * granularity, pSpline->segments[i].start, pSpline->segments[i].start);

		BG_CalculateSpline_r(pSpline, vec[2], vec[3], (i + 1) * granularity);
		VectorSubtract(vec[3], vec[2], vec[0]);
		VectorMA(vec[2], (i + 1) * granularity, vec[0], vec[0]);

		VectorSubtract(vec[0], pSpline->segments[i].start, pSpline->segments[i].v_norm);
		pSpline->segments[i].length = VectorLength(pSpline->segments[i].v_norm);
		VectorNormalize(pSpline->segments[i].v_norm);
	}
}