#ifndef PLAYER_H__
#define PLAYER_H__

struct DoomRPG_s;
struct Entity_s;

#include "Entity.h"
#include "CombatEntity.h"

typedef struct Player_s
{
	int memory;
	struct DoomRPG_s* doomRpg;
	struct Entity_s* facingEntity;
	struct Entity_s* dogFamiliar;
	byte inventory[5];
	byte ammo[6];
	int weapons;
	int keys;
	int level;
	int currentXP;
	int nextLevelXP;
	int weapon;
	int credits;
	struct CombatEntity_s ce;
	boolean noclip;
	boolean god;
	int disabledWeapons;
	int berserkerTics;
	short prevCeilingColor;
	short prevFloorColor;
	int time;
	int totalTime;
	int moves;
	int totalMoves;
	int completedLevels;
	int killedMonstersLevels;
	int foundSecretsLevels;
	int xpGained;
	int totalDeaths;
	char NotebookString[512];
} Player_t;

Player_t* Player_init(Player_t* player, DoomRPG_t* doomRpg);
boolean Player_addAmmo(Player_t* player, int i, int i2);
void Player_addArmor(Player_t* player, int i);
boolean Player_addCredits(Player_t* player, int i);
void Player_addHealth(Player_t* player, int i);
boolean Player_addItem(Player_t* player, int i, int i2);
void Player_nextLevel(Player_t* player);
void Player_addXP(Player_t* player, int xp);
void Player_updateBerserkerTics(Player_t* player);
int Player_calcDamageDir(Player_t* player, int i, int i2, int i3, int i4, int i5);
int Player_calcLevelXP(Player_t* player, int i);
void Player_died(Player_t* player);
void Player_removeWeapons(Player_t* player);
void Player_restoreWeapons(Player_t* player);
boolean Player_fireWeapon(Player_t* player, Entity_t* entity);
void Player_formatTime(Player_t* player, char* text, int maxlen, int timeMs);
void Player_updateDamageFaceTime(Player_t* player);
void Player_setup(Player_t* player);
void Player_addStatusItem(Player_t* player, int i, int i2);
void Player_fillMonsterStats(Player_t* player, int* statA, int* statB);
void Player_selectNextWeapon(Player_t* player);
void Player_painEvent(Player_t* player, Entity_t* entity);
void Player_pain(Player_t* player, int i, int i2);
void Player_addLevelStats(Player_t* player, boolean z);
void Player_selectPrevWeapon(Player_t* player);
boolean Player_checkStatusItem(Player_t* player, int i, int i2);
void Player_reset(Player_t* player);
void Player_fillSecretStats(Player_t* player, int* statA, int* statB);
void Player_selectWeapon(Player_t* player, int i);
boolean Player_useCollarItem(Player_t* player);
void Player_useItem(Player_t* player, byte item);

#endif
