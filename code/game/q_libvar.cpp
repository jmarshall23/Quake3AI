// q_libvar.cpp
//

#include "q_shared.hpp"

struct libvar_t
{
	libvar_t()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(libvar_t));
	}

	char name[512];
	char string[512];
	int		flags;
	qboolean	modified;	// set each time the cvar is changed
	float		value;
	qboolean	isUsed;
};

float GetValueFromLibVar(libvar_t *libVar) {
	return libVar->value;
}

//list with library variables
#define MAX_LIBVARIABLES		128
libvar_t libvarlist[MAX_LIBVARIABLES];

/*
=======================
LibVarStringValue
=======================
*/
float LibVarStringValue(char* string)
{
	int dotfound = 0;
	float value = 0;

	while (*string)
	{
		if (*string < '0' || *string > '9')
		{
			if (dotfound || *string != '.')
			{
				return 0;
			}
			else
			{
				dotfound = 10;
				string++;
			}
		}
		if (dotfound)
		{
			value = value + (float)(*string - '0') / (float)dotfound;
			dotfound *= 10;
		}
		else
		{
			value = value * 10.0 + (float)(*string - '0');
		}
		string++;
	}
	return value;
}

/*
=======================
LibVarAlloc
=======================
*/

libvar_t* LibVarAlloc(char* var_name)
{
	libvar_t* v = NULL;

	for(int i = 0; i < MAX_LIBVARIABLES; i++)
	{
		if(libvarlist[i].isUsed == qfalse)
		{
			libvarlist[i].isUsed = qtrue;
			v = &libvarlist[i];
			break;
		}
	}

	strcpy(v->name, var_name);
	return v;
}

/*
==================
LibVarDeAlloc
==================
*/
void LibVarDeAlloc(libvar_t* v)
{
	v->Reset();
} 

/*
==================
LibVarDeAlloc
==================
*/
void LibVarDeAllocAll(void)
{
	for(int i = 0; i < MAX_LIBVARIABLES; i++)
	{
		libvarlist[i].Reset();
	}
} 

/*
==================
LibVarDeAlloc
==================
*/
libvar_t* LibVarGet(char* var_name)
{	
	for (int i = 0; i < MAX_LIBVARIABLES; i++)
	{
		libvar_t* v = &libvarlist[i];

		if (!v->isUsed)
			continue;

		if (!Q_stricmp(v->name, var_name))
		{
			return v;
		}
	} 
	return NULL;
} 

/*
==================
LibVarGetString
==================
*/
char* LibVarGetString(char* var_name)
{
	libvar_t* v;

	v = LibVarGet(var_name);
	if (v)
	{
		return v->string;
	} 
	else
	{
		return "";
	} 
} 

/*
==================
LibVarGetValue
==================
*/
float LibVarGetValue(char* var_name)
{
	libvar_t* v;

	v = LibVarGet(var_name);
	if (v)
	{
		return v->value;
	} 
	else
	{
		return 0;
	} 
} 

/*
==================
LibVar
==================
*/
libvar_t* LibVar(char* var_name, char* value)
{
	libvar_t* v;
	v = LibVarGet(var_name);
	if (v) 
		return v;

	//create new variable
	v = LibVarAlloc(var_name);

	//variable string
	strcpy(v->string, value);
	
	//the value
	v->value = LibVarStringValue(v->string);
	
	//variable is modified
	v->modified = qtrue;
	
	return v;
} //end of the function LibVar

/*
==================
LibVarString
==================
*/
char* LibVarString(char* var_name, char* value)
{
	libvar_t* v;

	v = LibVar(var_name, value);
	return v->string;
} 

/*
==================
LibVarValue
==================
*/
float LibVarValue(char* var_name, char* value)
{
	libvar_t* v;

	v = LibVar(var_name, value);
	return v->value;
}

/*
==================
LibVarSet
==================
*/
void LibVarSet(char* var_name, char* value)
{
	libvar_t* v;

	v = LibVarGet(var_name);

	strcpy(v->string, value);
	
	//the value
	v->value = LibVarStringValue(v->string);
	
	//variable is modified
	v->modified = qtrue;
} 

/*
==================
LibVarSet
==================
*/
qboolean LibVarChanged(char* var_name)
{
	libvar_t* v;

	v = LibVarGet(var_name);
	if (v)
	{
		return v->modified;
	} 

	return qfalse;
} 

/*
==================
LibVarSetNotModified
==================
*/
void LibVarSetNotModified(char* var_name)
{
	libvar_t* v;

	v = LibVarGet(var_name);
	if (v)
	{
		v->modified = qfalse;
	} 
} 
