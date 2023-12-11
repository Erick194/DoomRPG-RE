
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Hud.h"
#include "Player.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Weapon.h"
#include "SDL_Video.h"

Hud_t* Hud_init(Hud_t* hud, DoomRPG_t* doomRpg)
{
	printf("Hud_init\n");

	if (hud == NULL)
	{
		hud = SDL_malloc(sizeof(Hud_t));
		if (hud == NULL) {
			return NULL;
		}
	}
    SDL_memset(hud, 0, sizeof(Hud_t));

	hud->msgCount = 0;
	hud->doomRpg= doomRpg;

	return hud;
}

void Hud_free(Hud_t* hud, boolean freePtr)
{
    DoomRPG_freeImage(hud->doomRpg, &hud->imgStatusBar);
    DoomRPG_freeImage(hud->doomRpg, &hud->imgStatusBarLarge);
    DoomRPG_freeImage(hud->doomRpg, &hud->imgHudFaces);
    DoomRPG_freeImage(hud->doomRpg, &hud->imgIconSheet);
    DoomRPG_freeImage(hud->doomRpg, &hud->imgAttArrow);
    DoomRPG_freeImage(hud->doomRpg, &hud->imgStatusArrow);

    if (freePtr) {
        SDL_free(hud);
    }
}

void Hud_addMessage(Hud_t* hud, char* str)
{
    Hud_addMessageForce(hud, str, false);
}

void Hud_addMessageForce(Hud_t* hud, char* str, boolean force)
{
    if (str) {
        if (force) {
            hud->msgCount = 0;
        }

        if ((hud->msgCount <= 0) || strcmp(str, hud->messages[hud->msgCount + -1])) {
            if (hud->msgCount == MAX_MESSAGES) {
                Hud_shiftMsgs(hud);
            }
            strncpy_s(hud->messages[hud->msgCount], MS_PER_CHAR, str, MS_PER_CHAR);
            hud->msgCount++;

            if (hud->msgCount == 1) {
                Hud_calcMsgTime(hud);
                if (force) {
                    hud->msgDuration *= 2;
                }
            }
        }
    }
}

void Hud_calcMsgTime(Hud_t* hud)
{
    int len;

    hud->msgTime = hud->doomRpg->doomCanvas->time;
    len = strlen(hud->messages[0]);

    if (len <= hud->msgMaxChars) {
        hud->msgDuration = MSG_DISPLAY_TIME;
    }
    else {
        hud->msgDuration = len * 100;
    }
}

void Hud_drawBarTiles(Hud_t* hud, int x, int y, int width, boolean isLargerStatusBar)
{
    Image_t* img;
    int height;

    if (isLargerStatusBar == false) {
        height = hud->imgStatusBar.height;
        img = &hud->imgStatusBar;
    }
    else {
        height = hud->imgStatusBarLarge.height;
        img = &hud->imgStatusBarLarge;
    }

    DoomCanvas_drawImageSpecial(hud->doomRpg->doomCanvas, img, 0, 0, width, height, 0, x, y, 0);
}

void Hud_drawBottomBar(Hud_t* hud)
{
    Image_t* img;
    DoomCanvas_t* doomCanvas;
    CombatEntity_t* ce;
    Combat_t* combat;
    int dispW, dispH, stbH;
    int cx, x, y, dy;
    int lx1, lx2;
    int health, maxHealth;
    int faceState, faceX;
    int weapon;
    char dir[2];

    ce = &hud->doomRpg->player->ce;
    doomCanvas = hud->doomRpg->doomCanvas;
    dispW = doomCanvas->displayRect.w;
    dispH = doomCanvas->displayRect.h;
    stbH = hud->statusBarHeight;
    cx = doomCanvas->SCR_CX;
    y = dispH - (stbH >> 1);

    if (hud->largeHud) {
        cx -= 88;
        y -= 8;
        x = 10;
    }
    else {
        cx -= 64;
        y -= 5;
        x = 7;
    }

    Hud_drawBarTiles(hud, 0, dispH - stbH, doomCanvas->displayRect.w, hud->largeHud);

    // draw vertical gray lines
    lx1 = hud->statusLine1Xpos + cx;
    lx2 = hud->statusLine2Xpos + cx;
    DoomRPG_setColor(hud->doomRpg, 0x313131);
    DoomRPG_drawLine(hud->doomRpg, lx1, dispH - stbH, lx1, dispH - 1);
    DoomRPG_drawLine(hud->doomRpg, lx2, dispH - stbH, lx2, dispH - 1);
    DoomRPG_setColor(hud->doomRpg, 0x808591);
    DoomRPG_drawLine(hud->doomRpg, lx1 + 1, dispH - stbH, lx1 + 1, dispH - 1);
    DoomRPG_drawLine(hud->doomRpg, lx2 + 1, dispH - stbH, lx2 + 1, dispH - 1);

    img = &hud->imgIconSheet;
    dy = dispH - (stbH >> 1);

    // draw health
    DoomCanvas_drawImageSpecial(doomCanvas, img, 0, 0, hud->iconSheetWidth, hud->iconSheetHeight, 0, hud->statusHealthXpos + cx, dy, 0x24);
    SDL_snprintf(hud->healthNum, 4, "%d", CombatEntity_getHealth(ce));
    DoomCanvas_drawFont(doomCanvas, hud->healthNum, hud->statusHealthXpos + cx + x * 2 + hud->iconSheetWidth + 1, y, 9, 0, 3, hud->largeHud);

    // draw armmor
    DoomCanvas_drawImageSpecial(doomCanvas, img, 0, hud->iconSheetHeight, hud->iconSheetWidth, hud->iconSheetHeight, 0, hud->statusArmorXpos + cx, dy, 0x24);
    SDL_snprintf(hud->armorNum, 4, "%d", CombatEntity_getArmor(ce));
    DoomCanvas_drawFont(doomCanvas, hud->armorNum, hud->statusArmorXpos + cx + x * 2 + hud->iconSheetWidth, y, 9, 0, 3, hud->largeHud);

    // draw face
    health = CombatEntity_getHealth(ce);

    if (health > 0 && doomCanvas->time - hud->gotFaceTime < 500) {
        faceState = 8;
    }
    else if (health <= 0 || doomCanvas->time >= hud->damageTime) {
        maxHealth = CombatEntity_getMaxHealth(ce);

        if (health <= maxHealth / 4) {
            faceState = 3;
        }
        else if (health <= maxHealth / 3) {
            faceState = 2;
        }
        else if (health <= maxHealth / 2) {
            faceState = 1;
        }
        else {
            faceState = 0;
        }
    }
    else {
        switch (hud->damageDir)
        {
        case 2:
            faceState = 7;
            break;
        case 1:
            faceState = 6;
            break;
        case 3:
            faceState = 5;
            break;
        default:
            faceState = 4;
            break;
        }
    }

    faceX = hud->statusHudFacesXpos + cx;
    DoomRPG_setColor(hud->doomRpg, 0x323232);
    DoomRPG_drawLine(hud->doomRpg, faceX - 1, dispH - hud->statusBarHeight, faceX - 1, dispH -1);
    DoomCanvas_drawImageSpecial(doomCanvas, &hud->imgHudFaces, 0, faceState * hud->hudFaceHeight, hud->hudFaceWidth, hud->hudFaceHeight, 0, faceX, dy, 0x24);
    DoomRPG_setColor(hud->doomRpg, 0x828282);
    DoomRPG_drawLine(hud->doomRpg, faceX + hud->hudFaceWidth, dispH - hud->statusBarHeight, faceX + hud->hudFaceWidth, dispH + -1);

    // draw weapon and ammo
    if (hud->doomRpg->player->weapons) {
        weapon = hud->doomRpg->player->weapon;

        if (weapon == 0) {
            DoomCanvas_drawImageSpecial(doomCanvas, img, 0, hud->iconSheetHeight << 1, hud->iconSheetWidth, hud->iconSheetHeight, 0, hud->statusAmmoXpos + cx, dy, 0x24);
            strncpy_s(hud->ammoNum, 3, "--", 4);
        }
        else {
            combat = hud->doomRpg->combat;
            DoomCanvas_drawImageSpecial(doomCanvas, img, 0, hud->iconSheetHeight * (combat->weaponInfo[weapon].ammoType + 3), hud->iconSheetWidth, hud->iconSheetHeight, 0, hud->statusAmmoXpos + cx, dy, 0x24);
            SDL_snprintf(hud->ammoNum, 3, "%d", hud->doomRpg->player->ammo[combat->weaponInfo[weapon].ammoType]);
        }

        DoomCanvas_drawFont(doomCanvas, hud->ammoNum, hud->statusAmmoXpos + cx + hud->iconSheetWidth + x * 2, y, 9, 0, 2, hud->largeHud);
    }

    // draw orientation text
    switch (doomCanvas->destAngle & 255) {
    case 0:
        strncpy_s(dir, sizeof(dir), "E", sizeof(dir));
        break;
    case 128:
        strncpy_s(dir, sizeof(dir), "W", sizeof(dir));
        break;
    case 192:
        strncpy_s(dir, sizeof(dir), "S", sizeof(dir));
        break;
    default:
        strncpy_s(dir, sizeof(dir), "N", sizeof(dir));
        break;
    }

    DoomCanvas_drawImage(doomCanvas, &hud->imgStatusArrow, hud->statusOrientationArrowXpos + cx, y - 3, 9);
    DoomCanvas_drawFont(doomCanvas, dir, hud->statusOrientationXpos + cx, y + 2, 9, 0, 1, hud->largeHud);
}

void Hud_drawEffects(Hud_t* hud)
{
    DoomRPG_t* doomRpg;
    DoomCanvas_t* doomCanvas;
    char str[8];
    int x, y, srcY;

    doomRpg = hud->doomRpg;
    doomCanvas = doomRpg->doomCanvas;

    if (doomCanvas->time < hud->damageTime) {
        if (hud->damageCount > 0) {
            DoomRPG_setColor(doomRpg, 0xBB0000);
        }
        else {
            DoomRPG_setColor(doomRpg, 0xFFFFFF);
        }

        DoomRPG_fillRect(doomRpg, 0, hud->statusTopBarHeight, 2, doomRpg->render->screenHeight + -1);
        DoomRPG_fillRect(doomRpg, doomCanvas->displayRect.w + -2, hud->statusTopBarHeight, 2, doomRpg->render->screenHeight + -1);
        DoomRPG_fillRect(doomRpg, 0, hud->statusTopBarHeight, doomCanvas->displayRect.w - 1, 2);
        DoomRPG_fillRect(doomRpg, 0, hud->statusTopBarHeight + doomRpg->render->screenHeight + -2, doomCanvas->displayRect.w - 1, 2);

        if (hud->damageDir && (hud->damageCount > 0)) {
            srcY = 0;
            if (hud->damageDir == 1) {
                x = doomCanvas->displayRect.w - 20;
                y = doomCanvas->displayRect.h >> 1;
                srcY = 36;
            }
            else if (hud->damageDir == 3) {
                x = 20;
                y = doomCanvas->displayRect.h >> 1;
                srcY = 18;
            }
            else {
                x = doomCanvas->displayRect.w >> 1;
                y = (doomCanvas->displayRect.h - hud->statusBarHeight) - 20;
            }

            DoomCanvas_drawImageSpecial(doomCanvas, &hud->imgAttArrow, 0, srcY, 18, 18, 0, x, y, 0x30);
        }
    }
    else if (hud->damageTime) {
        DoomCanvas_invalidateRectAndUpdateView(doomCanvas);
        hud->damageTime = 0;
    }

    if (doomRpg->player->berserkerTics) {
        // Bloqueo esta línea ya que la puse en otra función.
        // I block this line since I put it in another function.
        //{
        //    Render_setBerserkColor(doomRpg->render);
        //}

        SDL_snprintf(str, sizeof(str), "%d", hud->doomRpg->player->berserkerTics);
        doomCanvas = hud->doomRpg->doomCanvas;
        DoomCanvas_drawString1(doomCanvas, str, doomCanvas->displayRect.w - 2, hud->doomRpg->hud->statusTopBarHeight + 2, 9);
    }
}

void Hud_drawTopBar(Hud_t* hud)
{
    DoomCanvas_t* doomCanvas;
    char* text;
    int len;
    int time, w;
    int strBeg, strEnd;
    boolean updateTime;

    doomCanvas = hud->doomRpg->doomCanvas;
    Hud_drawBarTiles(hud, 0, 0, doomCanvas->displayRect.w, false);

    updateTime = true;
    // New Code Lines
    {
        // No actualiza los memsajes en los siguientes estados
        if (doomCanvas->state == ST_DYING) {
            updateTime = false;
        }
    }

    if (updateTime) {
        if ((hud->msgCount > 0) && ((doomCanvas->time - hud->msgTime) > (hud->msgDuration + 150))) {
            Hud_shiftMsgs(hud);
        }
    }

    strBeg = 0;
    if (hud->msgCount > 0)
    {
        text = hud->messages[0];
        if (updateTime) {
            if (hud->msgDuration < (hud->doomRpg->doomCanvas->time - hud->msgTime)) {
                return;
            }
        
            len = SDL_strlen(text) - hud->msgMaxChars;

            if (len > 0) {
                time = (doomCanvas->time - hud->msgTime);
                if (time > SCROLL_START_DELAY)
                {
                    strBeg = ((unsigned int)((time - SCROLL_START_DELAY) / 100));
                    if (strBeg > len - 1) {
                        strBeg = len - 1;
                    }
                }
            }
        }
    }
    else if (hud->statBarMessage != NULL) {
        text = hud->statBarMessage;
    }
    else if (hud->logMessage[0] != '\0') {
        text = hud->logMessage;
    }
    else if ((doomCanvas->state == ST_PLAYING) && (hud->doomRpg->player->facingEntity) && (hud->doomRpg->player->facingEntity->def->eType != 9)) {
        text = hud->doomRpg->player->facingEntity->def->name;
    }
    else {
        return;
    }  

    strEnd = SDL_strlen(text);
    w = doomCanvas->displayRect.w;
    if (((strEnd * 9) + 10) > w) {
        strEnd = ((unsigned int)((w - 1) / 7)) - 1;
    }

    DoomCanvas_drawFont(doomCanvas, text, 1, (hud->statusTopBarHeight >> 1) - 5, 0, strBeg, strEnd, false);
}

void Hud_finishMessageBufferForce(Hud_t* hud, boolean force)
{
    hud->msgCount++;
    if (hud->msgCount == 1) {
        Hud_calcMsgTime(hud);
        if (force) {
            hud->msgDuration *= 2;
        }
    }
}

void Hud_finishMessageBuffer(Hud_t* hud)
{
    Hud_finishMessageBufferForce(hud, false);
}

char* Hud_getMessageBufferForce(Hud_t* hud, boolean force)
{
    if (force) {
        hud->msgCount = 0;
    }
    if (hud->msgCount == MAX_MESSAGES) {
        Hud_shiftMsgs(hud);
    }
    hud->messages[hud->msgCount][0] = '\0';
    return hud->messages[hud->msgCount];
}

char* Hud_getMessageBuffer(Hud_t* hud)
{
    return Hud_getMessageBufferForce(hud, false);
}

void Hud_shiftMsgs(Hud_t* hud)
{
    int i;

    for (i = 0; i < (hud->msgCount - 1); i++) {
        strncpy_s(hud->messages[i], MS_PER_CHAR, hud->messages[i + 1], MS_PER_CHAR);
    }

    hud->msgCount--;
    hud->messages[hud->msgCount][0] = '\0';
    if (hud->msgCount > 0) {
        Hud_calcMsgTime(hud);
    }
}

void Hud_startup(Hud_t* hud, boolean largeStatus)
{
	DoomRPG_t* doomRpg;
    int height = 0;

	doomRpg = hud->doomRpg;

	hud->msgMaxChars = (doomRpg->doomCanvas->displayRect.w - 4) / 7;
	hud->statusTopBarHeight = 20;
    DoomRPG_createImage(doomRpg, "bar_lg.bmp", false, &hud->imgStatusBarLarge);
    DoomRPG_createImage(doomRpg, "k.bmp", false, &hud->imgStatusBar);
    DoomRPG_createImage(doomRpg, "n.bmp", true, &hud->imgAttArrow);
    DoomRPG_createImage(doomRpg, "o.bmp", true, &hud->imgStatusArrow);

    if (largeStatus == 0) {
        hud->largeHud = false;
        DoomRPG_createImage(doomRpg, "l.bmp", false, &hud->imgHudFaces);
        DoomRPG_createImage(doomRpg, "m.bmp", true, &hud->imgIconSheet);
        hud->statusArmorXpos = 35;
        hud->statusHudFacesXpos = 66;
        hud->statusAmmoXpos = 85;
        hud->statusOrientationXpos = 126;
        hud->statusOrientationArrowXpos = 128;
        hud->statusLine1Xpos = 33;
        hud->statusLine2Xpos = 155;
        hud->statusHealthXpos = 2;
        height = hud->imgStatusBar.height;
    }
    else {
        hud->largeHud = true;
        DoomRPG_createImage(doomRpg, "larger HUD faces.bmp", false, &hud->imgHudFaces);
        DoomRPG_createImage(doomRpg, "larger_HUD_icon_sheet.bmp", true, &hud->imgIconSheet);
        hud->statusArmorXpos = 46;
        hud->statusHudFacesXpos = 88;
        hud->statusAmmoXpos = 115;
        hud->statusOrientationXpos = 171;
        hud->statusOrientationArrowXpos = 172;
        hud->statusLine1Xpos = 44;
        hud->statusLine2Xpos = 158;
        hud->statusHealthXpos = 2;
        height = hud->imgStatusBarLarge.height;
    }
    hud->statusBarHeight = height;

    hud->hudFaceWidth = hud->imgHudFaces.width;
    hud->hudFaceHeight = (hud->imgHudFaces.height / 9);
    hud->iconSheetWidth = hud->imgIconSheet.width;
    hud->iconSheetHeight = (hud->imgIconSheet.height / 9);
}