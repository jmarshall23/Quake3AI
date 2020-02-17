// bot_char.c
//

#include "g_local.h"
#include "bot_local.h"

// jmarshall
bot_character_t* default_char_profile = NULL;

/*
============================
InitCharacteristicSystem
============================
*/
void InitCharacteristicSystem(void) {
	default_char_profile = BotLoadCharacterFromFile("bots/default_c.c", 1);
	if(default_char_profile == NULL) {
		G_Error("Failed to load default characteristic def file\n");
	}
}
// jmarshall end

/*
============================
CheckCharacteristicIndex
============================
*/
int CheckCharacteristicIndex(bot_character_t* ch, int index)
{
	if (index < 0 || index >= MAX_CHARACTERISTICS)
	{
		G_Error("characteristic %d does not exist\n", index);
		return qfalse;
	}
	if (!ch->c[index].type)
	{
		G_Error("characteristic %d is not initialized\n", index);
		return qfalse;
	}
	return qtrue;
}

/*
============================
Characteristic_Float
============================
*/
float Characteristic_Float(bot_character_t* ch, int index)
{
	if (!ch) 
		return 0;
	//check if the index is in range
	if (!CheckCharacteristicIndex(ch, index)) 
		return 0;

	if (ch->c[index].type == CT_INTEGER)
	{
		//an integer will be converted to a float
		return (float)ch->c[index].value.integer;
	}	
	else if (ch->c[index].type == CT_FLOAT)
	{
		//floats are just returned
		return ch->c[index].value._float;
	} 

	//cannot convert a string pointer to a float
	G_Error("characteristic %d is not a float\n", index);
	return 0;
}

/*
============================
Characteristic_String
============================
*/
void Characteristic_String(bot_character_t* ch, int index, char* buf, int size)
{
	//check if the index is in range
	if (!CheckCharacteristicIndex(ch, index)) 
		return;

	//an integer will be converted to a float
	if (ch->c[index].type == CT_STRING)
	{
		strncpy(buf, ch->c[index].value.string, size - 1);
		buf[size - 1] = '\0';
		return;
	}
	G_Error("characteristic %d is not a string\n", index);
}

/*
============================
Characteristic_BFloat
============================
*/
float Characteristic_BFloat(bot_character_t* ch, int index, float min, float max)
{
	float value;

	if (min > max)
	{
		G_Error("cannot bound characteristic %d between %f and %f\n", index, min, max);
		return 0;
	}
	
	value = Characteristic_Float(ch, index);
	if (value < min) return min;
	if (value > max) return max;
	return value;
} 

/*
============================
BotFreeCharacterStrings
============================
*/
void BotFreeCharacterStrings(bot_character_t* ch)
{
	int i;

	for (i = 0; i < MAX_CHARACTERISTICS; i++)
	{
		if (ch->c[i].type == CT_STRING)
		{
			//FreeMemory(ch->c[i].value.string); // jmarshall fix me
		}
	} 
} 

/*
============================
BotLoadCharacterFromFile
============================
*/
bot_character_t* BotLoadCharacterFromFile(char* charfile, int skill)
{
	int indent, index, foundcharacter;
	bot_character_t* ch;
	int source;
	pc_token_t token;

	foundcharacter = qfalse;
	
	//a bot character is parsed in two phases
	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	source = trap_PC_LoadSource(charfile);
	if (!source)
	{
		G_Printf("counldn't load %s\n", charfile);
		return NULL;
	} 

	ch = (bot_character_t*)G_Alloc(sizeof(bot_character_t) + MAX_CHARACTERISTICS * sizeof(bot_characteristic_t));
// jmarshall -- apply the defaults
	if (default_char_profile == NULL)
	{
		memset(ch, 0, sizeof(bot_character_t) + MAX_CHARACTERISTICS * sizeof(bot_characteristic_t));
	}
	else
	{
		memcpy(ch, default_char_profile, sizeof(bot_character_t) + MAX_CHARACTERISTICS * sizeof(bot_characteristic_t));
	}	
// jmarshall end
	strcpy(ch->filename, charfile);
	while (trap_PC_ReadToken(source, &token))
	{
		if (!strcmp(token.string, "skill"))
		{
			if (!trap_PC_ExpectTokenType(source, TT_NUMBER, 0, &token))
			{
				trap_PC_FreeSource(source);
				BotFreeCharacterStrings(ch);
				//FreeMemory(ch);
				return NULL;
			}

			if (!trap_PC_ExpectTokenString(source, "{"))
			{
				trap_PC_FreeSource(source);
				BotFreeCharacterStrings(ch);
				//FreeMemory(ch);
				return NULL;
			}

			//if it's the correct skill
			if (skill < 0 || token.intvalue == skill)
			{
				foundcharacter = qtrue;
				ch->skill = token.intvalue;
				while (trap_PC_ReadToken(source, &token))
				{
					if (!strcmp(token.string, "}")) 
						break;
					
					if (token.type != TT_NUMBER || !(token.subtype & TT_INTEGER))
					{
						G_Error("expected integer index, found %s\n", token.string);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
					//	FreeMemory(ch);
						return NULL;
					}

					index = token.intvalue;
					if (index < 0 || index > MAX_CHARACTERISTICS)
					{
						G_Error("characteristic index out of range [0, %d]\n", MAX_CHARACTERISTICS);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					}
// jmarshall - not sure what we loose by removing this check, basically duplicate definition check?
					//if (ch->c[index].type)
					//{
					//	G_Error("characteristic %d already initialized\n", index);
					//	trap_PC_FreeSource(source);
					//	BotFreeCharacterStrings(ch);
					//	//FreeMemory(ch);
					//	return NULL;
					//}
// jmarshall end

					if (!trap_PC_ReadToken(source, &token))
					{
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
					//	FreeMemory(ch);
						return NULL;
					}

					if (token.type == TT_NUMBER)
					{
						if (token.subtype & TT_FLOAT)
						{
							ch->c[index].value._float = token.floatvalue;
							ch->c[index].type = CT_FLOAT;
						} 
						else
						{
							ch->c[index].value.integer = token.intvalue;
							ch->c[index].type = CT_INTEGER;
						} 
					}
					else if (token.type == TT_STRING)
					{
						StripDoubleQuotes(token.string);
						ch->c[index].value.string = G_Alloc(strlen(token.string) + 1);
						strcpy(ch->c[index].value.string, token.string);
						ch->c[index].type = CT_STRING;
					}
					else
					{
						G_Error("expected integer, float or string, found %s\n", token.string);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					}
				} 
				break;
			} 
			else
			{
				indent = 1;
				while (indent)
				{
					if (!trap_PC_ReadToken(source, &token))
					{
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					} 
					if (!strcmp(token.string, "{")) 
						indent++;
					else if (!strcmp(token.string, "}")) 
						indent--;
				} 
			} 
		} 
		else
		{
			G_Error("unknown definition %s\n", token.string);
			trap_PC_FreeSource(source);
			BotFreeCharacterStrings(ch);
			//FreeMemory(ch);
			return NULL;
		} 
	} 
	trap_PC_FreeSource(source);
	
	if (!foundcharacter)
	{
		BotFreeCharacterStrings(ch);
		//FreeMemory(ch);
		return NULL;
	}
	return ch;
} 