
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Game.h"
#include "Player.h"
#include "Hud.h"
#include "Entity.h"
#include "EntityDef.h"
#include "CombatEntity.h"
#include "Combat.h"
#include "Weapon.h"
#include "MenuSystem.h"
#include "Sound.h"
#include "SDL_Video.h"

Player_t* Player_init(Player_t* player, DoomRPG_t* doomRpg)
{

	printf("Player_init\n");

	if (player == NULL)
	{
		player = SDL_malloc(sizeof(Player_t));
		if (player == NULL) {
			return NULL;
		}
	}
	SDL_memset(player, 0, sizeof(Player_t));

	player->doomRpg = doomRpg;
	SDL_memset(&player->ce, 0, sizeof(CombatEntity_t));
	player->noclip = false;
	player->god = false;
	player->ce.doomRpg = doomRpg;
	player->weapon = 0;
	Player_reset(player);
	player->totalDeaths = 0;

	return player;
}

boolean Player_addAmmo(Player_t* player, int i, int i2)
{
	byte ammo = player->ammo[i];

	if (ammo == 99) {
		return false;
	}

	ammo += i2;
	if (ammo > 99) {
		ammo = 99;
	}

	player->ammo[i] = ammo;
	return true;
}

void Player_addArmor(Player_t* player, int i)
{
	CombatEntity_t* ce;
	int armmor, maxArmmor;
	char msg[64];

	if (i > 0) {
		Player_updateDamageFaceTime(player);
	}

	ce = &player->ce;

	armmor = CombatEntity_getArmor(ce);
	maxArmmor = CombatEntity_getMaxArmor(ce);

	if (armmor + i > maxArmmor) {
		CombatEntity_setArmor(ce, maxArmmor);
	}
	else {
		CombatEntity_setArmor(ce, armmor + i);
	}
	if (CombatEntity_getArmor(ce) > armmor) {
		SDL_snprintf(msg, sizeof(msg), "Gained %d armor", (CombatEntity_getArmor(ce) - armmor));
		Hud_addMessage(player->doomRpg->hud, msg);
	}
}

boolean Player_addCredits(Player_t* player, int i)
{
	player->credits += i;
	return true;
}

void Player_addHealth(Player_t* player, int i)
{
	CombatEntity_t* ce;
	int health, maxHealth;
	char msg[64];

	if (i > 0) {
		Player_updateDamageFaceTime(player);
	}

	ce = &player->ce;

	health = CombatEntity_getHealth(ce);
	maxHealth = CombatEntity_getMaxHealth(ce);

	if (health + i > maxHealth) {
		CombatEntity_setHealth(ce, maxHealth);
	}
	else {
		CombatEntity_setHealth(ce, health + i);
	}
	if (CombatEntity_getHealth(ce) > health) {
		SDL_snprintf(msg, sizeof(msg), "Gained %d health", (CombatEntity_getHealth(ce) - health));
		Hud_addMessage(player->doomRpg->hud, msg);
	}
}

boolean Player_addItem(Player_t* player, int i, int i2)
{
	int invNum = i - 25;
	byte inventory = player->inventory[invNum];

	if (inventory == 99) {
		return false;
	}

	inventory += i2;
	if (inventory > 99) {
		inventory = 99;
	}

	player->inventory[invNum] = (byte)inventory;
	return true;
}

void Player_nextLevel(Player_t* player)
{
	CombatEntity_t* ce;
	char msg[64], text[256];
	player->level++;
	player->nextLevelXP = Player_calcLevelXP(player, player->level);
	ce = &player->ce;

	//if (!Mix_PlayingMusic()) // New Line Code
	{ 
		Sound_playSound(player->doomRpg->sound, 5043, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 6);
	}

	strncpy(text, MenuSystem_buildDivider(player->doomRpg->menuSystem, "Level up!"), sizeof(text));
	strncat(text, "|", sizeof(text));
	SDL_snprintf(msg, sizeof(msg), "Level: %d|", player->level);
	strncat(text, msg, sizeof(text));

	int nextInt = 3 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 3);
	int b = CombatEntity_getMaxHealth(ce);
	if (b + nextInt > 99) {
		nextInt = 99 - b;
	}
	if (nextInt != 0) {
		CombatEntity_setMaxHealth(ce, b + nextInt);
		SDL_snprintf(msg, sizeof(msg), "Max Health: +%d|", nextInt);
		strncat(text, msg, sizeof(text));
	}
	CombatEntity_setHealth(ce, b + nextInt);

	int nextInt2 = 3 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 3);
	int d = CombatEntity_getMaxArmor(ce);
	if (d + nextInt2 > 99) {
		nextInt2 = 99 - d;
	}
	if (nextInt2 != 0) {
		CombatEntity_setMaxArmor(ce, d + nextInt2);
		SDL_snprintf(msg, sizeof(msg), "Max Armor: +%d|", nextInt2);
		strncat(text, msg, sizeof(text));
	}

	int nextInt3 = 1 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 2);
	if (CombatEntity_getDefense(ce) + nextInt3 > 99) {
		nextInt3 = 99 - CombatEntity_getDefense(ce);
	}
	if (nextInt3 != 0) {
		CombatEntity_setDefense(ce, CombatEntity_getDefense(ce) + nextInt3);
		SDL_snprintf(msg, sizeof(msg), "Defense: +%d|", nextInt3);
		strncat(text, msg, sizeof(text));
	}

	int nextInt4 = 1 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 2);
	if (CombatEntity_getStrength(ce) + nextInt4 > 99) {
		nextInt4 = 99 - CombatEntity_getStrength(ce);
	}
	if (nextInt4 != 0) {
		CombatEntity_setStrength(ce, CombatEntity_getStrength(ce) + nextInt4);
		SDL_snprintf(msg, sizeof(msg), "Strength: +%d|", nextInt4);
		strncat(text, msg, sizeof(text));
	}

	int nextInt5 = 1 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 2);
	if (CombatEntity_getAgility(ce) + nextInt5 > 99) {
		nextInt5 = 99 - CombatEntity_getAgility(ce);
	}
	if (nextInt5 != 0) {
		CombatEntity_setAgility(ce, CombatEntity_getAgility(ce) + nextInt5);
		SDL_snprintf(msg, sizeof(msg), "Agility: +%d|", nextInt5);
		strncat(text, msg, sizeof(text));
	}

	int nextInt6 = 1 + ((DoomRPG_randNextInt(&player->doomRpg->random) & 255) % 2);
	if (CombatEntity_getAccuracy(ce) + nextInt6 > 99) {
		nextInt6 = 99 - CombatEntity_getAccuracy(ce);
	}
	if (nextInt6 != 0) {
		CombatEntity_setAccuracy(ce, CombatEntity_getAccuracy(ce) + nextInt6);
		SDL_snprintf(msg, sizeof(msg), "Accuracy: +%d|", nextInt5);
		strncat(text, msg, sizeof(text));
	}

	strncat(text, "|Health restored.", sizeof(text));
	if (player->doomRpg->doomCanvas->state != ST_MENU) {
		player->doomRpg->game->tileEvent = 0;
		DoomCanvas_startDialog(player->doomRpg->doomCanvas, text, false);
	}
}

void Player_addXP(Player_t* player, int xp)
{
	char text[64];

	SDL_snprintf(text, sizeof(text), "Gained %d XP!", xp);
	Hud_addMessage(player->doomRpg->hud, text);
	player->currentXP += xp;
	player->xpGained += xp;
	while (player->currentXP >= player->nextLevelXP) {
		player->currentXP -= player->nextLevelXP;
		Player_nextLevel(player);
	}
}

void Player_updateBerserkerTics(Player_t* player)
{
	int i;
	player->moves++;
	if (player->berserkerTics) {
		player->berserkerTics--;
		if (player->berserkerTics == 0) {
			Hud_addMessageForce(player->doomRpg->hud, "Berserker expired!", true);
			for (i = 0; i < player->doomRpg->render->screenWidth; i++) {
				player->doomRpg->render->floorColor[i] = player->prevFloorColor;
				player->doomRpg->render->ceilingColor[i] = player->prevCeilingColor;
			}
			DoomCanvas_updateViewTrue(player->doomRpg->doomCanvas);
		}
	}
}

int Player_calcDamageDir(Player_t* player, int i, int i2, int i3, int i4, int i5)
{
	switch (i3 & 255) {
	case 0:
		if (i5 > i2) {
			return 1;
		}
		if (i5 < i2) {
			return 3;
		}
		if (i4 < i) {
			return 2;
		}
		return i4 > i ? 0 : 0;
	case 64:
		if (i5 > i2) {
			return 2;
		}
		if (i5 < i2) {
			return 0;
		}
		if (i4 < i) {
			return 3;
		}
		return i4 > i ? 1 : 0;
	case 128:
		if (i5 > i2) {
			return 3;
		}
		if (i5 < i2) {
			return 1;
		}
		return (i4 >= i && i4 > i) ? 2 : 0;
	case 192:
		if (i5 > i2) {
			return 0;
		}
		if (i5 < i2) {
			return 2;
		}
		if (i4 < i) {
			return 1;
		}
		return i4 > i ? 3 : 0;
	default:
		return 0;
	}
}

int Player_calcLevelXP(Player_t* player, int i)
{
	return (i * 20) + 60;
}

void Player_died(Player_t* player)
{
	if (player->doomRpg->doomCanvas->state != ST_DYING) {
		player->totalTime += DoomRPG_GetUpTimeMS() - player->time;
		player->totalMoves += player->moves;
		player->totalDeaths++;
		Sound_playSound(player->doomRpg->sound, (DoomRPG_randNextByte(&player->doomRpg->random) & 1) ? 5058: 5059, 0, 5);
		DoomCanvas_startShake(player->doomRpg->doomCanvas, 350, 5, 350);
		DoomCanvas_setState(player->doomRpg->doomCanvas, ST_DYING);
	}
}

void Player_removeWeapons(Player_t* player)
{
	player->disabledWeapons = player->weapons;
	player->weapons = 0;
	player->weapon = 0;
	DoomCanvas_updateViewTrue(player->doomRpg->doomCanvas);
}

void Player_restoreWeapons(Player_t* player)
{
	int flg1, flg2;

	flg1 = (player->weapons & 0xE00) == 0;
	if ((player->weapons & 0xE00) != 0)
	{
		flg2 = player->disabledWeapons;
		flg1 = (flg2 & 0xE00) == 0;
	}
	if (!flg1) {
		player->disabledWeapons = flg2 & 0xFFFFF1FF;
	}

	player->weapons |= player->disabledWeapons;
	player->disabledWeapons = 0;
	if ((player->weapons & (1 << player->weapon)) == 0) {
		Player_selectNextWeapon(player);
	}
	DoomCanvas_updateViewTrue(player->doomRpg->doomCanvas);
}

boolean Player_fireWeapon(Player_t* player, Entity_t* entity)
{
	Weapon_t* weapon;
	byte ammo;

	if (player->disabledWeapons != 0 && (player->weapons & (1 << player->weapon)) == 0) {
		return false;
	}

	weapon = &player->doomRpg->combat->weaponInfo[player->weapon];
	ammo = player->ammo[weapon->ammoType];

	if (weapon->ammoUsage <= 0 || ammo - weapon->ammoUsage >= 0) {
		player->ammo[weapon->ammoType] = (ammo - weapon->ammoUsage);
		Combat_performAttack(player->doomRpg->combat, NULL, entity);
		return true;
	}

	Hud_addMessageForce(player->doomRpg->hud, "Not enough ammo!", true);
	return false;
}

void Player_formatTime(Player_t* player, char* text, int maxlen, int timeMs)
{
	int i2 = timeMs / 1000;
	int i3 = i2 / 60;
	int i4 = i3 / 60;
	int i5 = i3 % 60;
	int i6 = i2 - (i3 * 60);

	//SDL_snprintf(text, maxlen, "%02d:%02d", i4, i5); // Original Line Code

	SDL_snprintf(text, maxlen, "%02d:%02d:%02d", i4, i5, i6);
}

void Player_updateDamageFaceTime(Player_t* player)
{
	player->doomRpg->hud->damageTime = player->doomRpg->doomCanvas->time + 500;
	player->doomRpg->hud->damageCount = -1;
	player->doomRpg->hud->isUpdate = true;
}

void Player_setup(Player_t* player)
{
	player->time = DoomRPG_GetUpTimeMS();
	player->moves = 0;
	player->xpGained = 0;
	player->berserkerTics = 0;
	player->dogFamiliar = NULL;
	player->NotebookString[0] = '\0';
	if (player->disabledWeapons) {
		Player_restoreWeapons(player);
	}
}

void Player_addStatusItem(Player_t* player, int i, int i2)
{
	switch (i) {
	case 0:
		Player_addHealth(player, i2);
		if (i2 < 0) {
			Player_painEvent(player, NULL);
			return;
		}
		return;
	case 1:
		Player_addArmor(player, i2);
		return;
	case 2:
		Player_addCredits(player, i2);
		return;
	case 3:
		Player_addXP(player, i2);
		return;
	default:
		return;
	}
}

void Player_fillMonsterStats(Player_t* player, int* statA, int* statB)
{
	Game_t* game;
	int i, totalDeadMonsters, totalMonsters;
	game = player->doomRpg->game;

	totalMonsters = 0;
	totalDeadMonsters = 0;
	for (i = 0; i < game->numEntities; i++) {
		if (game->entities[i].monster) {
			totalMonsters++;
			if ((player->doomRpg->render->mapSprites[(game->entities[i].info & 65535) - 1].info & 0x1000000) || (game->entities[i].info & 0x20000)) {
				totalDeadMonsters++;
			}
		}
	}

	*statA = totalDeadMonsters;
	*statB = totalMonsters;
}

void Player_selectNextWeapon(Player_t* player)
{
	Weapon_t *wpn;
	int i, oldWeapon;
	oldWeapon = player->weapon;

	for (i = oldWeapon + 1; i <= 11; i++) {
		wpn = &player->doomRpg->combat->weaponInfo[i];

		if ((player->weapons & (1 << i)) != 0 && (player->ammo[wpn->ammoType] > 0 || wpn->ammoUsage == 0)) {
			Player_selectWeapon(player, i);
			return;
		}
	}

	if (player->weapon == oldWeapon && oldWeapon != 0)
	{
		for (i = 0; i < oldWeapon; i++) {
			wpn = &player->doomRpg->combat->weaponInfo[i];
			if ((player->weapons & (1 << i)) != 0 && (player->ammo[wpn->ammoType] > 0 || wpn->ammoUsage == 0)) {
				Player_selectWeapon(player, i);
				return;
			}
		}
	}
}

void Player_painEvent(Player_t* player, Entity_t* entity)
{
	DoomCanvas_t* doomCanvas;
	Sprite_t* sprite;
	int priority, sndId;

	if (!player->god)
	{
		player->doomRpg->hud->damageTime = player->doomRpg->doomCanvas->time + 500;
		if (entity) {
			sprite = &player->doomRpg->render->mapSprites[(entity->info & 65535) - 1];
			doomCanvas = player->doomRpg->doomCanvas;
			player->doomRpg->hud->damageDir = Player_calcDamageDir(player, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle, sprite->x, sprite->y);
		}
		else {
			player->doomRpg->hud->damageDir = 0;
		}
		player->doomRpg->hud->damageCount = 1;
		player->doomRpg->hud->isUpdate = true;


		if (((player->weapon == 9 || player->weapon == 10) || player->weapon == 11) && (player->ammo[5] != 0)) {
			Combat_spawnDogBloodParticles(player->doomRpg->combat);
			priority = 3;
			sndId = 5089;
		}
		else {
			priority = 2;
			sndId = 5081;
		}
		Sound_playSound(player->doomRpg->sound, sndId, 0, priority);
		DoomCanvas_startShake(player->doomRpg->doomCanvas, 500, 2, 150);
	}
}

void Player_pain(Player_t* player, int i, int i2)
{
	Sprite_t* sprite;
	Combat_t* combat;
	CombatEntity_t* ce;
	char msg[64], text[128];
	boolean dogDied;
	int armor, h1, h2, damage, dogDamage;

	if (player->god) {
		return;
	}

	SDL_memset(text, 0, 128);

	combat = player->doomRpg->combat;
	if ((combat->curTarget == NULL) && (combat->gotCrit)) {
		strncat(text, "Crit! ", sizeof(text));
	}

	dogDied = false;
	damage = (i + i2);

	if ((player->weapon == 9 || player->weapon == 10 || player->weapon == 11) && player->ammo[5] > 0) {
		strncat(text, "Dog took ", sizeof(text));

		dogDamage = (int)player->ammo[5] - damage;
		if (dogDamage < 0) {
			dogDamage = 0;
		}

		player->ammo[5] = (byte)dogDamage;

		if (dogDamage == 0) {
			if (player->dogFamiliar) {
				sprite = &player->doomRpg->render->mapSprites[(player->dogFamiliar->info & 65535) - 1];
				sprite->info = (sprite->info & -73728) | 0x1000000; 
				sprite->info |= 0x400;
				sprite->info |= player->dogFamiliar->def->tileIndex;
				player->dogFamiliar->info &= -131073;
				sprite->x = player->doomRpg->doomCanvas->destX;
				sprite->y = player->doomRpg->doomCanvas->destY;
				Render_relinkSprite(player->doomRpg->render, sprite);
			}
			else {
				Game_gsprite_alloc(player->doomRpg->game, (player->weapon == 11) ? 22 : player->weapon == 10 ? 21 : 20, 2, player->doomRpg->doomCanvas->destX, player->doomRpg->doomCanvas->destY);
			}

			Sound_playSound(player->doomRpg->sound, 5090, 0, 3);
			player->ammo[5] = 0;
			player->weapons &= 0xfffff1ff;
			Player_selectNextWeapon(player);
			dogDied = true;
		}

		SDL_snprintf(msg, sizeof(msg), "%d", damage);
		strncat(text, msg, sizeof(text));
		i = 0;
		i2 = 0;
	}
	else {
		SDL_snprintf(msg, sizeof(msg), "%d", damage);
		strncat(text, msg, sizeof(text));
	}
	strncat(text, " damage!", sizeof(text));

	if (dogDied) {
		strncat(text, " Dog died!", sizeof(text));
	}

	Hud_addMessage(player->doomRpg->hud, text);

	if (i + i2) {
		ce = &player->ce;

		armor = CombatEntity_getArmor(ce);
		if (armor < i2) {
			i += (i2 - CombatEntity_getArmor(ce));
			CombatEntity_setArmor(ce, 0);
		}
		else {
			CombatEntity_setArmor(ce, armor - i2);
		}

		h1 = (CombatEntity_getHealth(ce) << 16) / (CombatEntity_getMaxHealth(ce) << 8);
		h2 = ((CombatEntity_getHealth(ce) - i) << 16) / (CombatEntity_getMaxHealth(ce) << 8);
		if (h2 > 0) {
			if (h1 > 26 && h2 <= 26) {
				Hud_addMessageForce(player->doomRpg->hud, MenuSystem_buildDivider(player->doomRpg->menuSystem, "Near Death!"), true);
			}
			else if (h1 > 78 && h2 <= 78) {
				Hud_addMessageForce(player->doomRpg->hud, MenuSystem_buildDivider(player->doomRpg->menuSystem, "Low Health!"), true);
			}
			else if (armor > 0 && CombatEntity_getArmor(ce) == 0) {
				Hud_addMessageForce(player->doomRpg->hud, MenuSystem_buildDivider(player->doomRpg->menuSystem, "Armor Gone!"), true);
			}
		}
		Player_addHealth(player, -i);
		if (player->doomRpg->doomCanvas->state == ST_AUTOMAP) {
			DoomCanvas_setState(player->doomRpg->doomCanvas, ST_PLAYING);
		}
		if (CombatEntity_getHealth(ce) <= 0) {
			CombatEntity_setHealth(ce, 0);
			Player_died(player);
		}
	}
}

void Player_addLevelStats(Player_t* player, boolean z)
{
	int secretStats[2];
	int mosterStats[2];
	player->totalTime += DoomRPG_GetUpTimeMS() - player->time;
	player->totalMoves += player->moves;

	if (z && player->doomRpg->render->loadMapID != 2) {
		player->completedLevels |= 1 << (player->doomRpg->render->loadMapID - 1);
		Player_fillSecretStats(player, &secretStats[0], &secretStats[1]);
		if (secretStats[0] == secretStats[1]) {
			player->foundSecretsLevels |= 1 << (player->doomRpg->render->loadMapID - 1);
		}
		Player_fillMonsterStats(player, &mosterStats[0], &mosterStats[1]);
		if (mosterStats[0] == mosterStats[1]) {
			player->killedMonstersLevels |= 1 << (player->doomRpg->render->loadMapID - 1);
		}
	}

	player->berserkerTics = 0;
	player->dogFamiliar = NULL;
}

void Player_selectPrevWeapon(Player_t* player)
{
	Weapon_t* wpn;
	int i, oldWeapon;

	oldWeapon = player->weapon;
	for (i = oldWeapon - 1; i >= 0; i--) {
		wpn = &player->doomRpg->combat->weaponInfo[i];
		if ((player->weapons & (1 << i)) != 0 && (player->ammo[wpn->ammoType] > 0 || wpn->ammoUsage == 0)) {
			Player_selectWeapon(player, i);
			return;
		}
	}
	if (player->weapon == oldWeapon && oldWeapon != 11) {
		for (i = 11; i > oldWeapon; i--) {
			wpn = &player->doomRpg->combat->weaponInfo[i];
			if ((player->weapons & (1 << i)) != 0 && (player->ammo[wpn->ammoType] > 0 || wpn->ammoUsage == 0)) {
				Player_selectWeapon(player, i);
				return;
			}
		}
	}
}

boolean Player_checkStatusItem(Player_t* player, int i, int i2)
{
	switch (i) {
	case 0:
		if (CombatEntity_getHealth(&player->ce) >= i2) {
			return true;
		}
		Hud_addMessage(player->doomRpg->hud, "Insufficient health!");
		return false;
	case 1:
		if (CombatEntity_getArmor(&player->ce) >= i2) {
			return true;
		}
		Hud_addMessage(player->doomRpg->hud, "Insufficient armor!");
		return false;
	case 2:
		if (player->credits >= i2) {
			return true;
		}
		Hud_addMessage(player->doomRpg->hud, "Insufficient funds!");
		return false;
	case 3:
		if (player->currentXP >= i2) {
			return true;
		}
		Hud_addMessage(player->doomRpg->hud, "Insufficient XP!");
		return false;
	default:
		return true;
	}
}

void Player_reset(Player_t* player)
{
	int i;
	CombatEntity_t* ce;

	player->doomRpg->hud->logMessage[0] = '\0';
	player->doomRpg->hud->msgCount = 0;
	player->facingEntity = NULL;
	player->noclip = false;
	player->level = 1;
	player->currentXP = 0;
	player->nextLevelXP = 80;
	player->keys = 0;
	player->credits = 0;

	i = 0;
	do {
		player->ammo[i] = 0;
	} while (++i < 6);

	i = 0;
	do {
		player->inventory[i] = 0;
	} while (++i < 5);

	player->ammo[1] = 8;
	player->weapon = 2;
	player->weapons = 4;
	player->disabledWeapons = 0;
	player->foundSecretsLevels = 0;
	player->killedMonstersLevels = 0;
	player->prevFloorColor = 0;
	player->prevCeilingColor = 0;

	ce = &player->ce;
	CombatEntity_setMaxHealth(ce, 30);
	CombatEntity_setHealth(ce, 30);
	CombatEntity_setMaxArmor(ce, 20);
	CombatEntity_setArmor(ce, 0);
	CombatEntity_setDefense(ce, 16);
	CombatEntity_setStrength(ce, 12);
	CombatEntity_setAgility(ce, 14);
	CombatEntity_setAccuracy(ce, 16);
	player->ce.mType = -1;
	player->totalTime = 0;
	player->totalMoves = 0;
	player->completedLevels = 0;
}

void Player_fillSecretStats(Player_t* player, int* statA, int* statB)
{
	Line_t* line;
	Render_t* render;
	int i, totalSecrets, totalSecretsFound;

	render = player->doomRpg->render;

	totalSecrets = 0;
	totalSecretsFound = 0;
	for (i = 0; i < render->linesLength; i++) {
		line = &render->lines[i];
		if ((line->flags & 8) != 0) {
			totalSecrets++;
			if ((line->flags & 64) != 0) {
				totalSecretsFound++;
			}
		}
	}

	*statA = totalSecretsFound;
	*statB = totalSecrets;
}
void Player_selectWeapon(Player_t* player, int i)
{
	if (player->weapon != i) {
		DoomCanvas_updateViewTrue(player->doomRpg->doomCanvas);
	}
	player->weapon = i;
}

boolean Player_useCollarItem(Player_t* player)
{
	DoomCanvas_t* doomCanvas;
	Entity_t *entity;
	char *text;

	doomCanvas = player->doomRpg->doomCanvas;

	Game_trace(player->doomRpg->game,
		doomCanvas->viewX, doomCanvas->viewY, 
		doomCanvas->viewX + doomCanvas->viewStepX, 
		doomCanvas->viewY + doomCanvas->viewStepY,
		NULL, 22151);
	
	if (player->doomRpg->game->numTraceEntities == 0) {
		Hud_addMessage(player->doomRpg->hud, "No dog within range");
		return false;
	}

	entity = player->doomRpg->game->traceEntities[0];
	if (entity->def->eType != 1) {
		Hud_addMessage(player->doomRpg->hud, "Bad target for Dog Collar");
		return false;
	}
	else if (entity->def->eSubType != 1) {
		// Este texto no aparece en versiones BREW solo en J2ME, 
		// ya que no hacia las llamadas de las funciones (Hud_getMessageBuffer, Hud_finishMessageBuffer)
		// en esta version lo he reparado
		// This text does not appear in BREW versions only in J2ME, 
		// since it did not make the calls of the functions(Hud_getMessageBuffer, Hud_finishMessageBuffer)
		// in this version I have repaired it
		text = Hud_getMessageBuffer(player->doomRpg->hud);
		SDL_snprintf(text, MS_PER_CHAR, "Dog Collar won't fit %s%c", entity->def->name, 0x7f);
		Hud_finishMessageBuffer(player->doomRpg->hud);
		return false;
	}
	else {
		player->dogFamiliar = entity;
		player->weapons &= -3585;
		Combat_spawnCaptureDogParticles(player->doomRpg->combat, -1);
		player->doomRpg->doomCanvas->captureState = 0;
		player->doomRpg->doomCanvas->captureTime = player->doomRpg->doomCanvas->time + 200;
		DoomCanvas_setState(player->doomRpg->doomCanvas, ST_CAPTUREDOG);
		return true;
	}
}

void Player_useItem(Player_t* player, byte item)
{
	EntityDef_t *entity;
	byte indx;
	short* ceilingColor;
	short* floorColor;
	int i;

	entity = EntityDef_find(player->doomRpg->entityDef, 4, item);

	indx = (item - 25);
	if (player->inventory[indx])
	{
		switch (entity->eSubType) {
		case 25:
		case 26:
			Player_addHealth(player, entity->parm);
			Sound_playSound(player->doomRpg->sound, 5134, SND_FLG_NOFORCESTOP, 3);
			break;
		case 27:
			Player_addHealth(player->doomRpg->player, 200);
			Player_addArmor(player->doomRpg->player, 200);
			Sound_playSound(player->doomRpg->sound, 5062, SND_FLG_NOFORCESTOP, 3);
			break;
		case 28:
			Hud_addMessage(player->doomRpg->hud, "Berserker activated!");
			if (player->berserkerTics == 0) {
				player->prevCeilingColor = player->doomRpg->render->ceilingColor[0];
				player->prevFloorColor = player->doomRpg->render->floorColor[0];
				ceilingColor = player->doomRpg->render->ceilingColor;
				floorColor = player->doomRpg->render->floorColor;

#if 0	// No es necesario
				ceilingColor[0] = (((ceilingColor[0] & 0xF7DE) + 0xf000) >> 1);
				floorColor[0]	= (((floorColor[0] & 0xF7DE) + 0xf000) >> 1);
#endif
				for (i = 1; i < player->doomRpg->render->screenWidth; i++) {
					ceilingColor[i] = ceilingColor[0];
					floorColor[i]	= floorColor[0];
				}
			}
			player->berserkerTics += 31;
			DoomCanvas_updateViewTrue(player->doomRpg->doomCanvas);
			Sound_playSound(player->doomRpg->sound, 5062, SND_FLG_NOFORCESTOP, 3);
			break;
		case 29:
			if (Player_useCollarItem(player)) {
				player->inventory[indx] -= 1;
				return;
			}
			return;
		}
		player->inventory[indx] -= 1;
		Game_advanceTurn(player->doomRpg->game);
	}
}