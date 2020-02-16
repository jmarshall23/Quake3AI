// bot_weights.c
//

#include "q_shared.h"
#include "g_local.h"

#include "bot_local.h"

#define MAX_INVENTORYVALUE			999999
#define EVALUATERECURSIVELY

#define MAX_WEIGHT_FILES			128
weightconfig_t* weightFileList[MAX_WEIGHT_FILES];

/*
========================
ReadValue
========================
*/
int ReadValue(int source, float* value)
{
	pc_token_t token;

	if (!trap_PC_ReadToken(source, &token)) 
		return qfalse;

	if (!strcmp(token.string, "-"))
	{
		G_Printf("negative value set to zero\n");
		if (!trap_PC_ExpectTokenType(source, TT_NUMBER, 0, &token)) 
			return qfalse;
	}

	if (token.type != TT_NUMBER)
	{
		G_Error("invalid return value %s\n", token.string);
		return qfalse;
	}
	
	*value = token.floatvalue;
	
	return qtrue;
} 

/*
===================
ReadFuzzyWeight
===================
*/
int ReadFuzzyWeight(int source, fuzzyseperator_t* fs)
{
	if (trap_PC_CheckTokenString(source, "balance"))
	{
		fs->type = WT_BALANCE;

		if (!trap_PC_ExpectTokenString(source, "(")) 
			return qfalse;
		
		if (!ReadValue(source, &fs->weight)) 
			return qfalse;
		
		if (!trap_PC_ExpectTokenString(source, ",")) 
			return qfalse;

		if (!ReadValue(source, &fs->minweight)) 
			return qfalse;

		if (!trap_PC_ExpectTokenString(source, ",")) 
			return qfalse;

		if (!ReadValue(source, &fs->maxweight)) 
			return qfalse;

		if (!trap_PC_ExpectTokenString(source, ")")) 
			return qfalse;
	} 
	else
	{
		fs->type = 0;

		if (!ReadValue(source, &fs->weight)) 
			return qfalse;
		fs->minweight = fs->weight;
		fs->maxweight = fs->weight;
	} 
	
	if (!trap_PC_ExpectTokenString(source, ";")) 
		return qfalse;

	return qtrue;
}

/*
===================
FreeFuzzySeperators_r
===================
*/
void FreeFuzzySeperators_r(fuzzyseperator_t* fs)
{
	if (!fs) 
		return;
	if (fs->child) 
		FreeFuzzySeperators_r(fs->child);
	if (fs->next) 
		FreeFuzzySeperators_r(fs->next);

	//FreeMemory(fs); // jmarshall: todo
}

/*
===================
FreeWeightConfig2
===================
*/
void FreeWeightConfig2(weightconfig_t* config)
{
	int i;

	for (i = 0; i < config->numweights; i++)
	{
		FreeFuzzySeperators_r(config->weights[i].firstseperator);
// jmarshall - todo
		//if (config->weights[i].name) 
		//	FreeMemory(config->weights[i].name);
// jmarshall end
	}
	
} 

/*
===================
FreeWeightConfig
===================
*/
void FreeWeightConfig(weightconfig_t* config)
{
	if (!LibVarGetValue("bot_reloadcharacters")) 
		return;

	FreeWeightConfig2(config);
} 

/*
===================
ReadFuzzySeperators_r
===================
*/
fuzzyseperator_t* ReadFuzzySeperators_r(int source)
{
	int newindent, index, def, founddefault;
	pc_token_t token;
	fuzzyseperator_t* fs, * lastfs, * firstfs;

	founddefault = qfalse;
	firstfs = NULL;
	lastfs = NULL;
	if (!trap_PC_ExpectTokenString(source, "(")) 
		return NULL;
	
	if (!trap_PC_ExpectTokenType(source, TT_NUMBER, TT_INTEGER, &token)) 
		return NULL;

	index = token.intvalue;
	
	if (!trap_PC_ExpectTokenString(source, ")")) 
		return NULL;

	if (!trap_PC_ExpectTokenString(source, "{")) 
		return NULL;

	if (!trap_PC_ReadToken(source, &token)) 
		return NULL;
	do
	{
		def = !strcmp(token.string, "default");
		if (def || !strcmp(token.string, "case"))
		{
			fs = (fuzzyseperator_t*)G_AllocClearedMemory(sizeof(fuzzyseperator_t));
			fs->index = index;
			if (lastfs) lastfs->next = fs;
			else firstfs = fs;
			lastfs = fs;
			if (def)
			{
				if (founddefault)
				{
					G_Error("switch already has a default\n");
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				}
				fs->value = MAX_INVENTORYVALUE;
				founddefault = qtrue;
			} 
			else
			{
				if (!trap_PC_ExpectTokenType(source, TT_NUMBER, TT_INTEGER, &token))
				{
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				}

				fs->value = token.intvalue;
			} 
			
			if (!trap_PC_ExpectTokenString(source, ":") || !trap_PC_ExpectAnyToken(source, &token))
			{
				FreeFuzzySeperators_r(firstfs);
				return NULL;
			} 
			
			newindent = qfalse;
			if (!strcmp(token.string, "{"))
			{
				newindent = qtrue;
				if (!trap_PC_ExpectAnyToken(source, &token))
				{
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				} 
			}

			if (!strcmp(token.string, "return"))
			{
				if (!ReadFuzzyWeight(source, fs))
				{
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				} 
			} 
			else if (!strcmp(token.string, "switch"))
			{
				fs->child = ReadFuzzySeperators_r(source);
				if (!fs->child)
				{
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				} 
			}
			else
			{
				G_Error("invalid name %s\n", token.string);
				return NULL;
			}

			if (newindent)
			{
				if (!trap_PC_ExpectTokenString(source, "}"))
				{
					FreeFuzzySeperators_r(firstfs);
					return NULL;
				} 
			} 
		} 
		else
		{
			FreeFuzzySeperators_r(firstfs);
			G_Error( "invalid name %s\n", token.string);
			return NULL;
		}

		if (!trap_PC_ExpectAnyToken(source, &token))
		{
			FreeFuzzySeperators_r(firstfs);
			return NULL;
		} 
	} while (strcmp(token.string, "}"));
	
	if (!founddefault)
	{
		G_Printf("switch without default\n");
		fs = (fuzzyseperator_t*)G_AllocClearedMemory(sizeof(fuzzyseperator_t));
		fs->index = index;
		fs->value = MAX_INVENTORYVALUE;
		fs->weight = 0;
		fs->next = NULL;
		fs->child = NULL;
		if (lastfs) lastfs->next = fs;
		else firstfs = fs;
		lastfs = fs;
	}
	
	return firstfs;
}

/*
=====================
ReadWeightConfig
=====================
*/
weightconfig_t* ReadWeightConfig(char* filename)
{
	int newindent, avail = 0, n;
	pc_token_t token;
	int source;
	fuzzyseperator_t* fs;
	weightconfig_t* config = NULL;

	if (!LibVarGetValue("bot_reloadcharacters"))
	{
		avail = -1;
		for (n = 0; n < MAX_WEIGHT_FILES; n++)
		{
			config = weightFileList[n];
			if (!config)
			{
				if (avail == -1)
				{
					avail = n;
				}
				continue;
			}
			if (strcmp(filename, config->filename) == 0)
			{
				return config;
			}
		}

		if (avail == -1)
		{
			G_Error("weightFileList was full trying to load %s\n", filename);
			return NULL;
		}
	}

	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	source = trap_PC_LoadSource(filename);
	if (!source)
	{
		G_Error("counldn't load %s\n", filename);
		return NULL;
	}

	config = (weightconfig_t*)G_AllocClearedMemory(sizeof(weightconfig_t));
	config->numweights = 0;

	Q_strncpyz(config->filename, filename, sizeof(config->filename));

	//parse the item config file
	while (trap_PC_ReadToken(source, &token))
	{
		if (!strcmp(token.string, "weight"))
		{
			if (config->numweights >= MAX_WEIGHTS)
			{
				G_Error("too many fuzzy weights\n");
				break;
			} 

			if (!trap_PC_ExpectTokenType(source, TT_STRING, 0, &token))
			{
				FreeWeightConfig(config);
				trap_PC_FreeSource(source);
				return NULL;
			} 

			StripDoubleQuotes(token.string);
			config->weights[config->numweights].name = (char*)G_AllocClearedMemory(strlen(token.string) + 1);
			strcpy(config->weights[config->numweights].name, token.string);
			
			if (!trap_PC_ExpectAnyToken(source, &token))
			{
				FreeWeightConfig(config);
				trap_PC_FreeSource(source);
				return NULL;
			}

			newindent = qfalse;
			if (!strcmp(token.string, "{"))
			{
				newindent = qtrue;
				if (!trap_PC_ExpectAnyToken(source, &token))
				{
					FreeWeightConfig(config);
					trap_PC_FreeSource(source);
					return NULL;
				} //end if
			} //end if
			if (!strcmp(token.string, "switch"))
			{
				fs = ReadFuzzySeperators_r(source);
				if (!fs)
				{
					FreeWeightConfig(config);
					trap_PC_FreeSource(source);
					return NULL;
				} //end if
				config->weights[config->numweights].firstseperator = fs;
			} //end if
			else if (!strcmp(token.string, "return"))
			{
				fs = (fuzzyseperator_t*)G_AllocClearedMemory(sizeof(fuzzyseperator_t));
				fs->index = 0;
				fs->value = MAX_INVENTORYVALUE;
				fs->next = NULL;
				fs->child = NULL;
				if (!ReadFuzzyWeight(source, fs))
				{
					//FreeMemory(fs);
					FreeWeightConfig(config);
					trap_PC_FreeSource(source);
					return NULL;
				} //end if
				config->weights[config->numweights].firstseperator = fs;
			} //end else if
			else
			{
				G_Error("invalid name %s\n", token.string);
				FreeWeightConfig(config);
				trap_PC_FreeSource(source);
				return NULL;
			} //end else
			if (newindent)
			{
				if (!trap_PC_ExpectTokenString(source, "}"))
				{
					FreeWeightConfig(config);
					trap_PC_FreeSource(source);
					return NULL;
				} //end if
			} //end if
			config->numweights++;
		} //end if
		else
		{
			G_Error("invalid name %s\n", token.string);
			FreeWeightConfig(config);
			trap_PC_FreeSource(source);
			return NULL;
		} //end else
	} //end while
	
	 //free the source at the end of a pass
	trap_PC_FreeSource(source);
	
	//if the file was located in a pak file
	G_Printf("loaded %s\n", filename);

	if (!LibVarGetValue("bot_reloadcharacters"))
	{
		weightFileList[avail] = config;
	}
	
	return config;
}

/*
==================
FindFuzzyWeight
==================
*/
int FindFuzzyWeight(weightconfig_t* wc, char* name)
{
	int i;

	for (i = 0; i < wc->numweights; i++)
	{
		if (!strcmp(wc->weights[i].name, name))
		{
			return i;
		} 
	} 
	return -1;
} 

/*
==================
FuzzyWeight_r
==================
*/
float FuzzyWeight_r(int* inventory, fuzzyseperator_t* fs)
{
	float scale, w1, w2;

	if (inventory[fs->index] < fs->value)
	{
		if (fs->child) 
			return FuzzyWeight_r(inventory, fs->child);
		else 
			return fs->weight;
	} 
	else if (fs->next)
	{
		if (inventory[fs->index] < fs->next->value)
		{
			//first weight
			if (fs->child) 
				w1 = FuzzyWeight_r(inventory, fs->child);
			else 
				w1 = fs->weight;
			
			//second weight
			if (fs->next->child) 
				w2 = FuzzyWeight_r(inventory, fs->next->child);
			else 
				w2 = fs->next->weight;
			
			//the scale factor
			scale = (inventory[fs->index] - fs->value) / (fs->next->value - fs->value);
			
			//scale between the two weights
			return scale * w1 + (1 - scale) * w2;
		} 
		return FuzzyWeight_r(inventory, fs->next);
	} 
	return fs->weight;
} 

/*
============================
FuzzyWeightUndecided_r
============================
*/
float FuzzyWeightUndecided_r(int* inventory, fuzzyseperator_t* fs)
{
	float scale, w1, w2;

	if (inventory[fs->index] < fs->value)
	{
		if (fs->child) 
			return FuzzyWeightUndecided_r(inventory, fs->child);
		else 
			return fs->minweight + random() * (fs->maxweight - fs->minweight);
	} 
	else if (fs->next)
	{
		if (inventory[fs->index] < fs->next->value)
		{
			//first weight
			if (fs->child) 
				w1 = FuzzyWeightUndecided_r(inventory, fs->child);
			else 
				w1 = fs->minweight + random() * (fs->maxweight - fs->minweight);
			
			//second weight
			if (fs->next->child) 
				w2 = FuzzyWeight_r(inventory, fs->next->child);
			else 
				w2 = fs->next->minweight + random() * (fs->next->maxweight - fs->next->minweight);
			
			//the scale factor
			scale = (inventory[fs->index] - fs->value) / (fs->next->value - fs->value);
			
			//scale between the two weights
			return scale * w1 + (1 - scale) * w2;
		} 
		return FuzzyWeightUndecided_r(inventory, fs->next);
	} 
	return fs->weight;
}

/*
=================
FuzzyWeight
=================
*/
float FuzzyWeight(int* inventory, weightconfig_t* wc, int weightnum)
{
	return FuzzyWeight_r(inventory, wc->weights[weightnum].firstseperator);
}

/*
=================
FuzzyWeightUndecided
=================
*/
float FuzzyWeightUndecided(int* inventory, weightconfig_t* wc, int weightnum)
{
	return FuzzyWeightUndecided_r(inventory, wc->weights[weightnum].firstseperator);
} 

/*
====================
EvolveFuzzySeperator_r
====================
*/
void EvolveFuzzySeperator_r(fuzzyseperator_t* fs)
{
	if (fs->child)
	{
		EvolveFuzzySeperator_r(fs->child);
	}
	else if (fs->type == WT_BALANCE)
	{
		//every once in a while an evolution leap occurs, mutation
		if (random() < 0.01) 
			fs->weight += crandom() * (fs->maxweight - fs->minweight);
		else 
			fs->weight += crandom() * (fs->maxweight - fs->minweight) * 0.5;

		//modify bounds if necesary because of mutation
		if (fs->weight < fs->minweight) 
			fs->minweight = fs->weight;
		else if (fs->weight > fs->maxweight) 
			fs->maxweight = fs->weight;
	} 
	if (fs->next) EvolveFuzzySeperator_r(fs->next);
}

/*
====================
EvolveWeightConfig
====================
*/
void EvolveWeightConfig(weightconfig_t* config)
{
	int i;

	for (i = 0; i < config->numweights; i++)
	{
		EvolveFuzzySeperator_r(config->weights[i].firstseperator);
	}
}

/*
====================
ScaleWeight
====================
*/
void ScaleFuzzySeperator_r(fuzzyseperator_t* fs, float scale)
{
	if (fs->child)
	{
		ScaleFuzzySeperator_r(fs->child, scale);
	}
	else if (fs->type == WT_BALANCE)
	{
		fs->weight = (fs->maxweight + fs->minweight) * scale;
		
		//get the weight between bounds
		if (fs->weight < fs->minweight) 
			fs->weight = fs->minweight;
		else if (fs->weight > fs->maxweight) 
			fs->weight = fs->maxweight;
	}
	if (fs->next) 
		ScaleFuzzySeperator_r(fs->next, scale);
}

/*
====================
ScaleWeight
====================
*/
void ScaleWeight(weightconfig_t* config, char* name, float scale)
{
	int i;

	if (scale < 0) scale = 0;
	else if (scale > 1) scale = 1;
	for (i = 0; i < config->numweights; i++)
	{
		if (!strcmp(name, config->weights[i].name))
		{
			ScaleFuzzySeperator_r(config->weights[i].firstseperator, scale);
			break;
		}
	}
}

/*
====================
ScaleFuzzySeperatorBalanceRange_r
====================
*/
void ScaleFuzzySeperatorBalanceRange_r(fuzzyseperator_t* fs, float scale)
{
	if (fs->child)
	{
		ScaleFuzzySeperatorBalanceRange_r(fs->child, scale);
	}
	else if (fs->type == WT_BALANCE)
	{
		float mid = (fs->minweight + fs->maxweight) * 0.5;
		//get the weight between bounds
		fs->maxweight = mid + (fs->maxweight - mid) * scale;
		fs->minweight = mid + (fs->minweight - mid) * scale;
		if (fs->maxweight < fs->minweight)
		{
			fs->maxweight = fs->minweight;
		} 
	}
	if (fs->next) 
		ScaleFuzzySeperatorBalanceRange_r(fs->next, scale);
} 

/*
====================
ScaleFuzzyBalanceRange
====================
*/
void ScaleFuzzyBalanceRange(weightconfig_t* config, float scale)
{
	int i;

	if (scale < 0) 
		scale = 0;
	else if (scale > 100) 
		scale = 100;
	for (i = 0; i < config->numweights; i++)
	{
		ScaleFuzzySeperatorBalanceRange_r(config->weights[i].firstseperator, scale);
	}
}

/*
====================
InterbreedFuzzySeperator_r
====================
*/
int InterbreedFuzzySeperator_r(fuzzyseperator_t* fs1, fuzzyseperator_t* fs2, fuzzyseperator_t* fsout)
{
	if (fs1->child)
	{
		if (!fs2->child || !fsout->child)
		{
			G_Error("cannot interbreed weight configs, unequal child\n");
			return qfalse;
		} 
		if (!InterbreedFuzzySeperator_r(fs2->child, fs2->child, fsout->child))
		{
			return qfalse;
		} 
	}
	else if (fs1->type == WT_BALANCE)
	{
		if (fs2->type != WT_BALANCE || fsout->type != WT_BALANCE)
		{
			G_Error("cannot interbreed weight configs, unequal balance\n");
			return qfalse;
		}
		fsout->weight = (fs1->weight + fs2->weight) / 2;
		if (fsout->weight > fsout->maxweight) fsout->maxweight = fsout->weight;
		if (fsout->weight > fsout->minweight) fsout->minweight = fsout->weight;
	}
	if (fs1->next)
	{
		if (!fs2->next || !fsout->next)
		{
			G_Error("cannot interbreed weight configs, unequal next\n");
			return qfalse;
		}
		if (!InterbreedFuzzySeperator_r(fs1->next, fs2->next, fsout->next))
		{
			return qfalse;
		}
	}
	return qtrue;
}

/*
====================
InterbreedWeightConfigs
====================
*/
void InterbreedWeightConfigs(weightconfig_t* config1, weightconfig_t* config2, weightconfig_t* configout)
{
	int i;

	if (config1->numweights != config2->numweights ||
		config1->numweights != configout->numweights)
	{
		G_Error("cannot interbreed weight configs, unequal numweights\n");
		return;
	} 

	for (i = 0; i < config1->numweights; i++)
	{
		InterbreedFuzzySeperator_r(config1->weights[i].firstseperator,
			config2->weights[i].firstseperator,
			configout->weights[i].firstseperator);
	} 
} 


/*
=======================
BotShutdownWeights
=======================
*/
void BotShutdownWeights(void)
{
	int i;

	for (i = 0; i < MAX_WEIGHT_FILES; i++)
	{
		if (weightFileList[i])
		{
			FreeWeightConfig2(weightFileList[i]);
			weightFileList[i] = NULL;
		} 
	} 
} 
