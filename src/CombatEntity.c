#include <SDL.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "SDL_Video.h"

byte monsterAttacks[] = { 2, 3, 12, 13, 4, 4, 15, 12, 13, 14, 13, 12, 15, 13, 15, 14, 7, 12, 7, 3, 15, 15, 16, 17, 7, 17, 12, 13 };


void CombatEntity_initCombatEntity(CombatEntity_t* ce, int type, int health, int armor, int defense, int strength, int agility, int accuracy)
{
	CombatEntity_setMaxHealth(ce, health);
	CombatEntity_setHealth(ce, health);
	CombatEntity_setMaxArmor(ce, armor);
	CombatEntity_setArmor(ce, armor);
	CombatEntity_setDefense(ce, defense);
	CombatEntity_setStrength(ce, strength);
	CombatEntity_setAgility(ce, agility);
	CombatEntity_setAccuracy(ce, accuracy);
	ce->mType = type;
}


int CombatEntity_getHealth(CombatEntity_t* ce)
{
	return (ce->param1 & 0xff);
}

int CombatEntity_getMaxHealth(CombatEntity_t* ce)
{
	return (ce->param1 & 0xFF00) >> 8;
}

int CombatEntity_getArmor(CombatEntity_t* ce)
{
	return (ce->param1 & 0xFF0000) >> 16;
}

int CombatEntity_getMaxArmor(CombatEntity_t* ce)
{
	return (ce->param1 & 0xFF000000) >> 24;
}

void CombatEntity_setHealth(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param1 = ((ce->param1 & 0xFFFFFF00) | i);
}

void CombatEntity_setMaxHealth(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param1 = ((ce->param1 & 0xFFFF00FF) | i << 8);
}

void CombatEntity_setArmor(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param1 = ((ce->param1 & 0xFF00FFFF) | i << 16);
}

void CombatEntity_setMaxArmor(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param1 = ((ce->param1 & 0xFFFFFF) | i << 24);
}

//----------------------------------

int CombatEntity_getDefense(CombatEntity_t* ce)
{
	return (ce->param2 & 0xff);
}

int CombatEntity_getStrength(CombatEntity_t* ce)
{
	return (ce->param2 & 0xFF00) >> 8;
}

int CombatEntity_getAgility(CombatEntity_t* ce)
{
	return (ce->param2 & 0xFF0000) >> 16;
}

int CombatEntity_getAccuracy(CombatEntity_t* ce)
{
	return (ce->param2 & 0xFF000000) >> 24;
}

void CombatEntity_setDefense(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param2 = ((ce->param2 & 0xFFFFFF00) | i);
}

void CombatEntity_setStrength(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param2 = ((ce->param2 & 0xFFFF00FF) | i << 8);
}

void CombatEntity_setAgility(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param2 = ((ce->param2 & 0xFF00FFFF) | i << 16);
}

void CombatEntity_setAccuracy(CombatEntity_t* ce, int i)
{
	if (i < 0) {
		i = 0;
	}
	else if (i > 0xff) {
		i = 0xff;
	}
	ce->param2 = ((ce->param2 & 0xFFFFFF) | i << 24);
}

int CombatEntity_getEXP(CombatEntity_t* ce)
{
	return (((CombatEntity_getDefense(ce) + CombatEntity_getStrength(ce)) * 5) +
			((CombatEntity_getAgility(ce) + CombatEntity_getAccuracy(ce)) * 3) +
			((CombatEntity_getMaxHealth(ce) + CombatEntity_getMaxArmor(ce)) * 5) + 49) / 50;
}

void CombatEntity_setupEnemy(CombatEntity_t* ce, int i)
{
	int health, armor, calcDefence;
	int i2 = (i << 8) / 1024;
	int i3 = (i2 << 9) >> 8;

	health = ((CombatEntity_getMaxHealth(ce) << 8) * ((i + ((i3 * (DoomRPG_randNextInt(&ce->doomRpg->random) & 255)) >> 8)) - i2)) >> 16;
	CombatEntity_setHealth(ce, health);
	CombatEntity_setMaxHealth(ce, health);

	calcDefence = (i + ((i3 * (DoomRPG_randNextInt(&ce->doomRpg->random) & 255)) >> 8)) - i2;
	armor = ((CombatEntity_getMaxArmor(ce) << 8) * calcDefence) >> 16;
	CombatEntity_setArmor(ce, armor);
	CombatEntity_setMaxArmor(ce, armor);

	CombatEntity_setDefense(ce, ((CombatEntity_getDefense(ce) << 8) * calcDefence) >> 16);
	CombatEntity_setStrength(ce, ((CombatEntity_getStrength(ce) << 8) * ((i + ((i3 * (DoomRPG_randNextInt(&ce->doomRpg->random) & 255)) >> 8)) - i2)) >> 16);
	CombatEntity_setAgility(ce, ((CombatEntity_getAgility(ce) << 8) * ((i + ((i3 * (DoomRPG_randNextInt(&ce->doomRpg->random) & 255)) >> 8)) - i2)) >> 16);
	CombatEntity_setAccuracy(ce, ((CombatEntity_getAccuracy(ce) << 8) * ((i + ((i3 * (DoomRPG_randNextInt(&ce->doomRpg->random) & 255)) >> 8)) - i2)) >> 16);
}

int CombatEntity_calcWeaponDamage(DoomRPG_t* doomrpg, Weapon_t* wpn, CombatEntity_t* ce)
{
	Weapon_t *weaponInfo;
	int mType, ammoType;

	weaponInfo = &doomrpg->combat->weaponInfo[0];
	ammoType = wpn->ammoType;
	mType = ce->mType;

	if (mType != 0) {
		if (mType == 1) {
			if (ammoType == 2) {
				return 409;
			}
		}
		else if (mType == 2) {
			if (ammoType == 4) {
				return 512;
			}
			if (wpn == weaponInfo) {
				return 204;
			}
		}
		else if (mType == 3) {
			if (ammoType == 2) {
				return 512;
			}
			if (ammoType == 4) {
				return 165;
			}
		}
		else if (mType == 4) {
			if (wpn == weaponInfo) {
				return 191;
			}
			if (ammoType == 0) {
				return 2550;
			}
		}
		else if (mType == 5) {
			if (ammoType == 2) {
				return 382;
			}
			if (ammoType == 3) {
				return 191;
			}
		}
		else if (mType == 6) {
			if (wpn == weaponInfo) {
				return 768;
			}
			if (ammoType == 5) {
				return 140;
			}
		}
		else if (mType == 7) {
			if (ammoType == 5) {
				return 637;
			}
			if (ammoType == 1) {
				return 165;
			}
		}
		else if (mType == 8) {
			if (wpn == weaponInfo) {
				return 768;
			}
			if (ammoType == 2) {
				return 140;
			}
		}
		else if (mType == 9) {
			if (ammoType == 3) {
				return 512;
			}
			if (ammoType == 5) {
				return 165;
			}
		}
		else if (mType == 10) {
			if (ammoType == 0) {
				return 1530;
			}
			if (ammoType == 1) {
				return 165;
			}
		}
		else if (mType == 11) {
			if (ammoType == 2) {
				return 512;
			}
		}
		else if (mType == 12) {
			if (ammoType == 3) {
				return 382;
			}
		}
		else if (mType == 13 && ammoType == 4) {
			return 382;
		}
		return 256;
	}

	if (wpn == weaponInfo) {
		return 768;
	}

	return 204;
}

void CombatEntity_calcDamage(DoomRPG_t* doomrpg, CombatEntity_t* ce1, Weapon_t* wpn, CombatEntity_t* ce2, int i, int dist, int* getDamage, int* getDamageArmmor)
{
	int randStr, strength, defense, calWpnDmg;
	int calDmg, calDmgArm, decDmg;
	int distance, distStep;

	randStr = DoomRPG_randNextByte(&doomrpg->random);
	strength = CombatEntity_getStrength(ce1) << 16;
	defense = CombatEntity_getDefense(ce2) << 8;
	calWpnDmg = CombatEntity_calcWeaponDamage(doomrpg, wpn, ce2);

	calDmg = (((((((wpn->strMin << 8) + ((randStr * ((wpn->strMax - wpn->strMin) << 8)) >> 8)) * (strength / defense)) >> 8) * i) >> 8) * calWpnDmg) >> 8;

	distance = 4096;
	distStep = 2;
	decDmg = (calDmg * 76) >> 8;
	while (distance < dist - Weapon_getRangeMinToDist(wpn)) {
		calDmg -= decDmg;
		distStep++;
		distance = (distStep * 64) * (distStep * 64);
	}
	if (calDmg < 256) {
		calDmg = 256;
	}
	else if (calDmg > 255744) {
		calDmg = 255744;
	}

	calDmgArm = (calDmg * wpn->damage) >> 8;
	*getDamageArmmor = (calDmgArm + 128) >> 8;
	*getDamage = ((calDmg - calDmgArm) + 128) >> 8;
}

int CombatEntity_calcHit(DoomRPG_t* doomrpg, CombatEntity_t* ce1, Weapon_t* wnp, CombatEntity_t* ce2, int dist) {
	int randHit, calcHit, decHit;
	int distance, distStep, dist2;

	calcHit = ((((CombatEntity_getAccuracy(ce1) << 16) / (CombatEntity_getAgility(ce2) << 8)) * 128) >> 8) + ((wnp->rangeMax << 16) / 51200);
	dist2 = dist - Weapon_getRangeMinToDist(wnp);

	if (dist2 > 0 && wnp->rangeMin == 0) {
		return 0;
	}
	if (wnp->ammoType != 2) {
		distance = 4096;
		distStep = 2;
		decHit = (calcHit * 76) >> 8;
		while (distance < dist2) {
			calcHit -= decHit;
			distStep++;
			distance = (distStep * 64) * (distStep * 64);
		}
	}
	randHit = DoomRPG_randNextByte(&doomrpg->random);
	if (randHit < calcHit) {
		return (randHit < ((calcHit << 8) / 5120)) ? COMBATENTITY_CRIT : COMBATENTITY_HIT;
	}
	return COMBATENTITY_MISS;
}