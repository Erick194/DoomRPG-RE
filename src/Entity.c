
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Game.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "Player.h"
#include "Hud.h"
#include "Render.h"
#include "Sound.h"
#include "SDL_Video.h"

void Entity_attack(Entity_t* entity)
{
	if (!(entity->info & 0x10000)) {
		entity->info |= 0x10000;
		entity->monster->nextAttacker = entity->doomRpg->game->combatMonsters;
		entity->doomRpg->game->combatMonsters = entity;
	}
}

boolean Entity_aiGoal_MOVE(Entity_t* entity, int srcX, int srcY, int destX, int destY)
{
    int sX, sY, dX, dY;
    int closestPathDist, pathDist;

    sX = srcX >> 6;
    sY = srcY >> 6;
    dX = destX >> 6;
    dY = destY >> 6;

    closestPathDist = 9999;
    int i10 = 65415;

    entity->visitOrderCount = 0;
    if (entity->def->eSubType == 4 || entity->def->eSubType == 13) {
        i10 = 65415 & -3073;
    }
    else if (entity->def->eSubType == 6 || entity->def->eSubType == 7) {
        i10 = 65415 & -2049;
    }
    else if (entity->def->eSubType == 10) {
        i10 = 65415 & -1025;
    }
    if (Entity_checkLineOfSight(entity, srcX, srcY, srcX + 64, srcY, i10)) {
        pathDist = Entity_calcPath(entity, sX + 1, sY, dX, dY, 0, i10);
        if (pathDist < closestPathDist) {
            closestPathDist = pathDist;
            entity->visitOrder[entity->visitOrderCount++] = 2;
        }
        else if (pathDist == closestPathDist) {
            entity->visitOrder[entity->visitOrderCount++] = 2;
        }
    }
    if (Entity_checkLineOfSight(entity, srcX, srcY, srcX - 64, srcY, i10)) {
        pathDist = Entity_calcPath(entity, sX - 1, sY, dX, dY, 1, i10);
        if (pathDist < closestPathDist) {
            closestPathDist = pathDist;
            entity->visitOrder[entity->visitOrderCount++] = 3;
        }
        else if (pathDist == closestPathDist) {
            entity->visitOrder[entity->visitOrderCount++] = 3;
        }
    }
    if (Entity_checkLineOfSight(entity, srcX, srcY, srcX, srcY + 64, i10)) {
        pathDist = Entity_calcPath(entity, sX, sY + 1, dX, dY, 2, i10);
        if (pathDist < closestPathDist) {
            closestPathDist = pathDist;
            entity->visitOrder[entity->visitOrderCount++] = 1;
        }
        else if (pathDist == closestPathDist) {
            entity->visitOrder[entity->visitOrderCount++] = 1;
        }
    }
    if (Entity_checkLineOfSight(entity, srcX, srcY, srcX, srcY - 64, i10)) {
        pathDist = Entity_calcPath(entity, sX, sY - 1, dX, dY, 3, i10);
        if (pathDist < closestPathDist) {
            entity->visitOrder[entity->visitOrderCount++] = 0;
        }
        else if (pathDist == closestPathDist) {
            entity->visitOrder[entity->visitOrderCount++] = 0;
        }
    }
    if (entity->visitOrderCount == 0) {
        return false;
    }
    switch (entity->visitOrder[(DoomRPG_randNextByte(&entity->doomRpg->random) & 3) % entity->visitOrderCount]) {
    case 0:
        entity->monster->x = srcX;
        entity->monster->y = srcY - 64;
        break;
    case 1:
        entity->monster->x = srcX;
        entity->monster->y = srcY + 64;
        break;
    case 2:
        entity->monster->x = srcX + 64;
        entity->monster->y = srcY;
        break;
    case 3:
        entity->monster->x = srcX - 64;
        entity->monster->y = srcY;
        break;
    }
    Game_unlinkEntity(entity->doomRpg->game, entity);
    Game_linkEntity(entity->doomRpg->game, entity, entity->monster->x >> 6, entity->monster->y >> 6);
    return true;

}

Entity_t* Entity_powerCouplingDied(void* gameStruct)
{
    Entity_t* getEntity, *curEntity;
    Line_t* lines;
    Game_t* game;
    int i;

    game = (Game_t*)gameStruct;

    getEntity = NULL;
    for (i = game->firstSpecialEntityIndex; i <= game->lastSpecialEntityIndex; i++) {
        curEntity = &game->entities[i];
        if ((curEntity->def->eType == 7) && (curEntity->info & 0x200000)) {
            lines = &game->doomRpg->render->lines[(curEntity->info & 65535) - 1];
            if (curEntity->def->eSubType == 3) { // Portal
                lines->texture = 92;
                getEntity = curEntity;
            }
            else if (curEntity->def->eSubType == 4) {
                lines->texture = 91;
            }
        }
    }
    return getEntity;
}

void Entity_died(Entity_t* entity)
{
    GameSprite_t* gSprite;
    DoomCanvas_t* doomCanvas;
    DoomRPG_t* doomRpg;
    Sprite_t* sprite, * sprTmp;
    Player_t* player;
    Game_t* game;
    Entity_t* ent;

    int snd, rnd;

    doomRpg = entity->doomRpg;
    doomCanvas = doomRpg->doomCanvas;
    player = doomRpg->player;
    game = doomRpg->game;

    if (entity->info & 0x40000) {
        entity->info &= 0xfffbffff;

        sprite = &entity->doomRpg->render->mapSprites[(entity->info & 0xFFFF) - 1];

        switch (entity->def->eType) {

            case 1: // Enemy
                Player_addXP(player, CombatEntity_getEXP(&entity->monster->ce));
                sprite->info = (sprite->info & 0xffffe1ff) | 0x800;
                sprite->ent->monster->animFrameTime = entity->doomRpg->doomCanvas->time + 250;

                if (entity->info & 0x20000) {
                    sprite->info |= 0x10000;
                    Sound_playSound(entity->doomRpg->sound, 5091, 0, 3);
                }
                else {
                    sprite->info |= 0x1000000;

                    // Death Sound
                    snd = EntityMonster_getSoundRnd(sprite->ent->monster, 8);
                    if (snd != 0) {
                        Sound_playSound(entity->doomRpg->sound, snd, 0, 5);
                    }
                    Entity_trimCorpsePile(entity, sprite->x, sprite->y);
                }

                Game_deactivate(entity->doomRpg->game, entity);
                Game_unlinkEntity(entity->doomRpg->game, entity);
                Entity_spawnDropItem(entity);

                if ((entity->def->eSubType == 7) && !(entity->info & 0x10000000)) { // Beholder / Rahovart / Pain Elemental
                    entity->info |= 0x10000000;
                    sprTmp = &entity->doomRpg->render->mapSprites[(entity->info & 0xFFFF) + 0];
                    if (Game_findMapEntityXYFlag(game, sprite->x, sprite->y, 0xFF87) == NULL) {
                        sprTmp->info &= 0xfffeffff;
                        sprTmp->ent->monster->x = sprTmp->x = sprite->x;
                        sprTmp->ent->monster->y = sprTmp->y = sprite->y;
                        Render_relinkSprite(entity->doomRpg->render, sprTmp);
                        Game_linkEntity(game, sprTmp->ent, sprTmp->x >> 6, sprTmp->y >> 6);
                    }
                    else {
                        sprTmp->ent->info |= 0x20000;
                    }

                    sprTmp = &entity->doomRpg->render->mapSprites[(entity->info & 0xFFFF) + 1];
                    if ((Game_findMapEntityXYFlag(game, sprite->x - 64, sprite->y, 0xFF87) == NULL) && (sprite->x - 64 != doomCanvas->destX || sprite->y != doomCanvas->destY)) {
                        sprTmp->x = sprite->x - 64;
                        sprTmp->y = sprite->y;
                    }
                    else if ((Game_findMapEntityXYFlag(game, sprite->x + 64, sprite->y, 0xFF87) == NULL) && (sprite->x + 64 != doomCanvas->destX || sprite->y != doomCanvas->destY)) {
                        sprTmp->x = sprite->x + 64;
                        sprTmp->y = sprite->y;
                    }
                    else if ((Game_findMapEntityXYFlag(game, sprite->x, sprite->y - 64, 0xFF87) == NULL) && (sprite->x != doomCanvas->destX || sprite->y - 64 != doomCanvas->destY)) {
                        sprTmp->x = sprite->x;
                        sprTmp->y = sprite->y - 64;
                    }
                    else if ((Game_findMapEntityXYFlag(game, sprite->x, sprite->y + 64, 0xFF87) == NULL) && (sprite->x != doomCanvas->destX || sprite->y + 64 != doomCanvas->destY)) {
                        sprTmp->x = sprite->x;
                        sprTmp->y = sprite->y + 64;
                    }
                    else {
                        sprTmp->ent->info |= 0x20000;
                        break;
                    }

                    sprTmp->ent->monster->x = sprTmp->x;
                    sprTmp->ent->monster->y = sprTmp->y;
                    sprTmp->info &= 0xFFFEFFFF;
                    Render_relinkSprite(entity->doomRpg->render, sprTmp);
                    Game_linkEntity(game, sprTmp->ent, sprTmp->x >> 6, sprTmp->y >> 6);
                }
                else if (entity->def->eSubType == 12) { // Cyberdemon
                    Game_executeTile(game, 0x480, 0x4c0, 0x100);
                    if (sprite->info & 0x1000000) {
                        sprite->info = sprite->info & 0xfeffffff | 0x10000;
                        entity->info = entity->info | 0x20000;
                        player->god = true;
                        gSprite = Game_gsprite_allocAnim(game, 1, sprite->x + 16, sprite->y);
                        gSprite->time += 33;
                        gSprite = Game_gsprite_allocAnim(game, 1, sprite->x - 16, sprite->y);
                        gSprite->time += 66;
                        Game_gsprite_allocAnim(game, 1, sprite->x, sprite->y);
                    }
                }
                else if (entity->def->eSubType == 8) { // Ghoul / Fiend / Revenant
                    Game_gsprite_allocAnim(game, 1, sprite->x, sprite->y);
                    if ((sprite->info & 0x1000000) != 0) {
                        sprite->info = (sprite->info & 0xfeffffff) | 0x10000;
                        entity->info |= 0x20000;
                    }
                }
                else if (entity->def->eSubType == 13) { // Kronos
                    Game_executeTile(game, 1984, 0, 0x100);
                    sprite->info = (sprite->info & 0xfeffffff) | 0x10000;
                    entity->info |= 0x20000;
                }
                break;

            case 2: // Humans
                Game_remove(game, entity);
                break;

            case 10: // Fire
                gSprite = Game_gsprite_allocAnim(game, 13, sprite->x, sprite->y);
                gSprite->flags |= 4;
                gSprite->sprite->renderMode = 7;
                Game_remove(game, entity);
                Hud_addMessage(entity->doomRpg->hud, "Fire cleared!");
                Player_addXP(entity->doomRpg->player, 2);
                break;

            case 12: // Destructible Object
                switch (entity->def->eSubType) {

                    case 1: // Barrel
                        Game_gsprite_allocAnim(game, 1, sprite->x, sprite->y);
                        break;

                    case 2: // Crate
                        rnd = DoomRPG_randNextByte(&doomRpg->random);
                        if (rnd < 2) {
                            Game_gsprite_allocAnim(game, 1, sprite->x, sprite->y);
                            Hud_addMessage(entity->doomRpg->hud, "Trapped!");
                            break;
                        }
                        else {
                            if (rnd < 4) {
                                entity->def = EntityDef_find(entity->doomRpg->entityDef, (byte)3, (byte)23);
                            }
                            else if (rnd < 12) {
                                entity->def = EntityDef_find(entity->doomRpg->entityDef, (byte)3, (byte)22);
                            }
                            else if (rnd < 24) {
                                entity->def = EntityDef_find(entity->doomRpg->entityDef, (byte)4, (byte)25);
                            }
                            else if (rnd < 150) {
                                entity->def = EntityDef_find(entity->doomRpg->entityDef, (byte)3, (byte)21);
                            }
                            else {
                                if (rnd >= 213) {
                                    break;
                                }
                                entity->def = EntityDef_find(entity->doomRpg->entityDef, (byte)6, (byte)(DoomRPG_randNextByte(&entity->doomRpg->random) % 5));
                            }

                            entity->info |= 0x8000000;
                            sprite->info = (sprite->info & -512) | entity->def->tileIndex;
                            DoomCanvas_checkFacingEntity(entity->doomRpg->doomCanvas);
                            return;
                        }

                    case 3: // Jammed Door / Weak Wall
                        Hud_addMessage(doomRpg->hud, "Door cleared!");
                        Player_addXP(entity->doomRpg->player, 1);
                        break;

                    case 4: // Power Coupling
                        Game_gsprite_allocAnim(game, 1, sprite->x, sprite->y);
                        ++game->powerCouplingDeaths;
                        if (game->powerCouplingDeaths == 2) {
                            ent = Entity_powerCouplingDied(entity->doomRpg->game);
                            game->activePortal = false;
                            Game_executeTile(game, (ent->linkIndex % 32) << 6, (ent->linkIndex / 32) << 6, 0x100);
                        }
                        break;
                }
                Game_remove(game, entity);
                break;
        }

        DoomCanvas_checkFacingEntity(entity->doomRpg->doomCanvas);
    }
}

int Entity_calcWorldDistance(Entity_t* entity, int x, int y)
{
    Render_t* render;
    Sprite_t* sprite;
    Line_t* line;
    int pointX, pointY;

    render = entity->doomRpg->render;

    if ((entity->info & 2097152) == 0) {
        sprite = &render->mapSprites[(entity->info & 65535) - 1];
        return ((sprite->x - x) * (sprite->x - x)) + ((sprite->y - y) * (sprite->y - y));
    }

    line = &render->lines[(entity->info & 65535) - 1];
    pointX = line->vert1.x + ((line->vert2.x - line->vert1.x) / 2);
    pointY = line->vert1.y + ((line->vert2.y - line->vert1.y) / 2);
    return ((pointX - x) * (pointX - x)) + ((pointY - y) * (pointY - y));
}

void Entity_spawnDropItem(Entity_t* entity)
{
    Entity_t* entityDrop;
    Sprite_t* sprite;
    Game_t* game;
    int rnd;
    byte eSubType;
    int drop_eType, drop_eSubType;

    rnd = DoomRPG_randNextInt(&entity->doomRpg->random);
    eSubType = entity->def->eSubType;

    if ((entity->info & 0x100000) || eSubType == 1 || eSubType == 12 || eSubType == 13 || (rnd & 0xFF) < 51) {
        return;
    }

    entity->info |= 0x100000;
    int n = rnd >> 8;
    int n2 = n >> 1;
    drop_eType = -1;
    drop_eSubType = 0;
    int n5 = n2 & 0xFF;
    int n6 = n2 >> 8;

    if (n & 0x1) {
        if (n5 < 115) {
            drop_eType = 3;
            drop_eSubType = 20; // Health Vial
        }
        else if (n5 < 230) {
            drop_eType = 3;
            drop_eSubType = 21; // Armor Shard
        }
        else {
            int n7 = 0;
            if (entity->def->parm == 294) {
                n7 = 3;
            }
            else if (entity->def->parm == 467) {
                n7 = 8;
            }
            else if (entity->def->parm == 640) {
                n7 = 13;
            }
            drop_eType = 3;
            if (n5 < 230 + n7) {
                drop_eSubType = 23; // 5 credits
            }
            else {
                drop_eSubType = 22; // 1 credit
            }
        }
    }
    else {
        boolean b3 = n5 < 76;
        boolean b4 = (n6 & 0xFF) < 15;
        if (eSubType == 0) { // Zombie Pvt / Zombie Lt / Zombie Cpt
            drop_eType = (b4 ? 16 : 6);
            drop_eSubType = (b3 ? 2 : 1); // (Shell Clip / Shell Clips) : (Bullet Clip / Bullet Clips)
        }
        else if (eSubType == 2) { // Troop / Commando / Assassin
            drop_eType = ((b4 || b3) ? 16 : 6);
            drop_eSubType = 1; // Bullet Clip / Bullet Clips
        }
        else if (eSubType == 6) { // Malwrath / Cacodemon / Wretched
            drop_eType = 4;
            drop_eSubType = (b4 ? 26 : 25); // Lg Medkit : Sm Medkit
        }
        else if (eSubType == 7) { // Beholder / Rahovart / Pain Elemental
            drop_eType = 3;
            drop_eSubType = 23; // 5 credits
        }
        else if (eSubType == 8) { // Ghoul / Fiend / Revenant
            drop_eType = (b4 ? 16 : 6);
            drop_eSubType = 3; // Rocket / Rockets
        }
        else if (eSubType == 9) { // Behemoth / Mancubus / Druj
            drop_eType = (b4 ? 16 : 6);
            drop_eSubType = 4; // Cell Clip / Cell Clips
        }
        else if (eSubType == 10) { // Infernis / Archvile / Apollyon
            if (b3) {
                drop_eType = 4;
                drop_eSubType = (b4 ? 26 : 25); // Lg Medkit : Sm Medkit
            }
            else {
                drop_eType = 3;
                drop_eSubType = 23; // 5 credits
            }
        }
        else if (eSubType == 11) { // Ogre / Hell Knight / Baron
            drop_eType = 16;
            drop_eSubType = (b3 ? 4 : 2); // Cell Clips : Shell Clips
        }
    }

    if (drop_eType != -1) {
        game = entity->doomRpg->game;
        entityDrop = &game->entities[game->firstDropIndex + game->dropIndex];
        entityDrop->def = EntityDef_find(entity->doomRpg->entityDef, (byte)drop_eType, (byte)drop_eSubType);
        game->dropIndex = (game->dropIndex + 1) % 8;
        Game_unlinkEntity(entity->doomRpg->game, entityDrop);

        sprite = &entity->doomRpg->render->mapSprites[(entityDrop->info & 0xffff) - 1];

        sprite->info &= 0xFBFEE000;
        sprite->info |= entityDrop->def->tileIndex;
        sprite->x = entity->doomRpg->render->mapSprites[(entity->info & 0xffff) - 1].x;
        sprite->y = entity->doomRpg->render->mapSprites[(entity->info & 0xffff) - 1].y;
        sprite->ent = entityDrop;
        if (drop_eType == 16) {
            sprite->info |= 0x4000000;
        }
        Render_relinkSprite(entity->doomRpg->render, sprite);
        Game_linkEntity(entity->doomRpg->game, entityDrop, sprite->x >> 6, sprite->y >> 6);
    }
}

int Entity_calcPath(Entity_t* entity, int srcX, int srcY, int destX, int destY, int n5, int n6)
{
    boolean sight;
    int dist, newDist, stepX, stepY, sX, sY, dX, dY;
    int i, j;

    if (srcX == destX && srcY == destY) {
        return 0;
    }

    for (i = 0; i < 2; ++i) {
        dist = 999999;
        stepX = 0;
        stepY = 0;
        for (j = 0; j < 4; ++j) {
            sX = 0;
            sY = 0;
            switch (j) {
                case 0: {
                    sX = 1;
                    sY = 0;
                    break;
                }
                case 1: {
                    sX = -1;
                    sY = 0;
                    break;
                }
                case 2: {
                    sX = 0;
                    sY = 1;
                    break;
                }
                case 3: {
                    sX = 0;
                    sY = -1;
                    break;
                }
            }

            sight = Entity_checkLineOfSight(entity, srcX << 6, srcY << 6, (srcX + sX) << 6, (srcY + sY) << 6, n6);
            newDist = ((srcX + sX - destX) * (srcX + sX - destX) + (srcY + sY - destY) * (srcY + sY - destY));
            if (sight && (newDist < dist)) {
                dist = newDist;
                stepX = sX;
                stepY = sY;
            }
        }
        srcX += stepX;
        srcY += stepY;
        entity->visitDist[(i * 2) + 0] = srcX;
        entity->visitDist[(i * 2) + 1] = srcY;
        if (srcX == destX && srcY == destY) {
            break;
        }
    }

    if (i == 2) {
        dX = entity->visitDist[2];
        dY = entity->visitDist[3];
        dist = (dX - destX) * (dX - destX) + (dY - destY) * (dY - destY);

        return i + dist;
    }

    return i;
}

int Entity_aiMoveToGoal(Entity_t* entity)
{
    Game_t* game;
    Sprite_t* sprite;
    DoomCanvas_t* doomCanvas;

    game = entity->doomRpg->game;
    doomCanvas = entity->doomRpg->doomCanvas;

    sprite = &entity->doomRpg->render->mapSprites[(entity->info & 65535) - 1];

    int iVar4 = 3;
    if ((unsigned int)(sprite->info << 19) >> 28) {
        iVar4 = 0;
    }

    sprite->info = sprite->info & 0xffffe1ffU | iVar4 << 9;

    int time = entity->doomRpg->doomCanvas->time;
    if (entity->def->eSubType == 4){
        time += (byte)sprite->ent + 250;
    }
    else {
        time += (8 * (byte)sprite->ent) + 4000;
    }
    sprite->ent->monster->animFrameTime = time;

    if (entity->def->eSubType == 12) {
        Sound_playSound(entity->doomRpg->sound, 5127, 0, 3);
    }

    int i = (entity->def->eSubType == 4 || entity->def->eSubType == 13) ? 3 : 1;

    boolean z = entity->def->eSubType == 5 || entity->def->eSubType == 4 || entity->def->eSubType == 1 || entity->def->eSubType == 13;
 
    int i2 = ((sprite->x - doomCanvas->viewX) * (sprite->x - doomCanvas->viewX)) + ((sprite->y - doomCanvas->viewY) * (sprite->y - doomCanvas->viewY));
 
    entity->monster->frameTime++;
    if (entity->monster->frameTime < i) {
        if ((sprite->x == doomCanvas->viewX || sprite->y == doomCanvas->viewY) && i2 <= 4096) {
            entity->monster->frameTime = i;
        }
        else if (Entity_aiGoal_MOVE(entity, sprite->x, sprite->y, doomCanvas->destX, doomCanvas->destY)) {
            game->interpolatingMonsters = true;
            return true;
        }
    }

    if (entity->monster->frameTime != i || !z) {
        return false;
    }

    if ((sprite->x != doomCanvas->viewX && sprite->y != doomCanvas->viewY) || i2 > 4096) {
        return false;
    }

    Game_trace(game, sprite->x, sprite->y, doomCanvas->viewX, doomCanvas->viewY, entity, 22151);
    if (game->numTraceEntities != 0 && (game->numTraceEntities != 1 || game->traceEntities[0]->def->eType != 8)) {
        return false;
    }

    Entity_attack(entity);
    return false;

}

void Entity_initspawn(Entity_t* entity)
{
    CombatEntity_t* monster;
    Sprite_t* sprite;
    if (entity->def->eType == 1) { // Enemy
        sprite = &entity->doomRpg->render->mapSprites[(entity->info & 65535) - 1];
        entity->monster->x = sprite->x;
        entity->monster->y = sprite->y;
        monster = &entity->doomRpg->combat->monsters[entity->def->eSubType];

        entity->monster->ce.param1 = monster->param1;
        entity->monster->ce.param2 = monster->param2;
        entity->monster->ce.mType = monster->mType;
        CombatEntity_setupEnemy(&entity->monster->ce, entity->def->parm);

        // define spectre render mode
        if (entity->def->eSubType == 5 && entity->def->parm == 640) {
            sprite->renderMode = 9;
        }

        if (DoomRPG_randNextByte(&entity->doomRpg->random) & 1) {
            entity->info |= 0x800000;
        }
        entity->info |= 0x40000;
    }
    else if (entity->def->eType == 7 && entity->def->parm != 0) {
        entity->info |= 0x40000;
    }
    else if (entity->def->eType == 10) {
        entity->doomRpg->render->mapSprites[(entity->info & 0xffffU) - 1].renderMode = 7;
        entity->info |= 0x40000;
    }
    else if (entity->def->eType != 12) {
    }
    else {
        if (entity->def->eSubType == 4) {

            entity->doomRpg->game->powerCouplingIndex++;
            if (entity->doomRpg->game->powerCouplingIndex == 2) {
                entity->info |= 0x400000;
            }
        }
        else {
            entity->info |= 0x40000;
        }
    }
}

boolean Entity_checkLineOfSight(Entity_t* entity, int srcX, int srcY, int destX, int destY, int i5)
{
    Game_t* game;
    Line_t* line;

    game = entity->doomRpg->game;

    Game_trace(game, srcX, srcY, destX, destY, entity, i5);
    if (game->numTraceEntities == 1 && game->traceEntities[0]->def->eType == 0) {
        line = &entity->doomRpg->render->lines[(game->traceEntities[0]->info & 65535) - 1];
        if (srcX == line->vert1.x || srcX == line->vert2.x || srcY == line->vert1.y || srcY == line->vert2.y) {
            return true;
        }
    }
    return (game->numTraceEntities == 0) ? true : false;

}

boolean Entity_renderOnlyStateMonsters(Entity_t* entity)
{
    Sprite_t* sprite;

    sprite = &entity->doomRpg->render->mapSprites[(entity->info & 65535) - 1];

    if (sprite->x == entity->monster->x && sprite->y == entity->monster->y) {
        return false;
    }

    if (sprite->x < entity->monster->x) {
        sprite->x += entity->doomRpg->doomCanvas->animPos;
        if (sprite->x > entity->monster->x) {
            sprite->x = entity->monster->x;
        }
    }
    else if (sprite->x > entity->monster->x) {
        sprite->x -= entity->doomRpg->doomCanvas->animPos;
        if (sprite->x < entity->monster->x) {
            sprite->x = entity->monster->x;
        }
    }

    if (sprite->y < entity->monster->y) {
        sprite->y += entity->doomRpg->doomCanvas->animPos;
        if (sprite->y > entity->monster->y) {
            sprite->y = entity->monster->y;
        }
    }
    else if (sprite->y > entity->monster->y) {
        sprite->y -= entity->doomRpg->doomCanvas->animPos;
        if (sprite->y < entity->monster->y) {
            sprite->y = entity->monster->y;
        }
    }

    Render_relinkSprite(entity->doomRpg->render, sprite);

    if (sprite->x == entity->monster->x && sprite->y == entity->monster->y) {
        Entity_aiMoveToGoal(entity);
    }

    //DoomCanvas.updateViewTrue();
    return true;
}

void Entity_aiThink(Entity_t* entity)
{
    Entity_t* next, *inactMonst, *ent;
    Sprite_t* sprite, *spr;

    sprite = &entity->doomRpg->render->mapSprites[(entity->info & 65535) - 1];

    int i = entity->doomRpg->doomCanvas->destX;
    int i2 = entity->doomRpg->doomCanvas->destY;
    int i3 = ((sprite->x - i) * (sprite->x - i)) + ((sprite->y - i2) * (sprite->y - i2));

    if (sprite->x == entity->monster->x && sprite->y == entity->monster->y)
    {
        entity->monster->target = NULL;
        if (entity->def->eSubType == 10) // Infernis / Archvile / Apollyon
        {
            entity->info &= -16777217;
            if (DoomRPG_randNextByte(&entity->doomRpg->random) < 84) {
                inactMonst = entity->doomRpg->game->inactiveMonsters;
                if (inactMonst ) {
                    do {
                        spr = &entity->doomRpg->render->mapSprites[(inactMonst->info & 65535) - 1];
                        int i4 = ((spr->x - sprite->x) * (spr->x - sprite->x)) + ((spr->y - sprite->y) * (spr->y - sprite->y));
                        if (i4 == 0 || i4 > 0x4000 || (spr->info & 0x1000000) == 0 || (inactMonst->info & 33554432) != 0 || Game_findMapEntityXYFlag(entity->doomRpg->game, spr->x, spr->y, 16263)) {
                            next = inactMonst->monster->nextOnList;
                            inactMonst = next;
                        }
                        else {
                            entity->info |= 0x1000000;
                            inactMonst->info |= 0x2000000;
                            entity->monster->target = inactMonst;
                            Entity_attack(entity);
                            return;
                        }
                    } while (next != entity->doomRpg->game->inactiveMonsters);
                }
            }
            entity->info ^= 0x800000;
        }

        byte b = (entity->info & 0x800000) != 0 ? monsterAttacks[(entity->def->eSubType * 2) + 1] : monsterAttacks[(entity->def->eSubType * 2) + 0];
        int i5 = (1 + entity->doomRpg->combat->weaponInfo[b].rangeMin) << 6;
        int i6 = i5 * i5;
        boolean z = false;
        if ((sprite->x == i || sprite->y == i2) && i3 <= i6) {
            Game_trace(entity->doomRpg->game, sprite->x, sprite->y, i, i2, entity, 22151);
            if (entity->doomRpg->game->numTraceEntities > 0) {
                ent = entity->doomRpg->game->traceEntities[0];
                if (ent->def->eType == 12) {
                    entity->monster->target = ent;
                    z = true;
                }
                else if (ent->def->eType == 8) {
                    z = true;
                }
            }
            else {
                z = true;
            }
        }

        int i7 = (1 + entity->doomRpg->combat->weaponInfo[b].rangeMin) / 2;
        if (!z || (i7 != 0 && DoomRPG_randNextByte(&entity->doomRpg->random) >= 217)) {
            int i8 = (i7 << 6) * (i7 << 6);
            int i9 = i + entity->doomRpg->doomCanvas->viewStepX;
            int i10 = i2 + entity->doomRpg->doomCanvas->viewStepY;
            if (i3 <= i8 && ((sprite->x == i || sprite->y == i2) && DoomRPG_randNextByte(&entity->doomRpg->random) < 38)) {
                if (sprite->x < i) {
                    i9 -= 64;
                }
                else if (sprite->x > i) {
                    i9 += 64;
                }
                if (sprite->y < i2) {
                    i10 -= 64;
                }
                else if (sprite->y > i2) {
                    i10 += 64;
                }
            }
            if (Entity_aiGoal_MOVE(entity, sprite->x, sprite->y, i9, i10)) {
                entity->monster->frameTime = 0;
                entity->doomRpg->game->interpolatingMonsters = true;
            }
            else if (z) {
                Entity_attack(entity);
            }
        }
        else {
            Entity_attack(entity);
        }
    }
    else {
        do {
            sprite->x = entity->monster->x;
            sprite->y = entity->monster->y;
        } while (Entity_aiMoveToGoal(entity));
    }
}

void Entity_setPosition(Entity_t* entity, int x, int y)
{
    Sprite_t* sprite;

    sprite = &entity->doomRpg->render->mapSprites[(entity->info & 65535) - 1];
    sprite->info = (sprite->info & -8192) | entity->def->tileIndex;
    entity->monster->x = x;
    entity->monster->y = y;

    sprite->x = entity->monster->x;
    sprite->y = entity->monster->y;
    Render_relinkSprite(entity->doomRpg->render, sprite);
    if (sprite->x == entity->monster->x && sprite->y == entity->monster->y) {
        Entity_aiMoveToGoal(entity);
    }
    //DoomCanvas.updateViewTrue();
}

void Entity_pain(Entity_t* entity, int i, int i2)
{
    int armor, health;

    armor = CombatEntity_getArmor(&entity->monster->ce);
    if (armor < i2) {
        i += i2 - armor;
        CombatEntity_setArmor(&entity->monster->ce, 0);
    }
    else {
        CombatEntity_setArmor(&entity->monster->ce, armor - i2);
    }

    health = CombatEntity_getHealth(&entity->monster->ce);
    CombatEntity_setHealth(&entity->monster->ce, health - i);
}

void Entity_reset(Entity_t* entity)
{
	entity->def = NULL;
	entity->prevOnTile = NULL;
	entity->nextOnTile = NULL;
	entity->linkIndex = 0;
	entity->info = 0;
	entity->monster = NULL;
}

void Entity_touched(Entity_t* entity)
{
    Player_t* player;
    Weapon_t* wpn;
    Hud_t* hud;
    char* msg;
    int sound;
    boolean z;

    player = entity->doomRpg->player;
    hud = entity->doomRpg->hud;

    sound = 5060;

    switch (entity->def->eType)
    {
        case 3: {
            switch (entity->def->eSubType)
            {
                case 22:
                case 23: {
                    if (!Player_addCredits(player, entity->def->parm)) {
                        Hud_addMessage(hud, "Credits at maximum");
                        return;
                    }
                    Player_updateDamageFaceTime(player);
                    break;
                }

                case 21: {
                    if (CombatEntity_getArmor(&player->ce) >= CombatEntity_getMaxArmor(&player->ce)) {
                        Hud_addMessage(hud, "Armor at maximum");
                        return;
                    }
                    Player_addArmor(player, entity->def->parm);
                    break;
                }

                case 20: {
                    if (CombatEntity_getHealth(&player->ce) >= CombatEntity_getMaxHealth(&player->ce)) {
                        Hud_addMessage(hud, "Health at maximum");
                        return;
                    }
                    Player_addHealth(player, entity->def->parm);
                    break;
                }

                case 24: {
                    player->keys |= 1 << entity->def->parm;
                    Player_updateDamageFaceTime(player);
                    break;
                }
            }

            Sound_playSound(entity->doomRpg->sound, sound, 0, 2);
            hud->gotFaceTime = entity->doomRpg->doomCanvas->time + 500;

            msg = Hud_getMessageBuffer(hud);
            SDL_snprintf(msg, MS_PER_CHAR, "Got %s", entity->def->name);
            Hud_finishMessageBuffer(hud);
            Game_remove(entity->doomRpg->game, entity);
            break;
        }

        case 4: {
            if (!Player_addItem(player, entity->def->eSubType, 1)) {
                msg = Hud_getMessageBuffer(hud);
                SDL_snprintf(msg, MS_PER_CHAR, "Can't hold more %ss", entity->def->name);
                Hud_finishMessageBuffer(hud);
                return;
            }
            Player_updateDamageFaceTime(player);

            if (entity->def->eSubType != 25 && entity->def->eSubType != 26) {
                sound = 5062;
            }
            Sound_playSound(entity->doomRpg->sound, sound, 0, 3);

            hud->gotFaceTime = entity->doomRpg->doomCanvas->time + 500;
            msg = Hud_getMessageBuffer(hud);
            SDL_snprintf(msg, MS_PER_CHAR, "Got %s", entity->def->name);
            Hud_finishMessageBuffer(hud);
            Game_remove(entity->doomRpg->game, entity);
            break;
        }

        case 6:
        case 16: {
            if (!Player_addAmmo(player, entity->def->eSubType, entity->def->parm)) {
                Hud_addMessage(hud, "Ammo at maximum");
                return;
            }
            Player_updateDamageFaceTime(player);
            hud->gotFaceTime = entity->doomRpg->doomCanvas->time + 500;
            msg = Hud_getMessageBuffer(hud);
            SDL_snprintf(msg, MS_PER_CHAR, "Got %s", entity->def->name);
            Hud_finishMessageBuffer(hud);

            Sound_playSound(entity->doomRpg->sound, sound, 0, 3);
            Game_remove(entity->doomRpg->game, entity);
            break;
        }

        case 5: {
            z = (player->weapons & (1 << entity->def->eSubType)) == 0 && (player->disabledWeapons & (1 << entity->def->eSubType)) == 0;

            if ((player->weapons & (1 << entity->def->eSubType)) == 0) {
                Player_selectWeapon(player, entity->def->eSubType);
            }

            Player_updateDamageFaceTime(player);
            player->weapons |= 1 << entity->def->eSubType;

            wpn = &entity->doomRpg->combat->weaponInfo[entity->def->eSubType];
            if (wpn->ammoUsage != 0) {
                Player_addAmmo(player, wpn->ammoType, entity->def->parm);
            }

            //Sound.playSound(1);
            hud->gotFaceTime = entity->doomRpg->doomCanvas->time + 500;
            msg = Hud_getMessageBuffer(hud);
            SDL_snprintf(msg, MS_PER_CHAR, "Got %s", entity->def->name);
            Hud_finishMessageBuffer(hud);
            Game_remove(entity->doomRpg->game, entity);

            if (z) {
                entity->doomRpg->game->tileEvent = 0;

                if (entity->def->eSubType == 0) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the Axe!|Zombies beware...", false);
                    sound = 5060;
                }
                else if (entity->def->eSubType == 1) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the Fire|Extinguisher! It|uses halon can-|isters to put out|fires.", false);
                    sound = 5060;
                }
                else if (entity->def->eSubType == 3) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the|Shotgun!", false);
                    sound = 5057;
                }
                else if (entity->def->eSubType == 4) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the|Chaingun! Precise|and deadly, but|it's an ammo hog.", false);
                    sound = 5057;
                }
                else if (entity->def->eSubType == 5) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the Super|Shotgun! Fierce!", false);
                    sound = 5057;
                }
                else if (entity->def->eSubType == 7) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the|Rocket Launcher!|w00t!", false);
                    sound = 5057;
                }
                else if (entity->def->eSubType == 6) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the|Plasma Gun!", false);
                    sound = 5057;
                }
                else if (entity->def->eSubType == 8) {
                    DoomCanvas_startDialog(entity->doomRpg->doomCanvas, "You got the BFG!|We could tell you|what BFG stands|for, but this is|a family game.", false);
                    sound = 5057;
                }

                Sound_playSound(entity->doomRpg->sound, sound, 0, 3);
            }
            else { // New lines code

                sound = 5057;
                if ((entity->def->eSubType == 0) || entity->def->eSubType == 1) {
                    sound = 5060;
                }

                Sound_playSound(entity->doomRpg->sound, sound, 0, 3);
            }
            break;
        }

        case 10: {
            Player_painEvent(player, NULL);
            Player_pain(player, 1, 2);
            Hud_addMessage(hud, "It burns!");
            break;
        }

        case 11: {
            Player_painEvent(player, NULL);
            Player_pain(player, 10, 10);
            Hud_addMessage(hud, "It really burns!!");
            break;
        }
    }
}

void Entity_trimCorpsePile(Entity_t* entity, int x, int y)
{
    Entity_t* inactive, *prev;
    Sprite_t* sprite;
    int i;

    inactive = entity->doomRpg->game->inactiveMonsters;
    if (inactive) {
        i = 0;
        for (prev = inactive->monster->prevOnList; prev != entity->doomRpg->game->inactiveMonsters; prev = prev->monster->prevOnList) {
            sprite = &entity->doomRpg->render->mapSprites[(prev->info & 65535) - 1];
            if (sprite->x == x && sprite->y == y && (sprite->info & 0x1000000) != 0) {
                i++;
                if (i > 1) {
                    sprite->info = (sprite->info & -16777217) | 0x10000;
                    prev->info |= 0x20000;
                }
            }
        }
    }
}