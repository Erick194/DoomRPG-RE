#include <SDL.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "Weapon.h"
#include "SDL_Video.h"


void Weapon_initWeapon(Weapon_t* wpn, int strMin, int strMax, int rangeMin, int rangeMax, int ammoType, int ammoUsage, int damage, short resourceID)
{
    wpn->strMin = strMin;
    wpn->strMax = strMax;
    wpn->rangeMin = rangeMin;
    wpn->rangeMax = rangeMax;
    wpn->ammoType = ammoType;
    wpn->ammoUsage = ammoUsage;
    wpn->damage = damage;
    wpn->resourceID = resourceID;
}

int Weapon_getRangeMinToDist(Weapon_t* wpn)
{
    int range = (wpn->rangeMin * 64);
    return (range * range) + 4096;
}