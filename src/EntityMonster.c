#include <SDL.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "Entity.h"
#include "EntityMonster.h"
#include "SDL_Video.h"

static unsigned short monsterSounds[168] =
{
// Idle,   Sight1, Sight2, Sight3, Attack1, Attack2, Pain, Null,  Death1, Death2, Death3, 0
   5078,   5079,   5080,   5106,   5046,    5047,    5085, 0,     5082,   5084,   5107,   0,        // Zombie Pvt -> Zombie Lt -> Zombie Cpt
   5086,   5087,   0,      0,      5088,    5088,    5089, 0,     5090,   0,      0,      0,        // Hellhound -> Cerberus -> Demon Wolf
   5078,   5079,   5080,   5106,   5046,    5046,    5085, 0,     5082,   5084,   5107,   0,        // Troop -> Commando -> Assassin
   5069,   5070,   5071,   0,      5072,    5096,    5085, 0,     5076,   5077,   0,      0,        // Impling -> Imp -> Imp Lord
   5108,   0,      0,      0,      5100,    5100,    5099, 0,     5101,   0,      0,      0,        // Phantom -> Lost Soul -> Nightmare
   5108,   5093,   0,      0,      5094,    5094,    5099, 0,     5095,   0,      0,      0,        // Bull Demon -> Pinky -> Belphegor
   5108,   5097,   0,      0,      5072,    5096,    5099, 0,     5102,   0,      0,      0,        // Malwrath -> Cacodemon -> Wretched
   5108,   5109,   0,      0,      5072,    5096,    5110, 0,     5111,   0,      0,      0,        // Beholder -> Rahovart -> Pain Elemental
   5104,   5103,   0,      0,      5105,    5112,    5085, 0,     5113,   0,      0,      5114,     // Ghoul -> Fiend -> Revenant
   5078,   5115,   0,      0,      5116,    5047,    5117, 0,     5118,   0,      0,      0,        // Behemoth -> Mancubus -> Druj
   5119,   5120,   0,      0,      5121,    5121,    5122, 0,     5123,   0,      0,      5124,     // Infernis -> Archvile -> Apollyon
   5108,   5125,   0,      0,      5072,    5072,    5099, 0,     5126,   0,      0,      0,        // Ogre -> Hell Knight -> Baron
   5108,   5128,   0,      0,      5051,    5130,    5099, 0,     5129,   0,      0,      5127,     // Cyberdemon
   0,      5097,   0,      0,      5096,    5096,    5137, 0,     5138,   0,      0,      0         // Kronos
};

EntityMonster_t* EntityMonster_startup(EntityMonster_t* entityMonster)
{
    if (entityMonster == NULL)
    {
        entityMonster = SDL_malloc(sizeof(EntityMonster_t));
        if (entityMonster == NULL) {
            return NULL;
        }
    }

    //entityMonster->ce = SDL_malloc(sizeof(CombatEntity_t));
    EntityMonster_reset(entityMonster);
    return entityMonster;
}

int EntityMonster_getSoundRnd(EntityMonster_t* entityMonster, int field)
{
    int i, indx;

    indx = (entityMonster->ce.mType * 12) + field;
    i = 0;
    do
    {
        if (monsterSounds[indx + i] == 0)
            break;
    } while (++i < 3);

    if (i) {
        
        indx = DoomRPG_randNextByte(&entityMonster->doomRpg->random) % i;
        return monsterSounds[((entityMonster->ce.mType * 12) + field) + indx];
    }

    return 0;
}

int EntityMonster_getSoundID(EntityMonster_t* entityMonster, int field)
{
    return monsterSounds[(entityMonster->ce.mType * 12) + field];
}

void EntityMonster_reset(EntityMonster_t* entityMonster)
{
    entityMonster->prevOnList = NULL;
    entityMonster->nextOnList = NULL;
    entityMonster->target = NULL;
    entityMonster->nextAttacker = NULL;
    entityMonster->frameTime = 0;
    entityMonster->y = 0;
    entityMonster->x = 0;
    entityMonster->animFrameTime = 0;
    entityMonster->damageBlendTime = 0;
}