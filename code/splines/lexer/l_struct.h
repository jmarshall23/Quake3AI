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

/*****************************************************************************
 * name:		l_struct.h
 *
 * desc:		structure reading/writing
 *
 * $Archive: /source/code/botlib/l_struct.h $
 *
 *****************************************************************************/

//read a structure from a script
int ReadStructure(source_t *source, structdef_t *def, char *structure);

//write a structure to a file
int WriteStructure(FILE *fp, structdef_t *def, char *structure);

//writes indents
int WriteIndent(FILE *fp, int indent);

//writes a float without traling zeros
int WriteFloat(FILE *fp, float value);


