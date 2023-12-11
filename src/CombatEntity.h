#ifndef COMBATENTITY_H__
#define COMBATENTITY_H__

struct DoomRPG_s;

#include "DoomRPG.h"
#include "Weapon.h"

#define COMBATENTITY_MISS	0
#define COMBATENTITY_HIT	1
#define COMBATENTITY_CRIT	2

extern byte monsterAttacks[];

typedef struct CombatEntity_s
{
	struct DoomRPG_s* doomRpg;
	int param1;
	int param2;
	int mType;
} CombatEntity_t;

void CombatEntity_initCombatEntity(CombatEntity_t* ce, int type, int health, int armor, int defense, int strength, int agility, int accuracy);

void CombatEntity_setAccuracy(CombatEntity_t* ce, int i);
void CombatEntity_setAgility(CombatEntity_t* ce, int i);
void CombatEntity_setArmor(CombatEntity_t* ce, int i);
void CombatEntity_setDefense(CombatEntity_t* ce, int i);
void CombatEntity_setHealth(CombatEntity_t* ce, int i);
void CombatEntity_setMaxArmor(CombatEntity_t* ce, int i);
void CombatEntity_setMaxHealth(CombatEntity_t* ce, int i);
void CombatEntity_setStrength(CombatEntity_t* ce, int i);

int CombatEntity_getAccuracy(CombatEntity_t* ce);
int CombatEntity_getAgility(CombatEntity_t* ce);
int CombatEntity_getArmor(CombatEntity_t* ce);
int CombatEntity_getDefense(CombatEntity_t* ce);
int CombatEntity_getHealth(CombatEntity_t* ce);
int CombatEntity_getMaxArmor(CombatEntity_t* ce);
int CombatEntity_getMaxHealth(CombatEntity_t* ce);
int CombatEntity_getStrength(CombatEntity_t* ce);

int CombatEntity_getEXP(CombatEntity_t* ce);
void CombatEntity_setupEnemy(CombatEntity_t* ce, int i);
int CombatEntity_calcWeaponDamage(DoomRPG_t* doomrpg, Weapon_t* wpn, CombatEntity_t* ce);
void CombatEntity_calcDamage(DoomRPG_t* doomrpg, CombatEntity_t* ce1, Weapon_t* wpn, CombatEntity_t* ce2, int i, int dist, int* getDamage, int* getDamageArmmor);
int CombatEntity_calcHit(DoomRPG_t* doomrpg, CombatEntity_t* ce1, Weapon_t* wnp, CombatEntity_t* ce2, int dist);

#endif
