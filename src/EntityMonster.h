#ifndef ENTITYMONSTER_H__
#define ENTITYMONSTER_H__

struct DoomRPG_s;
struct Entity_s;

#include "CombatEntity.h"

typedef struct EntityMonster_s
{
	struct CombatEntity_s ce;
	struct Entity_s* nextOnList;
	struct Entity_s* prevOnList;
	struct Entity_s* nextAttacker;
	struct Entity_s* target;
	int x;
	int y;
	int frameTime;
	int animFrameTime;
	int damageBlendTime;
	struct DoomRPG_s* doomRpg;
} EntityMonster_t;

EntityMonster_t* EntityMonster_startup(EntityMonster_t* entityMonster);
int EntityMonster_getSoundRnd(EntityMonster_t* entityMonster, int field);
int EntityMonster_getSoundID(EntityMonster_t* entityMonster, int field);
void EntityMonster_reset(EntityMonster_t* entityMonster);
#endif
