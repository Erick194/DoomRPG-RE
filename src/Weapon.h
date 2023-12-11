#ifndef WEAPON_H__
#define WEAPON_H__

typedef struct Weapon_s
{
	int strMin;
	int strMax;
	int rangeMin;
	int rangeMax;
	int ammoType;
	int ammoUsage;
	int damage;
	short resourceID;
} Weapon_t;

void Weapon_initWeapon(Weapon_t* wpn, int strMin, int strMax, int rangeMin, int rangeMax, int ammoType, int ammoUsage, int damage, short resourceID);
int Weapon_getRangeMinToDist(Weapon_t* wpn);

#endif
