#ifndef COMBAT_H__
#define COMBAT_H__


#define MONSTER_FIELD_ATTACK1	0
#define MONSTER_FIELD_ATTACK2	1

#define MONSTER_WP_FIELD_NUMSHOTS	0
#define MONSTER_WP_FIELD_SHOTHOLD	1

#define FLD_WP_NUMSHOTS	0
#define FLD_WP_SHOTHOLD	1
#define FLD_WP_IDLEX	2
#define FLD_WP_IDLEY	3
#define FLD_WP_ATKX		4
#define FLD_WP_ATKY		5

struct DoomRPG_s;
struct Entity_s;

#include "CombatEntity.h"
#include "Weapon.h"
#include "Game.h"

typedef struct Combat_s
{
	int memory;
	struct Entity_s* curAttacker;
	struct Entity_s* curTarget;
	int stage;
	int nextStageTime;
	int missileZHeight;
	struct GameSpriteMissile_s activeMissiles[3];
	int numActiveMissiles;
	boolean exploded;
	byte f342d;
	int damage;
	int totalDamage;
	int armorDamage;
	int totalArmorDamage;
	int hitType;
	int animEndTime;
	int attackerWeaponId;
	int attackFrame;
	int animLoopCount;
	boolean gotCrit;
	boolean f336b;
	boolean f340c;
	boolean kronosTeleporter;
	boolean kronosTeleporterDest;
	int f339c;
	int frameTime;
	int endFrameTime;
	int bloodColor;
	int missileAnim;
	struct CombatEntity_s bMobj;
	struct CombatEntity_s aMobj;
	struct Weapon_s weaponInfo[19];
	struct CombatEntity_s monsters[14];
	int worldDist;
	int tileDist;
	struct DoomRPG_s* doomRpg;
} Combat_t;

Combat_t* Combat_init(Combat_t* combat, DoomRPG_t* doomRpg);
void Combat_free(Combat_t* combat, boolean freePtr);
int Combat_calcParticleIntensity(Combat_t* combat, int i);
int Combat_calcHit(Combat_t* combat, Entity_t* entity, int i);
void Combat_worldDistToTileDist(Combat_t* combat);
void Combat_drawWeapon(Combat_t* combat, int x, int y);
void Combat_explodeOnMonster(Combat_t* combat);
void Combat_explodeOnPlayer(Combat_t* combat);
int Combat_getMonsterWeaponInfo(Combat_t* combat, int indx, int field);
void Combat_launchProjectile(Combat_t* combat);
int Combat_monsterSeq(Combat_t* combat);
void Combat_performAttack(Combat_t* combat, Entity_t* attacker, Entity_t* target);
boolean Combat_playerSeq(Combat_t* combat);
void Combat_spawnBloodParticles(Combat_t* combat, int i, int i2, int i3);
void Combat_spawnParticlesDestructibleObject(Combat_t* combat, int i);
void Combat_spawnDogBloodParticles(Combat_t* combat);
void Combat_spawnCaptureDogParticles(Combat_t* combat, int color);
void Combat_spawnParticlesFire(Combat_t* combat, int i, int color);
void Combat_spawnParticlesJammedDoor(Combat_t* combat, int color);
boolean Combat_runFrame(Combat_t* combat);
void Combat_updateProjectile(Combat_t* combat);

#endif
