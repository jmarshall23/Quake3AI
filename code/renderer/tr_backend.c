/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "tr_local.h"

// jmarshall
#include "simplex.h"
// jmarshall end

backEndData_t	*backEndData[SMP_FRAMES];
backEndState_t	backEnd;

void RB_ShowDebugLines(void);
void RB_ShowDebugText(void);

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


/*
** GL_Bind
*/
void GL_Bind( image_t *image ) {
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		glBindTexture (GL_TEXTURE_2D, texnum);
	}
}

/*
** GL_SelectTexture
*/
void GL_SelectTexture( int unit )
{
	if ( glState.currenttmu == unit )
	{
		return;
	}

	if ( unit == 0 )
	{
		glActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE0_ARB )\n" );
		glClientActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE0_ARB )\n" );
	}
	else if ( unit == 1 )
	{
		glActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE1_ARB )\n" );
		glClientActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE1_ARB )\n" );
	} else {
		ri.Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GL_BindMultitexture
*/
void GL_BindMultitexture( image_t *image0, GLuint env0, image_t *image1, GLuint env1 ) {
	int		texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[1] != texnum1 ) {
		GL_SelectTexture( 1 );
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		glBindTexture( GL_TEXTURE_2D, texnum1 );
	}
	if ( glState.currenttextures[0] != texnum0 ) {
		GL_SelectTexture( 0 );
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		glBindTexture( GL_TEXTURE_2D, texnum0 );
	}
}


/*
** GL_Cull
*/
void GL_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}

	glState.faceCulling = cullType;

	if ( cullType == CT_TWO_SIDED ) 
	{
		glDisable( GL_CULL_FACE );
	} 
	else 
	{
		glEnable( GL_CULL_FACE );

		if ( cullType == CT_BACK_SIDED )
		{
			if ( backEnd.viewParms.isMirror )
			{
				glCullFace( GL_FRONT );
			}
			else
			{
				glCullFace( GL_BACK );
			}
		}
		else
		{
			if ( backEnd.viewParms.isMirror )
			{
				glCullFace( GL_BACK );
			}
			else
			{
				glCullFace( GL_FRONT );
			}
		}
	}
}

/*
** GL_TexEnv
*/
void GL_TexEnv( int env )
{
	if ( env == glState.texEnv[glState.currenttmu] )
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch ( env )
	{
	case GL_MODULATE:
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		break;
	case GL_REPLACE:
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		break;
	case GL_DECAL:
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		break;
	case GL_ADD:
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
		break;
	default:
		ri.Error( ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env );
		break;
	}
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( unsigned long stateBits )
{
	unsigned long diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			glDepthFunc( GL_EQUAL );
		}
		else
		{
			glDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		GLenum srcFactor, dstFactor;

		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
		{
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				srcFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				dstFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			glEnable( GL_BLEND );
			glBlendFunc( srcFactor, dstFactor );
		}
		else
		{
			glDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			glDepthMask( GL_TRUE );
		}
		else
		{
			glDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			glDisable( GL_DEPTH_TEST );
		}
		else
		{
			glEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS )
	{
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
			glDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		default:
			assert( 0 );
			break;
		}
	}

	glState.glStateBits = stateBits;
}



/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	float		c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = ( backEnd.refdef.time & 255 ) / 255.0f;
	glClearColor( c, c, c, 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	backEnd.isHyperspace = qtrue;
}


static void SetViewportAndScissor( void ) {
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf( backEnd.viewParms.projectionMatrix );
	glMatrixMode(GL_MODELVIEW);

	// set the window clipping
	glViewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
	glScissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView (void) {
	int clearBits = 0;

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		glFinish ();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State( GLS_DEFAULT );
	// clear relevant buffers
	clearBits = GL_DEPTH_BUFFER_BIT;

	if ( r_measureOverdraw->integer || r_shadows->integer == 2 )
	{
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}
	if ( r_fastsky->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		clearBits |= GL_COLOR_BUFFER_BIT;	// FIXME: only if sky shaders have been used
#ifdef _DEBUG
		glClearColor( 0.8f, 0.7f, 0.4f, 1.0f );	// FIXME: get color of sky
#else
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// FIXME: get color of sky
#endif
	}
	glClear( clearBits );

	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;		// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if ( backEnd.viewParms.isPortal ) {
		float	plane[4];
		double	plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct (backEnd.viewParms.or.axis[0], plane);
		plane2[1] = DotProduct (backEnd.viewParms.or.axis[1], plane);
		plane2[2] = DotProduct (backEnd.viewParms.or.axis[2], plane);
		plane2[3] = DotProduct (plane, backEnd.viewParms.or.origin) - plane[3];

		glLoadMatrixf( s_flipMatrix );
		glClipPlane (GL_CLIP_PLANE0, plane2);
		glEnable (GL_CLIP_PLANE0);
	} else {
		glDisable (GL_CLIP_PLANE0);
	}
}


#define	MAC_EVENT_PUMP_MSEC		5

/*
==================
RB_RenderDrawSurfList
==================
*/
void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t		*shader, *oldShader;
	int				fogNum, oldFogNum;
	int				entityNum, oldEntityNum;
	int				dlighted, oldDlighted;
	qboolean		depthRange, oldDepthRange;
	int				i;
	drawSurf_t		*drawSurf;
	int				oldSort;
	float			originalTime;
#ifdef __MACOS__
	int				macEventTime;

	Sys_PumpEvents();		// crutch up the mac's limited buffer queue size

	// we don't want to pump the event loop too often and waste time, so
	// we are going to check every shader change
	macEventTime = ri.Milliseconds() + MAC_EVENT_PUMP_MSEC;
#endif

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView ();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldDlighted = qfalse;
	oldSort = -1;
	depthRange = qfalse;

	backEnd.pc.c_surfaces += numDrawSurfs;

	for (i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++) {
		if ( drawSurf->sort == oldSort ) {
			// fast path, same as previous sort
			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
			continue;
		}
		oldSort = drawSurf->sort;
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if (shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted 
			|| ( entityNum != oldEntityNum && !shader->entityMergable ) ) {
			if (oldShader != NULL) {
#ifdef __MACOS__	// crutch up the mac's limited buffer queue size
				int		t;

				t = ri.Milliseconds();
				if ( t > macEventTime ) {
					macEventTime = t + MAC_EVENT_PUMP_MSEC;
					Sys_PumpEvents();
				}
#endif
				RB_EndSurface();
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;
		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = qfalse;

			if ( entityNum != ENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

				// set up the transformation matrix
				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.or );

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {
					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
				}

				if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK ) {
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
			}

			glLoadMatrixf( backEnd.or.modelMatrix );

			//
			// change depthrange if needed
			//
			if ( oldDepthRange != depthRange ) {
				if ( depthRange ) {
					glDepthRange (0, 0.3);
				} else {
					glDepthRange (0, 1);
				}
				oldDepthRange = depthRange;
			}

			oldEntityNum = entityNum;
		}

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if (oldShader != NULL) {
		RB_EndSurface();
	}

	// go back to the world modelview matrix
	glLoadMatrixf( backEnd.viewParms.world.modelMatrix );
	if ( depthRange ) {
		glDepthRange (0, 1);
	}

// jmarshall
	if(backEnd.refdef.renderDebug) {
		RB_ShowDebugLines();
		RB_ShowDebugText();
	}
// jmarshall end
#if 0
	RB_DrawSun();
#endif
	// darken down any stencil shadows
	RB_ShadowFinish();		

	// add light flares on lights that aren't obscured
	RB_RenderFlares();

#ifdef __MACOS__
	Sys_PumpEvents();		// crutch up the mac's limited buffer queue size
#endif
}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void	RB_SetGL2D (void) {
	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	glViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	glScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();
	glOrtho (0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();

	GL_State( GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_CULL_FACE );
	glDisable( GL_CLIP_PLANE0 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw (int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {
	int			i, j;
	int			start, end;

	if ( !tr.registered ) {
		return;
	}
	R_SyncRenderThread();

	// we definately want to sync every frame for the cinematics
	glFinish();

	start = end = 0;
	if ( r_speeds->integer ) {
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
	}
	for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
	}
	if ( ( 1 << i ) != cols || ( 1 << j ) != rows) {
		ri.Error (ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
	}

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );	
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}

	if ( r_speeds->integer ) {
		end = ri.Milliseconds();
		ri.Printf( PRINT_ALL, "glTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
	}

	RB_SetGL2D();

	glColor3f( tr.identityLight, tr.identityLight, tr.identityLight );

	glBegin (GL_QUADS);
	glTexCoord2f ( 0.5f / cols,  0.5f / rows );
	glVertex2f (x, y);
	glTexCoord2f ( ( cols - 0.5f ) / cols ,  0.5f / rows );
	glVertex2f (x+w, y);
	glTexCoord2f ( ( cols - 0.5f ) / cols, ( rows - 0.5f ) / rows );
	glVertex2f (x+w, y+h);
	glTexCoord2f ( 0.5f / cols, ( rows - 0.5f ) / rows );
	glVertex2f (x, y+h);
	glEnd ();
}

void RE_UploadCinematic (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );	
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}
}


/*
=============
RB_SetColor

=============
*/
const void	*RB_SetColor( const void *data ) {
	const setColorCommand_t	*cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic ( const void *data ) {
	const stretchPicCommand_t	*cmd;
	shader_t *shader;
	int		numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ] =
		*(int *)tess.vertexColors[ numVerts + 1 ] =
		*(int *)tess.vertexColors[ numVerts + 2 ] =
		*(int *)tess.vertexColors[ numVerts + 3 ] = *(int *)backEnd.color2D;

	tess.xyz[ numVerts ][0] = cmd->x;
	tess.xyz[ numVerts ][1] = cmd->y;
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ][1] = cmd->y;
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = cmd->x;
	tess.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawSurfs

=============
*/
const void	*RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer

=============
*/
const void	*RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	cmd = (const drawBufferCommand_t *)data;

	glDrawBuffer( cmd->buffer );

	// clear screen for debugging
	if ( r_clear->integer ) {
		glClearColor( 1, 0, 0.5, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	int		i;
	image_t	*image;
	float	x, y, w, h;
	int		start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	glClear( GL_COLOR_BUFFER_BIT );

	glFinish();

	start = ri.Milliseconds();

	for ( i=0 ; i<tr.numImages ; i++ ) {
		image = tr.images[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GL_Bind( image );
		glBegin (GL_QUADS);
		glTexCoord2f( 0, 0 );
		glVertex2f( x, y );
		glTexCoord2f( 1, 0 );
		glVertex2f( x + w, y );
		glTexCoord2f( 1, 1 );
		glVertex2f( x + w, y + h );
		glTexCoord2f( 0, 1 );
		glVertex2f( x, y + h );
		glEnd();
	}

	glFinish();

	end = ri.Milliseconds();
	ri.Printf( PRINT_ALL, "%i msec to draw all images\n", end - start );

}

/*
================
RB_SimpleWorldSetup
================
*/
void RB_SimpleWorldSetup(void) {
	//backEnd.currentSpace = &backEnd.viewDef->worldSpace;
	SetViewportAndScissor();

	//glLoadMatrixf(backEnd.or.modelMatrix);

	//backEnd.currentScissor = backEnd.viewDef->scissor;
	//glScissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
	//	backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
	//	backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
	//	backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
}

// jmarshall

/*
================
RB_DrawTextLength

  returns the length of the given text
================
*/
float RB_DrawTextLength(const char* text, float scale, int len) {
	int i, num, index, charIndex;
	float spacing, textLen = 0.0f;

	if (text && *text) {
		if (!len) {
			len = strlen(text);
		}
		for (i = 0; i < len; i++) {
			charIndex = text[i] - 32;
			if (charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS) {
				continue;
			}
			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while (index - 2 < num) {
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				index += 2;
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
			}
			textLen += spacing * scale;
		}
	}
	return textLen;
}

/*
================
RB_DrawText

  oriented on the viewaxis
  align can be 0-left, 1-center (default), 2-right
================
*/
static void RB_DrawText(const char* text, const vec3_t origin, float scale, const vec4_t color, const int align) {
	int i, j, len, num, index, charIndex, line;
	float textLen, spacing;
	vec3_t org, p1, p2;

	if (text && *text) {
		glBegin(GL_LINES);
		glColor3fv(color);

		if (text[0] == '\n') {
			line = 1;
		}
		else {
			line = 0;
		}

		len = strlen(text);
		for (i = 0; i < len; i++) {

			if (i == 0 || text[i] == '\n') {
				org[0] = origin[0] - backEnd.or.axis[2][0] * (line * 36.0f * scale);
				org[0] = origin[1] - backEnd.or.axis[2][1] * (line * 36.0f * scale);
				org[0] = origin[2] - backEnd.or.axis[2][2] * (line * 36.0f * scale);
				if (align != 0) {
					for (j = 1; i + j <= len; j++) {
						if (i + j == len || text[i + j] == '\n') {
							textLen = RB_DrawTextLength(text + i, scale, j);
							break;
						}
					}
					if (align == 2) {
						// right
						org[0] += backEnd.or.axis[1][0] * textLen;
						org[1] += backEnd.or.axis[1][1] * textLen;
						org[2] += backEnd.or.axis[1][2] * textLen;
					}
					else {
						// center
						org[0] += backEnd.or.axis[1][0] * (textLen * 0.5f);
						org[1] += backEnd.or.axis[1][1] * (textLen * 0.5f);
						org[2] += backEnd.or.axis[1][2] * (textLen * 0.5f);
					}
				}
				line++;
			}

			charIndex = text[i] - 32;
			if (charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS) {
				continue;
			}
			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while (index - 2 < num) {
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				p1[0] = org[0] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][0] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][0];
				p1[1] = org[1] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][1] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][1];
				p1[2] = org[2] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][2] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][2];
				index += 2;
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				p2[0] = org[0] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][0] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][0];
				p2[1] = org[1] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][1] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][1];
				p2[2] = org[2] + scale * simplex[charIndex][index] * -backEnd.or.axis[1][2] + scale * simplex[charIndex][index + 1] * backEnd.or.axis[2][2];

				glVertex3fv(p1);
				glVertex3fv(p2);
			}
			org[0] -= backEnd.or.axis[1][0] * (spacing * scale);
			org[1] -= backEnd.or.axis[1][1] * (spacing * scale);
			org[2] -= backEnd.or.axis[1][2] * (spacing * scale);
		}

		glEnd();
	}
}

/*
================
RB_ShowDebugText
================
*/
void RB_ShowDebugText(void) {
	int			i;
	int			width;
	debugText_t* text;

	if (backEndData[0]->numDebugText <= 0) {
		return;
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	glDisable(GL_TEXTURE_2D);

	width = 4; // r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	}
	else if (width > 10) {
		width = 10;
	}

	// draw lines
	GL_State(GLS_POLYMODE_LINE);
	glLineWidth(width);

	//if (!r_debugLineDepthTest.GetBool()) {
		glDisable(GL_DEPTH_TEST);
	//}

	text = backEndData[0]->debugText;
	for (i = 0; i < backEndData[0]->numDebugText; i++, text++) {
		if (!text->depthTest) {
			RB_DrawText(text->text, text->origin, text->scale, text->color, text->align);
		}
	}

	//if (!r_debugLineDepthTest.GetBool()) {
		glEnable(GL_DEPTH_TEST);
	//}

	text = backEndData[0]->debugText;
	for (i = 0; i < backEndData[0]->numDebugText; i++, text++) {
		if (text->depthTest) {
			RB_DrawText(text->text, text->origin, text->scale, text->color, text->align);
		}
	}

	glLineWidth(1);
	GL_State(GLS_DEFAULT);
	glEnable(GL_TEXTURE_2D);
}
// jmarshall end

/*
================
RB_ShowDebugLines
================
*/
// jmarshall
void RB_ShowDebugLines(void) {
	int			i;
	int			width;
	debugLine_t* line;

	if (backEndData[0]->numDebugLines <= 0)
		return;

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();
	glDisable(GL_TEXTURE_2D);

	//globalImages->BindNull();

	width = 4; // r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	}
	else if (width > 10) {
		width = 10;
	}

	// draw lines
	GL_State(GLS_POLYMODE_LINE);//| GLS_DEPTHMASK ); //| GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	glLineWidth(width);

	//if (!r_debugLineDepthTest.GetBool()) {
		glDisable(GL_DEPTH_TEST);
	//}

	glBegin(GL_LINES);

	line = backEndData[0]->debugLines;
	for (i = 0; i < backEndData[0]->numDebugLines; i++, line++) {
		if (!line->depthTest) {
			glColor3fv(line->rgb);
			glVertex3fv(line->start);
			glVertex3fv(line->end);
		}
	}
	glEnd();

	//if (!r_debugLineDepthTest.GetBool()) {
		glEnable(GL_DEPTH_TEST);
	//}

	glBegin(GL_LINES);

	line = backEndData[0]->debugLines;
	for (i = 0; i < backEndData[0]->numDebugLines; i++, line++) {
		if (line->depthTest) {
			glColor4fv(line->rgb);
			glVertex3fv(line->start);
			glVertex3fv(line->end);
		}
	}

	glEnd();

	glLineWidth(1);
	GL_State(GLS_DEFAULT);
	glEnable(GL_TEXTURE_2D);
}
// jmarshall end

/*
=============
RB_SwapBuffers

=============
*/
const void	*RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}


	if ( !glState.finishCalled ) {
		glFinish();
	}

	GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	GLimp_EndFrame();

	backEnd.projection2D = qfalse;

	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data ) {
	int		t1, t2;

	t1 = ri.Milliseconds ();

	if ( !r_smp->integer || data == backEndData[0]->commands.cmds ) {
		backEnd.smpFrame = 0;
	} else {
		backEnd.smpFrame = 1;
	}

	while ( 1 ) {
		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;
		case RC_SCREENSHOT:
			data = RB_TakeScreenshotCmd( data );
			break;

		case RC_END_OF_LIST:
		default:
			// stop rendering on this thread
			t2 = ri.Milliseconds ();
			backEnd.pc.msec = t2 - t1;
			return;
		}
	}

}


/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void ) {
	const void	*data;

	// wait for either a rendering command or a quit command
	while ( 1 ) {
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if ( !data ) {
			return;	// all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands( data );

		renderThreadActive = qfalse;
	}
}

