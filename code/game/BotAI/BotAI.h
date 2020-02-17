// BotAI.h
//

#pragma once

#include "../bot_local.h"

//
// BotAI.c
//
void BotUpdateBattleInventory(bot_state_t* bs, int enemy);
float BotEntityVisible(int viewer, vec3_t eye, vec3_t viewangles, float fov, int ent);
int BotFindEnemy(bot_state_t* bs, int curenemy);
qboolean InFieldOfVision(vec3_t viewangles, float fov, vec3_t angles);
qboolean EntityIsDead(gentity_t* ent);
qboolean EntityIsInvisible(gentity_t* ent);
qboolean EntityIsShooting(gentity_t* ent);
void BotAimAtEnemy(bot_state_t* bs);
void BotCheckAttack(bot_state_t* bs);
int BotNearbyGoal(bot_state_t* bs, int tfl, bot_goal_t* ltg, float range);
void BotUpdateInventory(bot_state_t* bs);

//
// BotAI_deathmatch.c
//
void BotDeathmatchAI(bot_state_t* bs, float thinktime);
int BotWantsToChase(bot_state_t* bs);
void AIEnter_Seek_NBG(bot_state_t* bs, char* s);

//
// BotAI_ltg.c
//
void AIEnter_Seek_LTG(bot_state_t* bs, char* s);
int BotReachedGoal(bot_state_t* bs, bot_goal_t* goal);
int BotLongTermGoal(bot_state_t* bs, int tfl, int retreat, bot_goal_t* goal);


//
// BotAI_Battle.c
//
void AIEnter_Battle_Chase(bot_state_t* bs, char* s);
void AIEnter_Battle_Retreat(bot_state_t* bs, char* s);
void AIEnter_Battle_SuicidalFight(bot_state_t* bs, char* s);
void AIEnter_Battle_Fight(bot_state_t* bs, char* s);
void AIEnter_Battle_NBG(bot_state_t* bs, char* s);