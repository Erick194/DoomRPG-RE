
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Hud.h"
#include "Render.h"
#include "Game.h"
#include "Player.h"
#include "ParticleSystem.h"
#include "Sound.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Combat.h"
#include "SDL_Video.h"

static char processing[] = "Processing...";
static char justAMoment[] = "(Just a moment!)";

#define MOVEFORWARD	1
#define MOVEBACK	2
#define TURNLEFT	3
#define TURNRIGHT	4
#define MENUOPEN	5
#define SELECT		6
#define AUTOMAP		7
//8
#define MOVELEFT		9
#define MOVERIGHT		10
#define PREVWEAPON		11
#define NEXTWEAPON		12
//13
#define PASSTURN		14
//15
#define MENU_UP			16
#define MENU_DOWN		17
#define MENU_PAGE_UP	18
#define MENU_PAGE_DOWN	19
#define MENU_SELECT		20
#define MENU_OPEN		21


#define NUM_KEYPADS	10
byte keys_numKeyPadActions[NUM_KEYPADS] = {MENUOPEN   ,
							MOVELEFT   , MOVEFORWARD , MOVERIGHT,
							TURNLEFT   , SELECT      , TURNRIGHT,
							NEXTWEAPON , MOVEBACK    , PASSTURN};

#define NUM_CODES 34
int keys_codeActions[NUM_CODES] = {
	AVK_CLR,		15,
	AVK_SOFT2,		AUTOMAP,
	AVK_SOFT1,		MENUOPEN,
	// New items Only Port
	AVK_STAR,		PREVWEAPON,
	AVK_POUND,		AUTOMAP,
	AVK_NEXTWEAPON, NEXTWEAPON,
	AVK_PREVWEAPON, PREVWEAPON,
	AVK_AUTOMAP,	AUTOMAP,
	AVK_UP,			MOVEFORWARD,
	AVK_DOWN,		MOVEBACK,
	AVK_LEFT,		TURNLEFT,
	AVK_RIGHT,		TURNRIGHT,
	AVK_MOVELEFT,	MOVELEFT,
	AVK_MOVERIGHT,	MOVERIGHT,
	AVK_SELECT,		SELECT,
	AVK_MENUOPEN,	MENUOPEN,
	AVK_PASSTURN,	PASSTURN
};

DoomCanvas_t* DoomCanvas_init(DoomCanvas_t* doomCanvas, DoomRPG_t* doomRpg) // 0x12A20
{
	printf("DoomCanvas_init\n");

	if (doomCanvas == NULL)
	{
		doomCanvas = SDL_malloc(sizeof(DoomCanvas_t));
		if (doomCanvas == NULL) {
			return NULL;
		}
	}
	SDL_memset(doomCanvas, 0, sizeof(DoomCanvas_t));

	doomCanvas->doomRpg = doomRpg;
	doomCanvas->skipShakeX = 0;
	doomCanvas->insufficientSpace = 0;
	doomCanvas->creditsText = NULL;
	doomCanvas->castEntity = NULL;
	doomCanvas->castSeq = 0;
	doomCanvas->field_0xdcc = 0;
	doomCanvas->castEntityX = 0;
	doomCanvas->castEntityY = 0;
	doomCanvas->numEvents = 0;
	doomCanvas->legalsNextImage = 0;
	doomCanvas->legalsTime = 0;
	doomCanvas->oldState = -1;
	doomCanvas->imgFont.imgBitmap = NULL;
	doomCanvas->imgLargerFont.imgBitmap = NULL;
	doomCanvas->imgLegals.imgBitmap = NULL;
	doomCanvas->imgMapCursor.imgBitmap = NULL;
	doomCanvas->imgSpaceBG.imgBitmap = NULL;
	doomCanvas->imgLinesLayer.imgBitmap = NULL;
	doomCanvas->imgPlanetLayer.imgBitmap = NULL;
	doomCanvas->imgSpaceship.imgBitmap = NULL;
	doomCanvas->storyText1[0] = NULL;
	doomCanvas->storyText1[1] = NULL;
	doomCanvas->storyText2 = NULL;
	doomCanvas->softKeyRight[0] = '\0';
	doomCanvas->softKeyLeft[0] = '\0';
	doomCanvas->f438d = false;
	doomCanvas->automapDrawn = false;
	doomCanvas->loadType = 0;
	doomCanvas->saveType = 0;
	doomCanvas->slowBlit = 0;
	doomCanvas->unloadMedia = false;
	doomCanvas->st_count = 0;
	doomCanvas->clipRect.x = 0;
	doomCanvas->clipRect.y = 0;
	doomCanvas->clipRect.w = sdlVideo.rendererW;
	doomCanvas->clipRect.h = sdlVideo.rendererH;
	//printf("clipRect W(%d) H(%d)\n", doomCanvas->clipRect.w, doomCanvas->clipRect.h);

	//From J2ME Version
	doomCanvas->lastPacifierUpdate = 0;
	doomCanvas->fillRectIndex = 0;

	//New
	doomCanvas->fontColor = 0xffffffff;
	doomCanvas->mouseSensitivity = 50;
	doomCanvas->mouseYMove = true;
	doomCanvas->sndPriority = false;
	doomCanvas->vibrateEnabled = true;
	doomCanvas->renderFloorCeilingTextures = true;

	return doomCanvas;
}

void DoomCanvas_free(DoomCanvas_t* doomCanvas, boolean freePtr)
{
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgFont);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgLargerFont);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgLegals);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgMapCursor);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceBG);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgLinesLayer);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgPlanetLayer);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceship);

	SDL_free(doomCanvas->storyText1[0]);
	SDL_free(doomCanvas->storyText1[1]);
	SDL_free(doomCanvas->storyText2);
	SDL_free(doomCanvas->creditsText);

	if (freePtr) {
		SDL_free(doomCanvas);
	}
}

void DoomCanvas_updatePlayerDoors(DoomCanvas_t* doomCanvas, Line_t* line)
{
	int indx;
	if (doomCanvas->animFrameCount) {
		do {
		} while (DoomCanvas_updatePlayerAnimDoors(doomCanvas));
	}
	indx = doomCanvas->openDoorsCount;
	doomCanvas->openDoorsCount = indx + 1;
	doomCanvas->openDoors[indx] = line;
}

void DoomCanvas_attemptMove(DoomCanvas_t* doomCanvas, int x, int y)
{
	if (doomCanvas->renderOnly) {
		doomCanvas->destX = x;
		doomCanvas->destY = y;
		return;
	}

	Game_trace(doomCanvas->game, doomCanvas->viewX, doomCanvas->viewY, x, y, NULL, 62087);

	if (doomCanvas->player->noclip || doomCanvas->game->numTraceEntities == 0) {
		doomCanvas->f438d = true;
		Game_eventFlagsForMovement(doomCanvas->game, doomCanvas->viewX, doomCanvas->viewY, x, y);
		doomCanvas->abortMove = false;
		Game_executeTile(doomCanvas->game, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->game->eventFlags[0] | 1024);
		if (doomCanvas->player->noclip || !doomCanvas->abortMove) {
			doomCanvas->destX = x;
			doomCanvas->destY = y;
			DoomCanvas_updateViewTrue(doomCanvas);
		}
	}
	else if (doomCanvas->state == ST_AUTOMAP) {
		Game_advanceTurn(doomCanvas->game);
	}
}

void DoomCanvas_automapState(DoomCanvas_t* doomCanvas)
{
	Game_t* game;
	int waitTime;
	if (doomCanvas->automapDrawn || doomCanvas->openDoorsCount != 0) {
		//DoomCanvas_drawAutomap(doomCanvas, false); // Old
		DoomCanvas_drawAutomap(doomCanvas, true);
	}
	else {
		doomCanvas->automapDrawn = true;
		doomCanvas->render->skipStretch = true;
		DoomCanvas_updateView(doomCanvas);
		doomCanvas->render->skipStretch = false;

		if (doomCanvas->state == ST_AUTOMAP) {
			DoomCanvas_drawAutomap(doomCanvas, true);
		}
	}

	waitTime = doomCanvas->game->waitTime;
	if (waitTime && (doomCanvas->time >= waitTime)) {
		doomCanvas->game->waitTime = 0;
		game = doomCanvas->game;
		Game_runEvent(game, game->tileEvent, game->tileEventIndex + 1, game->tileEventFlags);
	}
}

void DoomCanvas_setupmenu(DoomCanvas_t* doomCanvas, boolean notdrawLoading)
{
	if (notdrawLoading == false) {
		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		DoomCanvas_drawString1(doomCanvas, "Loading...", doomCanvas->SCR_CX, doomCanvas->SCR_CY + -0x18, 0x11);
		DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}

	Player_reset(doomCanvas->player);
	Render_freeRuntime(doomCanvas->render);
	Game_unloadMapData(doomCanvas->game);

	if (notdrawLoading == false) {
		Sound_playSound(doomCanvas->doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
		MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_MAIN);
	}
	else {
		if (doomCanvas->skipIntro) {
			DoomCanvas_loadMap(doomCanvas, doomCanvas->startupMap);
		}
		else {
			MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_ENABLE_SOUNDS);
		}
	}
}

int DoomCanvas_getOverall(DoomCanvas_t* doomCanvas)
{
	int i, level, totalDeaths;
	Player_t* player;

	player = doomCanvas->player;
	i = 0;
	for (level = 0; level < 11; level++) {
		if ((player->foundSecretsLevels & (1 << level)) != 0) {
			i += 2;
		}
		if ((player->killedMonstersLevels & (1 << level)) != 0) {
			i += 2;
		}
		if ((player->completedLevels & (1 << level)) != 0) {
			i += 2;
		}
	}

	totalDeaths = 5 - player->totalDeaths;
	if (totalDeaths > 0) {
		i += 5 * totalDeaths;
	}

	return i;
}

void DoomCanvas_captureDogState(DoomCanvas_t* doomCanvas)
{
	Entity_t* entity;
	Player_t* player;

	player = doomCanvas->player;
	entity = player->dogFamiliar;

	if (doomCanvas->time >= doomCanvas->captureTime) {
		switch (doomCanvas->captureState) {
		case 0:
			doomCanvas->render->mapSprites[(entity->info & 65535) - 1].info |= 0x10000;
			entity->info |= 0x20000;
			Game_deactivate(doomCanvas->game, entity);
			Game_unlinkEntity(doomCanvas->game, entity);
			DoomCanvas_checkFacingEntity(doomCanvas);
			DoomCanvas_updateViewTrue(doomCanvas);
			doomCanvas->captureState = 1;
			doomCanvas->captureTime = doomCanvas->time + 100;
			break;
		case 1:
			if (entity->def->parm == 294) {
				player->weapons |= 512;
				Player_selectWeapon(player, 9);
			}
			else if (entity->def->parm == 467) {
				player->weapons |= 1024;
				Player_selectWeapon(player, 10);
			}
			else {
				player->weapons |= 2048;
				Player_selectWeapon(player, 11);
			}
			player->ammo[5] = 0;
			Player_addAmmo(doomCanvas->player, 5, CombatEntity_getHealth(&entity->monster->ce) * 5);
			Sound_playSound(doomCanvas->doomRpg->sound, 5134, 0, 3);
			doomCanvas->captureState = 2;
			doomCanvas->captureTime = doomCanvas->time + 500;
			break;
		case 2:
			SDL_snprintf(doomCanvas->printMsg, sizeof(doomCanvas->printMsg), "You captured|the %s!|", entity->def->name);
			doomCanvas->game->tileEvent = 0;
			DoomCanvas_startDialog(doomCanvas, doomCanvas->printMsg, false);
			doomCanvas->captureState = 0;
			doomCanvas->captureTime = 0;
			DoomCanvas_updateViewTrue(doomCanvas);
			Game_advanceTurn(doomCanvas->game);
			break;
		}
	}
}

void DoomCanvas_closeDialog(DoomCanvas_t* doomCanvas)
{
	Sound_stopSounds(doomCanvas->doomRpg->sound);
	doomCanvas->dialogBuffer[0] = '\0';
	DoomCanvas_setState(doomCanvas, ST_PLAYING);
	doomCanvas->hud->isUpdate = true;
	doomCanvas->staleView = true;
	doomCanvas->isUpdateView = true;
}

void DoomCanvas_combatState(DoomCanvas_t* doomCanvas)
{
	ParticleSystem_t* particleSystem;
	Combat_t* combat;

	particleSystem = doomCanvas->doomRpg->particleSystem;
	combat = doomCanvas->doomRpg->combat;

	boolean z = true;
	if (doomCanvas->isUpdateView) {
		DoomCanvas_renderScene(doomCanvas, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle);
		doomCanvas->staleView = true;
		doomCanvas->isUpdateView = true;
		DoomCanvas_drawRGB(doomCanvas);
	}
	else if (particleSystem->nodeListA.next == &particleSystem->nodeListA || 
		(doomCanvas->slowBlit && doomCanvas->time - particleSystem->endTime < 250)) {
		z = false;
	}
	else {
		//invalidateRect();
		doomCanvas->staleView = true;
		doomCanvas->isUpdateView = true;
		DoomCanvas_drawRGB(doomCanvas);
	}

	// En el código original esta función está en la función "Hud_drawEffects", pero decidí moverla aquí, 
	// esto evita que se superponga a otros objetos dibujados previamente.
	// 
	// In the original code this function is in the "Hud_drawEffects" function, but I decided to move it here, 
	// this prevents it from overlapping other previously drawn objects
	{
		if (doomCanvas->doomRpg->player->berserkerTics) {
			Render_setBerserkColor(doomCanvas->doomRpg->render);
		}
	}

	ParticleSystem_render(doomCanvas->doomRpg->particleSystem, z);

	if (combat->curAttacker == NULL || doomCanvas->time >= combat->f339c) {
		if (doomCanvas->combatDone) {
			if (particleSystem->nodeListA.next == &particleSystem->nodeListA) {
				if (combat->curAttacker == NULL) {
					Game_advanceTurn(doomCanvas->doomRpg->game);
				}
				DoomCanvas_setState(doomCanvas, ST_PLAYING);
			}
		}
		else if (!Combat_runFrame(combat)) {
			if (combat->curAttacker == NULL) {
				Game_touchTile(doomCanvas->doomRpg->game, doomCanvas->destX, doomCanvas->destY, false);
				doomCanvas->combatDone = true;
			}
			else if (combat->curAttacker->monster->nextAttacker) {
				Combat_performAttack(combat,
					combat->curAttacker->monster->nextAttacker,
					combat->curAttacker->monster->nextAttacker->monster->target);
			}
			else {
				doomCanvas->doomRpg->game->combatMonsters = NULL;
				Game_endMonstersTurn(doomCanvas->doomRpg->game);
				DoomCanvas_drawSoftKeys(doomCanvas->doomRpg->doomCanvas, "Menu", "Map");
				doomCanvas->combatDone = true;
				doomCanvas->isUpdateView = true;
			}
		}
		doomCanvas->doomRpg->hud->isUpdate = true;
	}

	Hud_drawEffects(doomCanvas->doomRpg->hud);
	Hud_drawTopBar(doomCanvas->doomRpg->hud);
	Hud_drawBottomBar(doomCanvas->doomRpg->hud);
}

void DoomCanvas_dialogState(DoomCanvas_t* doomCanvas)
{
	char text[8];
	int strBeg, strEnd, strNxt, strlen;
	int i, posY;
	boolean strblink;

	doomCanvas->isUpdateView = true;
	DoomCanvas_updateView(doomCanvas);
	DoomCanvas_drawRGB(doomCanvas);

	// En el código original esta función está en la función "Hud_drawEffects", pero decidí moverla aquí, 
	// esto evita que se superponga a otros objetos dibujados previamente.
	// 
	// In the original code this function is in the "Hud_drawEffects" function, but I decided to move it here, 
	// this prevents it from overlapping other previously drawn objects
	{
		if (doomCanvas->doomRpg->player->berserkerTics) {
			Render_setBerserkColor(doomCanvas->doomRpg->render);
		}
	}

	ParticleSystem_render(doomCanvas->particleSystem, true);
	Hud_drawTopBar(doomCanvas->hud);
	Hud_drawBottomBar(doomCanvas->hud);
	Hud_drawEffects(doomCanvas->hud);

	if (doomCanvas->doomRpg->player->berserkerTics) {
		// Bloqueo esta linea ya que hace que el color del berserk sea el doble de intenso, esto no sucede en la version J2ME
		// I block this line as it makes the berserk color twice as intense, this does not happen in the J2ME version
		//{
		// Render_setBerserkColor(doomCanvas->render);
		//}

		SDL_snprintf(text, 8, "%d", doomCanvas->doomRpg->player->berserkerTics);
		DoomCanvas_drawString1(doomCanvas, text, doomCanvas->displayRect.w - 2, doomCanvas->doomRpg->hud->statusTopBarHeight + 2, 9);
	}

	if (doomCanvas->dialogBuffer[0] == '\0') {
		return;
	}

	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, doomCanvas->SCR_CX - 64, doomCanvas->displayRect.h - 54, 128, 54);

	if (doomCanvas->player->facingEntity != NULL && 
		doomCanvas->player->facingEntity->def->eType == 7 && 
		doomCanvas->player->facingEntity->def->eSubType == 5) {
		DoomRPG_setColor(doomCanvas->doomRpg, 0x3FBF00);
	}
	else {
		DoomRPG_setColor(doomCanvas->doomRpg, 0xffffff);
	}

	if (doomCanvas->displayRect.w <= 128) {

		DoomRPG_drawLine(doomCanvas->doomRpg, 
			doomCanvas->SCR_CX - 64, doomCanvas->displayRect.h - 55, 
			doomCanvas->SCR_CX + 63, doomCanvas->displayRect.h - 55);

		DoomRPG_drawLine(doomCanvas->doomRpg,
			doomCanvas->SCR_CX - 64, doomCanvas->displayRect.h - 1,
			doomCanvas->SCR_CX + 63, doomCanvas->displayRect.h - 1);
	}
	else {
		DoomRPG_drawRect(doomCanvas->doomRpg, doomCanvas->SCR_CX - 65, doomCanvas->displayRect.h - 55, 129, 54);
	}

	if (doomCanvas->state == ST_DIALOGPASSWORD) {

		strblink = ((DoomRPG_GetUpTimeMS() / 300) & 1) == 1;

		strlen = SDL_strlen(doomCanvas->passCode);
		if (strlen < (int)SDL_strlen(doomCanvas->strPassCode))
		{
			if (strblink && doomCanvas->strPassCode[strlen] == ' ') {
				doomCanvas->strPassCode[strlen] = '_';
			}
			else if (!strblink && doomCanvas->strPassCode[strlen] == '_') {
				doomCanvas->strPassCode[strlen] = ' ';
			}
		}
	}

	posY = doomCanvas->displayRect.h - 52;
	for (i = 0; i < 4 && doomCanvas->currentDialogLine + i < doomCanvas->numDialogLines; ++i) {
		strBeg = doomCanvas->dialogIndexes[((doomCanvas->currentDialogLine + i) * 2) + 0];
		strNxt = doomCanvas->dialogIndexes[((doomCanvas->currentDialogLine + i) * 2) + 1];
		strEnd = 0;
		if (i == doomCanvas->dialogTypeLineIdx) {
			strEnd = (doomCanvas->time - doomCanvas->dialogLineStartTime) / 25;
			if (strEnd >= strNxt) {
				strEnd = strNxt;
				doomCanvas->dialogTypeLineIdx++;
				doomCanvas->dialogLineStartTime = doomCanvas->time;
			}
		}
		else if (i < doomCanvas->dialogTypeLineIdx) {
			strEnd = strNxt;
		}
		DoomCanvas_drawFont(doomCanvas, doomCanvas->dialogBuffer, doomCanvas->SCR_CX - 64, posY, 0, strBeg, strEnd, 0);

		posY += 12;
	}

	if (doomCanvas->state == ST_DIALOGPASSWORD && doomCanvas->dialogTypeLineIdx == doomCanvas->numDialogLines) {
		DoomCanvas_drawString2(doomCanvas, 
			doomCanvas->strPassCode, 
			(doomCanvas->SCR_CX - 64) + ((doomCanvas->dialogIndexes[((doomCanvas->numDialogLines - 1) * 2) + 1] + 1) * 7),
			//doomCanvas->SCR_CX + (doomCanvas->dialogIndexes[(doomCanvas->numDialogLines * 2) - 1] * 7) - 57,
			posY - 12, 0, -1);
	}
	if (doomCanvas->numDialogLines > 4) {
		if (doomCanvas->currentDialogLine + 4 == doomCanvas->numDialogLines) {
			DoomCanvas_drawScrollBar(doomCanvas, doomCanvas->displayRect.h - 54, 53, doomCanvas->currentDialogLine, doomCanvas->numDialogLines, doomCanvas->numDialogLines);
		}
		else {
			DoomCanvas_drawScrollBar(doomCanvas, doomCanvas->displayRect.h - 54, 53, doomCanvas->currentDialogLine, doomCanvas->currentDialogLine + 4, doomCanvas->numDialogLines + 4);
		}
	}
}

void DoomCanvas_disposeEpilogue(DoomCanvas_t* doomCanvas)
{
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceBG);
	Sound_stopSounds(doomCanvas->doomRpg->sound);
	doomCanvas->printMsg[0] = '\0';
	DoomCanvas_setState(doomCanvas, ST_LOADING);
}

void DoomCanvas_disposeIntro(DoomCanvas_t* doomCanvas)
{
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceBG);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgLinesLayer);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgPlanetLayer);
	DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceship);
	SDL_free(doomCanvas->storyText1[0]);
	doomCanvas->storyText1[0] = NULL;
	SDL_free(doomCanvas->storyText1[1]);
	doomCanvas->storyText1[1] = NULL;
	SDL_free(doomCanvas->storyText2);
	doomCanvas->storyText2 = NULL;

	DoomRPG_setClipFalse(doomCanvas->doomRpg);
	DoomCanvas_loadMap(doomCanvas, doomCanvas->startupMap);
}

void DoomCanvas_drawAutomap(DoomCanvas_t* doomCanvas, boolean z)
{
	Entity_t* entity;
	Sprite_t* sprite;
	Line_t* line;
	int cw, ch;

	cw = doomCanvas->clipRect.w;
	ch = doomCanvas->clipRect.h;
	if (cw < ch) {
		ch = cw + ((cw >> 0x1f) >> 0x1b);
	}
	else {
		ch = ch + ((ch >> 0x1f) >> 0x1b);
	}

	int i;
	int i2;
	int i3;
	int i4;
	int i5 = ch / 32;
	int i6 = doomCanvas->SCR_CX - (i5 * 16);
	int i7 = doomCanvas->SCR_CY - (i5 * 16);
	int i8 = 0x400000 / (i5 << 8);

	if (z)
	{
		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		DoomRPG_setClipTrue(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);


		int i9 = 0;
		for (int i10 = 0; i10 < 32; i10++) {
			for (int i11 = 0; i11 < 32; i11++) {
				byte b = doomCanvas->render->mapFlags[i9 + i11];
				boolean z2 = false;
				if ((b & BIT_AM_VISITED) != 0 && (b & BIT_AM_WALL) == 0) {
					z2 = true;
					if ((b & BIT_AM_ENTRANCE) != 0) {
						DoomRPG_setColor(doomCanvas->doomRpg, 0xFFAA00);
					}
					else {
						DoomRPG_setColor(doomCanvas->doomRpg, 0x660000);
					}
					DoomRPG_fillRect(doomCanvas->doomRpg, i6 + (i5 * i11), i7 + (i5 * i10), i5, i5);
				}
				for (entity = doomCanvas->game->entityDb[i9 + i11]; entity != NULL; entity = entity->nextOnTile) {
					if (z2 && entity->def->eType == 2) {
						DoomRPG_setColor(doomCanvas->doomRpg, 0x33BB00);
						DoomRPG_fillRect(doomCanvas->doomRpg, i6 + (i5 * i11) + (i5 / 2), i7 + (i5 * i10) + (i5 / 2), i5 / 2, i5 / 2);
					}
				}
			}
			i9 += 32;
		}

		int i12 = 0;
		for (int i13 = 0; i13 < 32; i13++) {
			for (int i14 = 0; i14 < 32; i14++) {
				for (entity = doomCanvas->game->entityDb[i12 + i14]; entity != NULL; entity = entity->nextOnTile) {
					if (entity->def->eType == 14 || entity->def->eType == 15) {
						sprite = &doomCanvas->render->mapSprites[(entity->info & 65535) - 1];
						if ((sprite->info & 268435456) != 0) {
							if ((sprite->info & 262144) != 0) {
								DoomRPG_setColor(doomCanvas->doomRpg, 0xCC0000);
							}
							else {
								DoomRPG_setColor(doomCanvas->doomRpg, 0x880000);
							}
							if ((sprite->info & 524288) != 0) {
								i4 = ((sprite->x - 32) << 16) / i8;
								i2 = ((sprite->x + 32) << 16) / i8;
								int i15 = (sprite->y << 16) / i8;
								i = i15;
								i3 = i15;
							}
							else if ((sprite->info & 1048576) != 0) {
								i4 = ((sprite->x - 32) << 16) / i8;
								i2 = ((sprite->x + 32) << 16) / i8;
								int i16 = (sprite->y << 16) / i8;
								i = i16;
								i3 = i16;
							}
							else if ((sprite->info & 4194304) != 0) {
								int i17 = (sprite->x << 16) / i8;
								i2 = i17;
								i4 = i17;
								i3 = ((sprite->y - 32) << 16) / i8;
								i = ((sprite->y + 32) << 16) / i8;
							}
							else {
								int i18 = (sprite->x << 16) / i8;
								i2 = i18;
								i4 = i18;
								i3 = ((sprite->y - 32) << 16) / i8;
								i = ((sprite->y + 32) << 16) / i8;
							}

							DoomRPG_drawLine(doomCanvas->doomRpg, i6 + ((i4 + 128) >> 8), i7 + ((i3 + 128) >> 8), i6 + ((i2 + 128) >> 8), i7 + ((i + 128) >> 8));
						}
					}
				}
			}
			i12 += 32;
		}

		DoomRPG_setColor(doomCanvas->doomRpg, 0xCC0000);
		for (int i19 = 0; i19 < doomCanvas->render->linesLength; i19++) {
			line = &doomCanvas->render->lines[i19];
			if ((line->flags & 128) != 0) {
				int i20 = (line->vert1.x << 16) / i8;
				int i21 = (line->vert1.y << 16) / i8;
				int i22 = (line->vert2.x << 16) / i8;
				int i23 = (line->vert2.y << 16) / i8;
				if ((line->flags & 4) != 0) {
					DoomRPG_setColor(doomCanvas->doomRpg, 0xCC9900);
				}
				else {
					DoomRPG_setColor(doomCanvas->doomRpg, 0xCC0000);
				}
				DoomRPG_drawLine(doomCanvas->doomRpg, i6 + ((i20 + 128) >> 8), i7 + ((i21 + 128) >> 8), i6 + ((i22 + 128) >> 8), i7 + ((i23 + 128) >> 8));
			}
		}
	}

	if (doomCanvas->time > doomCanvas->automapBlinkTime) {
		doomCanvas->automapBlinkTime = doomCanvas->time + 333;
		doomCanvas->automapBlinkState = !doomCanvas->automapBlinkState;
	}

	int i24 = 0;
	switch (doomCanvas->destAngle & 255) {
	case 0:
		i24 = 2;
		break;
	case 128:
		i24 = 3;
		break;
	case 192:
		i24 = 1;
		break;
	}

	int i25 = i24 * 6;
	if (doomCanvas->automapBlinkState) {
		i25 += 24;
	}

	int px = i6 + ((i5 * (doomCanvas->viewX - 32)) / 64) + (i5 / 2);
	int py = i7 + ((i5 * (doomCanvas->viewY - 32)) / 64) + (i5 / 2);

	// Port: Corrige la posicion de la imagen ya que se sobrepone sobre las lineas del mapa
	{
		px += 1;
		py += 1;
	}

	if (py < (doomCanvas->displayRect.y + doomCanvas->displayRect.h)) {
		DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgMapCursor, 0, i25, 6, 6, 0, px, py, 0x30);
	}
}

static char creditsText[] = "     CREDITS\n\nProduced by\nJOHN CARMACK\nKATHERINE A. KANG\n\nProgramming by\nJOHN CARMACK\nJAH RAPHAEL\nHEATH MORRISON\n\nDesign by\nMATTHEW C. ROSS\nDAVID WHITLARK\n\nArt by\nDAVID WHITLARK\nMATTHEW C. ROSS\n\nSupport by\nBRETT ESTABROOK\n\n\n\n\n\nThanks for\nplaying.\n\n\n\nPress OK to\ncontinue.";
void DoomCanvas_drawCredits(DoomCanvas_t* doomCanvas)
{
	int len;
	if (doomCanvas->creditsTextTime == -1) {
		len = SDL_strlen(creditsText);
		doomCanvas->creditsText = SDL_malloc(len + 1);
		strcpy_s(doomCanvas->creditsText, len + 1, creditsText);
		doomCanvas->creditsTextTime = doomCanvas->time;

		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}

	if (doomCanvas->time - doomCanvas->creditsTextTime >= 24750) {
		doomCanvas->creditsTextTime = doomCanvas->time - 24750;
	}

	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
	//DoomRPG_flushGraphics(doomCanvas->doomRpg);

	// New line From J2ME Version
	DoomRPG_setClipTrue(doomCanvas->doomRpg, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 128, 128);

	DoomCanvas_scrollSpaceBG(doomCanvas);
	
	DoomCanvas_drawString2(doomCanvas, doomCanvas->creditsText, doomCanvas->SCR_CX - 64, (doomCanvas->SCR_CY + 64) - ((doomCanvas->time - doomCanvas->creditsTextTime) / 62), 0, -1);
	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, -doomCanvas->displayRect.y, doomCanvas->displayRect.w, doomCanvas->displayRect.y + doomCanvas->SCR_CY - 64);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, doomCanvas->SCR_CY + 64, doomCanvas->displayRect.w, (doomCanvas->clipRect.h - doomCanvas->SCR_CY) - 64);

}

void DoomCanvas_castState(DoomCanvas_t* doomCanvas)
{
	GameSprite_t* gSprite;
	Sprite_t* sprite;
	boolean spectreBlephegor;
	int x, y, w, h;
	int snd, field;
	int texture, spriteId;

	if (doomCanvas->castSeq == -1) {
		if (doomCanvas->castTime == 0) {
			if (doomCanvas->doomRpg->sound->soundEnabled != 0) {
				doomCanvas->castTime = doomCanvas->time + 2500;
				Sound_playSound(doomCanvas->doomRpg->sound, 5043, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
			}
			else {
				doomCanvas->castSeq = 0;
			}
		}
		else {
			if (doomCanvas->time < doomCanvas->castTime) {
				DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
				DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);

				w = (13 - ((doomCanvas->castTime - doomCanvas->time) / 200)) * (doomCanvas->displayRect.w / 13);
				h = (13 - ((doomCanvas->castTime - doomCanvas->time) / 200)) * (doomCanvas->displayRect.h / 13);
				x = (doomCanvas->displayRect.w - w + (doomCanvas->displayRect.w < w)) / 2;
				y = (doomCanvas->displayRect.h - h + (doomCanvas->displayRect.h < h)) / 2;

				DoomRPG_setColor(doomCanvas->doomRpg, 0xffffff);
				DoomRPG_fillRect(doomCanvas->doomRpg, x, y, w, h);;
			}
			else {
				doomCanvas->castSeq = 0;
				doomCanvas->castTime = 0;
			}
		}
	}
	else if (doomCanvas->castSeq == -2) {
		Render_render(doomCanvas->render, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewZ, doomCanvas->viewAngle);
		if (doomCanvas->time >= doomCanvas->castTime + 1500) {
			Render_setup(doomCanvas->doomRpg->render, &doomCanvas->screenRect);
			Render_freeRuntime(doomCanvas->doomRpg->render);
			Game_unloadMapData(doomCanvas->doomRpg->game);
			DoomCanvas_setState(doomCanvas, ST_CREDITS);
			DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
			DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);

		}
		else {
			int fade = 255 - ((65280 * (((doomCanvas->time - doomCanvas->castTime) << 16) / 384000)) >> 16);
			Render_fadeScreen(doomCanvas->render, fade & 0xff);
			DoomCanvas_drawRGB(doomCanvas);
		}
	}
	else {
		Render_render(doomCanvas->render, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewZ, doomCanvas->viewAngle);
		DoomCanvas_drawRGB(doomCanvas);

		if (doomCanvas->castEntity == NULL) {
			doomCanvas->castEntityX++;

			if (doomCanvas->castEntityX > 30) {
				doomCanvas->castEntityX = 1;
				doomCanvas->castEntityY++;
			}

			doomCanvas->castEntity = Game_findMapEntityXYFlag(doomCanvas->game, doomCanvas->castEntityX << 6, doomCanvas->castEntityY << 6, 0xFFFF);

			if (doomCanvas->castEntity == NULL) {
				doomCanvas->castSeq = -2;
				doomCanvas->castTime = doomCanvas->time;
				return;
			}

			doomCanvas->castSeq = 0;
			doomCanvas->castTime = doomCanvas->time;
			doomCanvas->castEntityEndAttackTime = 0;
			doomCanvas->castEntityLoopFrames = Combat_getMonsterWeaponInfo(doomCanvas->doomRpg->combat, doomCanvas->castEntity->def->eSubType, 0);
			doomCanvas->castEntityBegAttackTime = Combat_getMonsterWeaponInfo(doomCanvas->doomRpg->combat, doomCanvas->castEntity->def->eSubType, 1);
			doomCanvas->field_0xdd8 = 0;
			doomCanvas->field_0xddc = 0;
			Game_gsprite_allocAnim(doomCanvas->game, 2, doomCanvas->viewX, doomCanvas->viewY - 64);
		}

		sprite = &doomCanvas->render->mapSprites[(doomCanvas->castEntity->info & 0xFFFF) - 1];

		spectreBlephegor = false;
		if ((doomCanvas->castEntity->def->eSubType == 5) && (doomCanvas->castEntity->def->parm == 640)) {
			spectreBlephegor = true;
		}

		if ((doomCanvas->castEntity) && ((sprite->info & 0x10000) == 0)) {
			DoomCanvas_drawString2(doomCanvas, doomCanvas->castEntity->def->name, doomCanvas->displayRect.w / 2, doomCanvas->displayRect.h, 18, -1);
		}

		switch (doomCanvas->castSeq) {
			case 0:
				if (doomCanvas->time > doomCanvas->castTime + 200) {
					sprite->x = doomCanvas->viewX;
					sprite->y = doomCanvas->viewY + -0x40;

					if (!spectreBlephegor) {
						sprite->renderMode = 4;
					}

					Render_relinkSprite(doomCanvas->render, sprite);

					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 1:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					if (!spectreBlephegor) {
						sprite->renderMode = 5;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 2:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					if (!spectreBlephegor) {
						sprite->renderMode = 6;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 3:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					if (!spectreBlephegor) {
						sprite->renderMode = 0;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 4:
				if (doomCanvas->castEntityEndAttackTime == 0) {
					doomCanvas->field_0xdd8 = 0;
					doomCanvas->castEntityLoopFrames = Combat_getMonsterWeaponInfo(doomCanvas->doomRpg->combat, doomCanvas->castEntity->def->eSubType, 0);
					doomCanvas->castEntityBegAttackTime = Combat_getMonsterWeaponInfo(doomCanvas->doomRpg->combat, doomCanvas->castEntity->def->eSubType, 1) * 10;
					doomCanvas->castEntityEndAttackTime = doomCanvas->time + doomCanvas->castEntityBegAttackTime;
					if (doomCanvas->field_0xddc != 0) {
						doomCanvas->castEntityEndAttackTime = doomCanvas->castEntityEndAttackTime + 500;
					}
				}

				if (doomCanvas->time > doomCanvas->castEntityEndAttackTime)
				{
					doomCanvas->field_0xdd8 = !doomCanvas->field_0xdd8;

					if (doomCanvas->field_0xdd8) {

						if (doomCanvas->field_0xddc == false) {
							sprite->info = (sprite->info & 0xffffe1ff) | 0x200;
							field = 4;
						}
						else {
							sprite->info = (sprite->info & 0xffffe1ff) | 0xa00;
							field = 5;
						}

						// Attack Sound
						snd = EntityMonster_getSoundID(doomCanvas->castEntity->monster, field);
						if (snd != 0) {
							Sound_playSound(doomCanvas->doomRpg->sound, snd, 0, 2);
						}

						doomCanvas->castEntityEndAttackTime = doomCanvas->castEntityBegAttackTime + doomCanvas->time;
					}
					else {
						doomCanvas->castEntityLoopFrames--;
						sprite->info &= 0xffffe1ff;
						doomCanvas->castEntityEndAttackTime = (doomCanvas->castEntityBegAttackTime / 2) + doomCanvas->time;
					}
				}

				if (doomCanvas->castEntityLoopFrames == 0) {
					if (doomCanvas->field_0xddc == false) {
						
						texture  = doomCanvas->render->mediaSpriteIds[sprite->info & 0x1ff] * 2;
						if (doomCanvas->render->mediaBitShapeOffsets[texture + 2] != doomCanvas->render->mediaBitShapeOffsets[texture + 10]) {
							doomCanvas->castEntityEndAttackTime = 0;
							doomCanvas->field_0xddc = true;
						}
					}

					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 5:
				if (doomCanvas->time > doomCanvas->castTime + 500)
				{
					// Death Sound
					snd = EntityMonster_getSoundRnd(doomCanvas->castEntity->monster, 8);
					if (snd != 0) {
						Sound_playSound(doomCanvas->doomRpg->sound, snd, 0, 2);
					}

					spriteId = sprite->info & 0x1ff;

					if ((unsigned int)(spriteId - 41) <= 2) {
						gSprite = Game_gsprite_allocAnim(doomCanvas->game, 1, doomCanvas->viewX, doomCanvas->viewY - 64);
						gSprite->flags |= 4;
						sprite->info |= 0x10000;
					}
					else if (spriteId == 53) {

						gSprite = Game_gsprite_allocAnim(doomCanvas->game, 1, doomCanvas->viewX - 16, doomCanvas->viewY - 64);
						gSprite->flags |= 4;
						gSprite->time += DoomRPG_randNextByte(&doomCanvas->doomRpg->random) / 100;

						gSprite = Game_gsprite_allocAnim(doomCanvas->game, 1, doomCanvas->viewX, doomCanvas->viewY - 64);
						gSprite->flags |= 4;

						gSprite = Game_gsprite_allocAnim(doomCanvas->game, 1, doomCanvas->viewX + 16, doomCanvas->viewY - 64);
						gSprite->flags |= 4;
						gSprite->time += DoomRPG_randNextByte(&doomCanvas->doomRpg->random) / 100;
						sprite->info |= 0x10000;
					}
					else if (spriteId == 54) {
						sprite->info &= 0xffffe1ff;
						doomCanvas->castSeq++;
					}
					else {
						sprite->info = (sprite->info & 0xffffe1ff) | 0x800;
					}

					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 6:
				if (doomCanvas->time > doomCanvas->castTime + 250)
				{
					sprite->info = (sprite->info & 0xffffe1ff) | 0x400;
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 7:
				if (doomCanvas->time > doomCanvas->castTime + 400) {
					if (!spectreBlephegor) {
						sprite->renderMode = 6;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 8:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					if (!spectreBlephegor) {
						sprite->renderMode = 5;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 9:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					if (!spectreBlephegor) {
						sprite->renderMode = 4;
					}
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 10:
				if (doomCanvas->time > doomCanvas->castTime + 100) {
					sprite->info |= 0x10000;
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
				}
				break;

			case 11:
				if (doomCanvas->time > doomCanvas->castTime + 1000)
				{
					
					sprite->y = doomCanvas->viewY - 640;
					Render_relinkSprite(doomCanvas->render, sprite);
					doomCanvas->castSeq++;
					doomCanvas->castTime = doomCanvas->time;
					doomCanvas->castEntity = NULL;
				}
				break;
		}
	}
}

void DoomCanvas_drawEpilogue(DoomCanvas_t* doomCanvas)
{
	if (doomCanvas->epilogueTextTime == -1) {
		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);

		doomCanvas->epilogueTextTime = doomCanvas->time;
	}

	// New line From J2ME Version
	DoomRPG_setClipTrue(doomCanvas->doomRpg, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 128, 128);

	if (doomCanvas->epilogueTextPage == 2) {
		DoomCanvas_disposeEpilogue(doomCanvas);
	}
	else {
		DoomCanvas_scrollSpaceBG(doomCanvas);
		DoomCanvas_drawString2(doomCanvas, doomCanvas->epilogueText[doomCanvas->epilogueTextPage], 
			doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0, (doomCanvas->showTextDone != 0) ? -1 : doomCanvas->epilogueTextTime);

		if (doomCanvas->epilogueTextPage < 1) {
			DoomCanvas_drawImage(doomCanvas, &doomCanvas->menuSystem->imgHand, (doomCanvas->SCR_CX + 36) - 4, doomCanvas->SCR_CY + 64, 10);
			DoomCanvas_drawString1(doomCanvas, "More", (doomCanvas->SCR_CX + 64) - 4, doomCanvas->SCR_CY + 64, 10);
		}
		else {
			DoomCanvas_drawImage(doomCanvas, &doomCanvas->menuSystem->imgHand, (doomCanvas->SCR_CX + 8) - 4, doomCanvas->SCR_CY + 64, 10);
			DoomCanvas_drawString1(doomCanvas, "Continue", (doomCanvas->SCR_CX + 64) - 4, doomCanvas->SCR_CY + 64, 10);
		}

		if ((doomCanvas->time - doomCanvas->epilogueTextTime) > (((int) SDL_strlen(doomCanvas->epilogueText[doomCanvas->epilogueTextPage]) * 25))) {
			doomCanvas->showTextDone = true;
		}
	}
}

void DoomCanvas_drawImage(DoomCanvas_t* doomcanvas, Image_t* img, int x, int y, int flags)
{
	DoomCanvas_drawImageSpecial(doomcanvas, img, 0, 0, 0, 0, 0, x, y, flags);
}

void DoomCanvas_drawStory(DoomCanvas_t* doomCanvas)
{
	char **text;
	int iVar1;

	if (doomCanvas->storyAnimTime == -1) {
		doomCanvas->storyAnimTime = DoomRPG_GetUpTimeMS();
	}
	if (doomCanvas->storyTextTime == -1) {
		doomCanvas->storyTextTime = DoomRPG_GetUpTimeMS();
	}

	if (doomCanvas->time >= doomCanvas->storyTextTime && doomCanvas->time >= doomCanvas->storyAnimTime) {

		int i = (doomCanvas->time - doomCanvas->storyAnimTime);
		int i2 = (doomCanvas->time - doomCanvas->storyTextTime);

		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);

		// New line From J2ME Version
		DoomRPG_setClipTrue(doomCanvas->doomRpg, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 128, 128);

		if (doomCanvas->storyPage == 0 || doomCanvas->storyPage == 2) {

			if (doomCanvas->storyPage == 0){
				text = doomCanvas->storyText1;
				iVar1 = 2;
			}
			else {
				text = &doomCanvas->storyText2;
				iVar1 = 1;
			}

			if (iVar1 <= doomCanvas->storyTextPage) {
				DoomCanvas_changeStoryPage(doomCanvas);
				return;
			}

			DoomCanvas_scrollSpaceBG(doomCanvas);

			if (doomCanvas->showTextDone) {
				DoomCanvas_drawString2(doomCanvas, text[doomCanvas->storyTextPage], doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0, -1);
			}
			else {
				DoomCanvas_drawString2(doomCanvas, text[doomCanvas->storyTextPage], doomCanvas->SCR_CX -64,doomCanvas->SCR_CY - 64, 0, doomCanvas->storyTextTime);
			}

			if (doomCanvas->storyTextPage < iVar1 - 1) {
				DoomCanvas_drawImage(doomCanvas, &doomCanvas->doomRpg->menuSystem->imgHand, (doomCanvas->SCR_CX + 36) - 4, (doomCanvas->SCR_CY + 64) - 2, 10);
				DoomCanvas_drawString1(doomCanvas, "More", (doomCanvas->SCR_CX + 64) - 4, (doomCanvas->SCR_CY + 64), 10);
			}
			else {
				DoomCanvas_drawImage(doomCanvas, &doomCanvas->doomRpg->menuSystem->imgHand, (doomCanvas->SCR_CX + 8) - 4, (doomCanvas->SCR_CY + 64) - 2, 10);
				DoomCanvas_drawString1(doomCanvas, "Continue", (doomCanvas->SCR_CX + 64) - 4, (doomCanvas->SCR_CY + 64), 10);
			}

			if (i2 > ((int)SDL_strlen(text[doomCanvas->storyTextPage]) * 25)) {
				doomCanvas->showTextDone = true;
			}
		}
		else {

			if (i > 10000) {
				DoomCanvas_changeStoryPage(doomCanvas);
			}

			int i3 = i / 457;
			int i4 = i / 157;
			int i5 = (doomCanvas->SCR_CX - 64) + (i / 142);
			int i6 = (doomCanvas->SCR_CY + 22) + (i / -333);

			// BREW Code
			// {
			//DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgSpaceBG, i3, 0, 128, 128, 0, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
			//DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgLinesLayer, i4, 0, 128, 128, 0, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
			// }
			
			// J2ME Code
			//{
			DoomCanvas_drawImage(doomCanvas, &doomCanvas->imgSpaceBG, (doomCanvas->SCR_CX - 64) - i3, doomCanvas->SCR_CY - 64, 0);
			DoomCanvas_drawImage(doomCanvas, &doomCanvas->imgLinesLayer, (doomCanvas->SCR_CX - 64) - i4, doomCanvas->SCR_CY - 64, 0);
			//}

			DoomCanvas_drawImage(doomCanvas, &doomCanvas->imgPlanetLayer, doomCanvas->SCR_CX - 64,doomCanvas->SCR_CY - 64, 0);
			DoomCanvas_drawImage(doomCanvas, &doomCanvas->imgSpaceship, i5, i6, 0);
			if ((i / 500) % 2 == 0)
			{
				DoomRPG_setClipTrue(doomCanvas->doomRpg, (doomCanvas->SCR_CX - 63), (doomCanvas->SCR_CY - 63), 126, 126);

				DoomRPG_setColor(doomCanvas->doomRpg, 0xBB0000);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5, i6 - 1, i5 + 9, i6 - 1);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5 + 4, 0, i5 + 4, i6 - 1);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5, i6 + 9, i5 + 9, i6 + 9);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5 + 4, i6 + 9, i5 + 4, doomCanvas->displayRect.h);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5 - 1, i6, i5 - 1, i6 + 9);
				DoomRPG_drawLine(doomCanvas->doomRpg, 0, i6 + 4, i5 - 1, i6 + 4);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5 + 9, i6, i5 + 9, i6 + 9);
				DoomRPG_drawLine(doomCanvas->doomRpg, i5 + 9, i6 + 4, doomCanvas->displayRect.w, i6 + 4);
			}
		}
	}
}

void DoomCanvas_drawRGB(DoomCanvas_t* doomCanvas)
{
	// Port:
	// aplicar esta función antes de actualizar el framebuffer
	// apply this function before updating the framebuffer
	//if (doomCanvas->doomRpg->player->berserkerTics) {
		//Render_setBerserkColor(doomCanvas->doomRpg->render);
	//}

	if (doomCanvas->time < doomCanvas->shaketime) {
		if (!doomCanvas->skipShakeX) {
			doomCanvas->shakeX = ((DoomRPG_randNextByte(&doomCanvas->doomRpg->random)) % (doomCanvas->shakeVal * 2)) - doomCanvas->shakeVal;
		}
		doomCanvas->shakeY = ((DoomRPG_randNextByte(&doomCanvas->doomRpg->random)) % (doomCanvas->shakeVal * 2)) - doomCanvas->shakeVal;
	}
	else if (!(doomCanvas->shakeX == 0 && doomCanvas->shakeY == 0)) {
		doomCanvas->shakeY = 0;
		doomCanvas->shakeX = 0;
	}

	if (doomCanvas->state == ST_CAST) {
		doomCanvas->shakeY = 0;
		doomCanvas->shakeX = 0;
	}

	SDL_Rect renderQuad, clip;

	clip.x = doomCanvas->render->screenX;
	clip.y = doomCanvas->render->screenY;
	clip.w = doomCanvas->render->screenWidth;
	clip.h = doomCanvas->render->screenHeight;

	renderQuad.x = doomCanvas->render->screenX;
	renderQuad.y = doomCanvas->render->screenY;
	renderQuad.w = sdlVideo.rendererW;
	renderQuad.h = sdlVideo.rendererH;
	if (clip.w <= renderQuad.w) {
		renderQuad.w = clip.w;
	}
	if (clip.h <= renderQuad.h) {
		renderQuad.h = clip.h;
	}

	SDL_UpdateTexture(doomCanvas->render->piDIB, NULL, doomCanvas->render->framebuffer, sdlVideo.rendererW * 2);
	SDL_RenderCopy(sdlVideo.renderer, doomCanvas->render->piDIB, &clip, &renderQuad);

	//DoomRPG_flushGraphics(doomCanvas->doomRpg);
}

void DoomCanvas_drawImageSpecial(DoomCanvas_t* doomCanvas, Image_t* img, int xSrc, int ySrc, int width, int height, int param_7, int xDst, int yDst, int flags)
{
	SDL_Rect renderQuad, clip;

	if (width == 0) {
		width = img->width;
	}
	if (height == 0) {
		height = img->height;
	}

	if ((flags & 16) == 0) {
		if ((flags & 8) != 0) {
			xDst = xDst - width;
		}
	}
	else {
		xDst = xDst - ((unsigned int)width >> 1);
	}
	if ((flags & 32) == 0) {
		if ((flags & 2) != 0) {
			yDst = yDst - height;
		}
	}
	else {
		yDst = yDst - ((unsigned int)height >> 1);
	}

	renderQuad.w = img->width;
	renderQuad.h = img->height;

	if (img->imgBitmap) {

		int uVar3 = width;
		int iVar4 = xDst;

		do {
			do {
				clip.x = xSrc;
				clip.y = ySrc;
				clip.w = uVar3;
				clip.h = height;

				renderQuad.x = doomCanvas->displayRect.x + iVar4;
				renderQuad.y = doomCanvas->displayRect.y + yDst;
				if (clip.w <= renderQuad.w) {
					renderQuad.w = clip.w;
				}
				if (clip.h <= renderQuad.h) {
					renderQuad.h = clip.h;
				}

				if (flags & 64) { // New flag Scale
					// calculate new x and y
					renderQuad.x = (renderQuad.x + renderQuad.w / 2) - (((renderQuad.w / 2) * 3) / 2);
					renderQuad.y = (renderQuad.y + renderQuad.h / 2) - (((renderQuad.h / 2) * 3) / 2);

					renderQuad.w = (renderQuad.w * 3) / 2;
					renderQuad.h = (renderQuad.h * 3) / 2;
				}

				SDL_RenderCopy(sdlVideo.renderer, img->imgBitmap, &clip, &renderQuad);

				uVar3 = uVar3 - img->width;
				iVar4 = img->width + iVar4;
			} while (0 < (int)uVar3);
			height = height - img->height;
			yDst = img->height + yDst;
			uVar3 = width;
			iVar4 = xDst;
		} while (0 < height);
	}
}

void DoomCanvas_drawScrollBar(DoomCanvas_t* doomCanvas, int y, int totalHeight, int i3, int i4, int i5)
{
	int i6 = i4 - i3;
	if (i6 != 0) {
		int barHeight = totalHeight / (((i5 + i6) - 1) / i6);
		int offSetY = (((i3 << 16) / ((i5 - 1) << 8)) * ((totalHeight - barHeight) << 8)) >> 16;
		if (i4 == i5) {
			offSetY = (totalHeight - 14) - barHeight;
		}
		int barOffset_y = offSetY + 7;

		DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->doomRpg->menuSystem->imgArrowUpDown, 0, 0, 7, 7, 0, doomCanvas->SCR_CX + 64, y, 9);
		DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->doomRpg->menuSystem->imgArrowUpDown, 0, 7, 7, 7, 0, doomCanvas->SCR_CX + 64, y + totalHeight, 10);

		DoomRPG_setColor(doomCanvas->doomRpg, 0x888888);
		DoomRPG_fillRect(doomCanvas->doomRpg, (doomCanvas->SCR_CX + 64) - 7, y + 7, 7, totalHeight - 14);

		DoomRPG_setColor(doomCanvas->doomRpg, 0xDDDDDD);
		DoomRPG_fillRect(doomCanvas->doomRpg, (doomCanvas->SCR_CX + 64) - 7, y + barOffset_y, 7, barHeight);

		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_drawRect(doomCanvas->doomRpg, (doomCanvas->SCR_CX + 64) - 7, y + barOffset_y, 6, barHeight - 1);
		DoomRPG_drawRect(doomCanvas->doomRpg, (doomCanvas->SCR_CX + 64) - 7, y, 6, totalHeight - 1);
	}

}

void DoomCanvas_drawSoftKeys(DoomCanvas_t* doomCanvas, char* softKeyLeft, char* softKeyRight)
{
	int x, y, x1, y1;

	if (doomCanvas->displaySoftKeys) {

		doomCanvas->restoreSoftKeys = true; // New Line Code

		x1 = x = -doomCanvas->displayRect.x;
		y1 = y = doomCanvas->softKeyY - (int)(doomCanvas->displayRect).y;

		Hud_drawBarTiles(doomCanvas->doomRpg->hud, x, y, doomCanvas->clipRect.w, false);
		if (softKeyLeft == NULL) {
			doomCanvas->softKeyLeft[0] = '\0';
		}
		else {
			if (doomCanvas->softKeyLeft != softKeyLeft) {
				strcpy_s(doomCanvas->softKeyLeft, sizeof(doomCanvas->softKeyLeft), softKeyLeft);
			}

			DoomRPG_setColor(doomCanvas->doomRpg, 0x313131);
			DoomRPG_drawLine(doomCanvas->doomRpg, x1 + 52, y1, x1 + 52, y1 + 19);
			DoomRPG_setColor(doomCanvas->doomRpg, 0x808591);
			DoomRPG_drawLine(doomCanvas->doomRpg, x1 + 53, y1, x1 + 53, y1 + 19);
			DoomCanvas_drawString1(doomCanvas, softKeyLeft, x + 26, y + 5, 17);
		}

		if (softKeyRight == NULL) {
			doomCanvas->softKeyRight[0] = '\0';
		}
		else {
			if (doomCanvas->softKeyRight != softKeyRight) {
				strcpy_s(doomCanvas->softKeyRight, sizeof(doomCanvas->softKeyRight), softKeyRight);
			}
			DoomRPG_setColor(doomCanvas->doomRpg, 0x313131);
			x1 += (doomCanvas->clipRect.w - 52);
			DoomRPG_drawLine(doomCanvas->doomRpg, x1, y1, x1, y1 + 19);
			DoomRPG_setColor(doomCanvas->doomRpg, 0x808591);
			DoomRPG_drawLine(doomCanvas->doomRpg, x1 + 1, y1, x1 + 1, y1 + 19);
			DoomCanvas_drawString1(doomCanvas, softKeyRight, (doomCanvas->clipRect.w + x) - 28, y + 5, 17);
		}
	}
}

void DoomCanvas_scrollSpaceBG(DoomCanvas_t* doomCanvas)
{
	int i = (int)(-((doomCanvas->time / 157) % 192));
	int i2 = i;
	int i3 = i + 192;
	if (i2 <= -192) {
		i2 = i2 + 384;
	}

	// New Code
	//{
	DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgSpaceBG, 0, 0, 0, 0, 0, -i2 + doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
	DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgSpaceBG, 0, 0, 0, 0, 0, -i3 + doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
	//}
	
	// Original Code
	//DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgSpaceBG, i2, 0, 128, 128, 0, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
	//DoomCanvas_drawImageSpecial(doomCanvas, &doomCanvas->imgSpaceBG, i3, 0, 128, 128, 0, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
}

void DoomCanvas_drawString1(DoomCanvas_t* doomCanvas, char* text, int x, int y, int flags)
{
	DoomCanvas_drawFont(doomCanvas, text, x, y, flags, 0, -1, 0);
}

void DoomCanvas_drawString2(DoomCanvas_t* doomCanvas, char* text, int x, int y, int flags, int param_6)
{
	DoomCanvas_drawFont(doomCanvas, text, x, y, flags, 0, (doomCanvas->time - param_6) / 25, 0);
}

void DoomCanvas_drawFont(DoomCanvas_t* doomCanvas, char* text, int x, int y, int flags, int strBeg, int strEnd, boolean isLargerFont)
{
	Image_t* imgFont;
	int iVar4, width, height, len, xpos, i;
	unsigned int c;

	if (strEnd == 0) {
		return;
	}

	if (isLargerFont == 0) {
		imgFont = &doomCanvas->imgFont;
		iVar4 = 7;
		width = 9;
		height = 12;
	}
	else {
		imgFont = &doomCanvas->imgLargerFont;
		iVar4 = 10;
		width = 13;
		height = 17;
	}

	// New, only port
	{
		byte r, g, b;

		r = (doomCanvas->fontColor & 0x00FF0000) >> 16;
		g = (doomCanvas->fontColor & 0x0000FF00) >> 8;
		b = (doomCanvas->fontColor & 0x000000FF);

		SDL_SetTextureColorMod(imgFont->imgBitmap, r, g, b);
	}

	len = SDL_strlen(text) - strBeg;

	if ((len > strEnd) && (strEnd >= 0)) {
		len = strEnd;
	}

	if (flags & 8) {
		x -= len * iVar4;
	}
	else if (flags & 16) {
		x -= (iVar4 * ((len << FRACBITS) / 512)) >> 8;
	}

	if (flags & 2) {
		y -= height;
	}
	else if (flags & 32) {
		y -= (height >> 1);
	}

	len += strBeg;
	xpos = x;
	for (i = strBeg; i < len; ++i)
	{
		c = text[i];
		if (c == 10) {
			y += height;
			xpos = x;
		}
		else {
			if (c != ' ') {
				DoomCanvas_drawImageSpecial(doomCanvas,
					imgFont,
					width * ((c - 33) & 0xf),
					height * ((unsigned int)((c - 33) << 24) >> 28),
					width,
					height,
					0,
					xpos,
					y,
					0);
			}
			xpos += iVar4;
		}
	}
}


void DoomCanvas_dyingState(DoomCanvas_t* doomCanvas)
{
	// New Code Lines
	{
		Hud_drawTopBar(doomCanvas->hud);
		Hud_drawBottomBar(doomCanvas->hud);
	}

	if (doomCanvas->time < doomCanvas->deathTime + 750) {
		doomCanvas->viewZ = 36 - ((8192 * ((((int)(doomCanvas->time - doomCanvas->deathTime)) << 16) / 192000)) >> 16);
		DoomCanvas_updateViewTrue(doomCanvas);
		DoomCanvas_updateView(doomCanvas);
		DoomCanvas_drawRGB(doomCanvas);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
	else if (doomCanvas->time < doomCanvas->deathTime + 3000) {
		int fade = 255 - ((65280 * ((((int)((doomCanvas->time - doomCanvas->deathTime) - 750)) << 16) / 576000)) >> 16);
		Render_fadeScreen(doomCanvas->render, fade & 0xff);
		DoomCanvas_drawRGB(doomCanvas);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
	else {
		Game_saveConfig(doomCanvas->doomRpg->game, 0);
		MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_INGAME_DEAD);
	}
}

void DoomCanvas_sorryState(DoomCanvas_t* doomCanvas)
{
	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
	
	DoomRPG_setClipTrue(doomCanvas->doomRpg, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 128, 128);
	DoomCanvas_scrollSpaceBG(doomCanvas);

	DoomCanvas_drawString2(doomCanvas, "Sorry!", doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0, -1);
	DoomCanvas_drawString2(doomCanvas, doomCanvas->printMsg, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 40, 0, -1);
}

void DoomCanvas_finishMovement(DoomCanvas_t* doomCanvas)
{
	Game_executeTile(doomCanvas->game, doomCanvas->destX, doomCanvas->destY, 
		doomCanvas->game->eventFlags[1] | DoomCanvas_flagForFacingDir(doomCanvas) | 0x400);

	DoomCanvas_checkFacingEntity(doomCanvas);
	DoomCanvas_uncoverAutomap(doomCanvas);
	Game_touchTile(doomCanvas->game, doomCanvas->destX, doomCanvas->destY, true);
	Game_advanceTurn(doomCanvas->game);
}

void DoomCanvas_finishRotation(DoomCanvas_t* doomCanvas)
{
	doomCanvas->viewSin = doomCanvas->render->sinTable[doomCanvas->destAngle & 255];
	doomCanvas->viewCos = doomCanvas->render->sinTable[(doomCanvas->destAngle + 64) & 255];
	doomCanvas->viewStepX = (doomCanvas->viewCos * 64) >> 16;
	doomCanvas->viewStepY = ((-doomCanvas->viewSin) * 64) >> 16;
	Game_executeTile(doomCanvas->game, doomCanvas->destX, doomCanvas->destY, DoomCanvas_flagForFacingDir(doomCanvas) | 0x400);
	DoomCanvas_checkFacingEntity(doomCanvas);
}


int DoomCanvas_flagForFacingDir(DoomCanvas_t* doomCanvas)
{
	int i = doomCanvas->destAngle & 255;

	if (i == 64) {
		return 0x10000000;
	}
	else if (i == 0) {
		return 0x20000000;
	}
	else if (i == 192) {
		return 0x40000000;
	}
	else if (i == 128) {
		return 0x80000000;
	}
	else {
		return 0;
	}

}

int DoomCanvas_getKeyAction(DoomCanvas_t* doomCanvas, int i) {

	int j;

	if (doomCanvas->state == ST_MENU) {
		if (i & AVK_MENU_UP) {
			return MENU_UP;
		}
		if (i & AVK_MENU_DOWN) {
			return MENU_DOWN;
		}
		if (i & AVK_MENU_PAGE_UP) {
			return MENU_PAGE_UP;
		}
		if (i & AVK_MENU_PAGE_DOWN) {
			return MENU_PAGE_DOWN;
		}
		if (i & AVK_MENU_SELECT) {
			return MENU_SELECT;
		}
	}

	if (i & AVK_MENU_OPEN) {
		return MENU_OPEN;
	}

	i &= ~(AVK_MENU_UP | AVK_MENU_DOWN | AVK_MENU_PAGE_UP | AVK_MENU_PAGE_DOWN | AVK_MENU_SELECT | AVK_MENU_OPEN);

#if 0
	if ((i - AVK_0) >= 0 && (i - AVK_0) <= 9) {
		return keys_numKeyPadActions[(i - AVK_0)];
	}
#endif

	for (j = 0; j < (NUM_CODES/2); j++)
	{
		if (keys_codeActions[(j * 2) + 0] == i)
		{
			return keys_codeActions[(j * 2) + 1];
		}
	}

	return 0;
}

void DoomCanvas_handleDialogEvents(DoomCanvas_t* doomCanvas, int i)
{
	int key;

	key = DoomCanvas_getKeyAction(doomCanvas, i);

	if (key == SELECT || key == 15) {
		if (doomCanvas->dialogTypeLineIdx < 4 && doomCanvas->dialogTypeLineIdx < doomCanvas->numDialogLines - doomCanvas->currentDialogLine) {
			doomCanvas->dialogTypeLineIdx = 4;
			return;
		}
		if (doomCanvas->currentDialogLine < doomCanvas->numDialogLines - 4) {
			doomCanvas->dialogLineStartTime = doomCanvas->time;
			doomCanvas->dialogTypeLineIdx = 0;
			doomCanvas->currentDialogLine += 4;
			return;
		}

		DoomCanvas_closeDialog(doomCanvas);
		if (doomCanvas->game->tileEvent != 0) {
			Game_runEvent(doomCanvas->game, doomCanvas->game->tileEvent, doomCanvas->game->tileEventIndex + 1, doomCanvas->game->tileEventFlags);
		}
	}
	else if (key == MOVEFORWARD) {
		--doomCanvas->currentDialogLine;
		if (doomCanvas->currentDialogLine < 0) {
			doomCanvas->currentDialogLine = 0;
		}
	}
	else if (key == MOVEBACK) {
		++doomCanvas->currentDialogLine;
		if (doomCanvas->currentDialogLine <= doomCanvas->numDialogLines - 4) {
			doomCanvas->dialogLineStartTime = doomCanvas->time;
			doomCanvas->dialogTypeLineIdx = 3;
			return;
		}
		doomCanvas->currentDialogLine = doomCanvas->numDialogLines - 4;
		if (doomCanvas->currentDialogLine < 0) {
			doomCanvas->currentDialogLine = 0;
		}
	}
	else if (doomCanvas->dialogBackSoftKey && (key == TURNLEFT || key == TURNRIGHT || key == MENUOPEN || key == MENU_OPEN || key == 15)) {
		DoomCanvas_closeDialog(doomCanvas);
	}
}

void DoomCanvas_handleEvent(DoomCanvas_t* doomCanvas, int i) {

	int keyAction;

#if 0 // BREW phones Only
	if (i == 0xe047) { /* AVK_VOLUME_UP */
		Sound_addVolume(doomCanvas->doomRpg->sound, 10);
	}
	else {
		if (i == 0xe048) { /* AVK_VOLUME_DOWN */
			Sound_minusVolume(doomCanvas->doomRpg->sound, 10);
		}
	}
#endif

	if (doomCanvas->benchmarkString) {
		doomCanvas->benchmarkString = false;
		doomCanvas->renderOnly = false;
		doomCanvas->doomRpg->menuSystem->field_0xc58 = 80;
	}

	switch (doomCanvas->state) {

	case ST_LEGALS:
		keyAction = DoomCanvas_getKeyAction(doomCanvas, i);
		doomCanvas->legalsSkip = true;
		break;

	case ST_UNK1:
		break;

	case ST_MENU:
		DoomCanvas_handleMenuEvents(doomCanvas, i);
		break;

	case ST_PLAYING:
		DoomCanvas_handlePlayingEvents(doomCanvas, i);
		break;

	case ST_COMBAT:
		if (doomCanvas->combatDone != 0) {
			ParticleSystem_freeAllParticles(doomCanvas->particleSystem);
			DoomCanvas_setState(doomCanvas, ST_PLAYING);

			if (doomCanvas->combat->curAttacker == NULL) {
				Game_advanceTurn(doomCanvas->game);
				if (doomCanvas->state == ST_PLAYING) {
					DoomCanvas_handlePlayingEvents(doomCanvas, i);
				}
			}
		}
		break;

	case ST_AUTOMAP:
		doomCanvas->automapDrawn = false;
		DoomCanvas_handlePlayingEvents(doomCanvas, i);
		break;

	case ST_LOADING:
		break;

	case ST_DIALOG:
		DoomCanvas_handleDialogEvents(doomCanvas, i);
		break;

	case ST_DIALOGPASSWORD:
		DoomCanvas_handlePasswordEvents(doomCanvas, i);
		break;

	case ST_INTRO:
		if (DoomCanvas_getKeyAction(doomCanvas, i) == SELECT) {
			DoomCanvas_handleStoryInput(doomCanvas);
		}
		break;

	case ST_BENCHMARK:
	case ST_BENCHMARKDONE:
		DoomCanvas_setState(doomCanvas, ST_PLAYING);
		DoomCanvas_setAnimFrames(doomCanvas, doomCanvas->animFrames);
		return;

	case ST_DYING:
		break;

	case ST_PARTICLE:
		break;

	case ST_EPILOGUE:
		if (DoomCanvas_getKeyAction(doomCanvas, i) == SELECT) {
			DoomCanvas_handleEpilogueInput(doomCanvas);
		}
		break;

	case ST_CAST:
		keyAction = DoomCanvas_getKeyAction(doomCanvas, i);
		if ((doomCanvas->castSeq >= 0) && (keyAction == 15 || keyAction == SELECT)) {
			doomCanvas->castSeq = 11;
			doomCanvas->castTime = 0;
		}
		break;

	case ST_CREDITS:
		if ((doomCanvas->time - doomCanvas->creditsTextTime) > 24750 && DoomCanvas_getKeyAction(doomCanvas, i) == SELECT) {
			if (doomCanvas->creditsText) {
				SDL_free(doomCanvas->creditsText);
				doomCanvas->creditsText = NULL;
			}
			DoomRPG_freeImage(doomCanvas->doomRpg, &doomCanvas->imgSpaceBG);
			MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_GOTO_JUNCTION);
		}
		break;
	}
}

void DoomCanvas_handlePasswordEvents(DoomCanvas_t* doomCanvas, int i)
{
	int key;
	int len1, len2, j;

	key = DoomCanvas_getKeyAction(doomCanvas, i);

	boolean z = false;
	if (doomCanvas->passwordTime == 0) {

		len1 = SDL_strlen(doomCanvas->passCode);
		len2 = SDL_strlen(doomCanvas->game->passCode);

		//printf("i %d\n", i);
		if ((i - AVK_0) >= 0 && (i - AVK_0) <= 9) {
			doomCanvas->passCode[len1] = (char)(i - AVK_0) + '0';
			doomCanvas->passCode[len1 + 1] = '\0';
			Sound_playSound(doomCanvas->doomRpg->sound, 5060, 0, 3);
			len1++;
			if (len1 == len2) {
				z = true;
			}
		}
		else if (key == TURNLEFT || key == 15) {
			Sound_playSound(doomCanvas->doomRpg->sound, 5042, 0, 3);
			if (len1 != 0 || key != 15) {
				doomCanvas->passCode[len1 - 1] = '\0';
				len1--;
			}
		}
		else if (key == SELECT) {
			Sound_playSound(doomCanvas->doomRpg->sound, 5042, SND_FLG_NOFORCESTOP, 3);
			z = true;
		}

		strcpy_s(doomCanvas->strPassCode, sizeof(doomCanvas->strPassCode), doomCanvas->passCode);

		for (j = len1; j < len2; j++) {
			doomCanvas->strPassCode[j] = '_';
		}
		doomCanvas->strPassCode[j] = '\0';

		if (z) {
			if (len1 == len2) {
				doomCanvas->passwordTime = doomCanvas->time + 300;
			}
			else {
				doomCanvas->passwordTime = doomCanvas->time;
			}
		}
	}
}

void DoomCanvas_handleMenuEvents(DoomCanvas_t* doomCanvas, int i) {

	int k, keyAction, menuKey;

	menuKey = i;
	i &= ~(AVK_MENU_UP | AVK_MENU_DOWN | AVK_MENU_PAGE_UP | AVK_MENU_PAGE_DOWN | AVK_MENU_SELECT | AVK_MENU_OPEN);

	k = (i - AVK_0);

	if ((k > 9) || MenuSystem_enterDigit(doomCanvas->doomRpg->menuSystem, i - AVK_0) == false)
	{
		keyAction = DoomCanvas_getKeyAction(doomCanvas, menuKey);
		//printf("keyAction %d\n", keyAction);

		switch (keyAction)
		{
		case MENU_UP: // Old MOVEFORWARD
			MenuSystem_scrollUp(doomCanvas->doomRpg->menuSystem);
			break;
		case MENU_DOWN: // Old MOVEBACK
			MenuSystem_scrollDown(doomCanvas->doomRpg->menuSystem);
			break;
		case MENU_PAGE_UP: // Old TURNLEFT
			MenuSystem_scrollPageUp(doomCanvas->doomRpg->menuSystem);
			break;
		case MENU_PAGE_DOWN: // Old TURNRIGHT
			MenuSystem_scrollPageDown(doomCanvas->doomRpg->menuSystem);
			break;
		case SELECT:
		case MENU_SELECT: // Old SELECT
			MenuSystem_select(doomCanvas->doomRpg->menuSystem);
			break;
		case MENUOPEN:
		case MENU_OPEN: // Old MENUOPEN
			MenuSystem_back(doomCanvas->doomRpg->menuSystem);
			break;
		default:
			break;
		}
	}
}

void DoomCanvas_handlePlayingEvents(DoomCanvas_t* doomCanvas, int i)
{
	int key, angle;
	boolean z = false;

	key = DoomCanvas_getKeyAction(doomCanvas, i);
	//printf("key %d\n", key);

	if (doomCanvas->renderOnly) {
		doomCanvas->viewX = doomCanvas->destX;
		doomCanvas->viewY = doomCanvas->destY;
		doomCanvas->viewAngle = doomCanvas->destAngle;
		doomCanvas->viewSin = doomCanvas->render->sinTable[doomCanvas->destAngle & 255];
		doomCanvas->viewCos = doomCanvas->render->sinTable[(doomCanvas->destAngle + 64) & 255];
		doomCanvas->viewStepX = (doomCanvas->viewCos * 64) >> 16;
		doomCanvas->viewStepY = ((-doomCanvas->viewSin) * 64) >> 16;
	}
	else {

		if (doomCanvas->game->waitTime) {
			return;
		}

		if (doomCanvas->game->activeSprites) {
			return;
		}

		while (DoomCanvas_updatePlayerAnimDoors(doomCanvas)) {
			z = true;
		}

		if (doomCanvas->viewX != doomCanvas->destX || doomCanvas->viewY != doomCanvas->destY) {
			z = true;
			doomCanvas->viewX = doomCanvas->destX;
			doomCanvas->viewY = doomCanvas->destY;
			DoomCanvas_finishMovement(doomCanvas);
			DoomCanvas_updateViewTrue(doomCanvas);
			//invalidateRect();
		}
		else if (doomCanvas->viewAngle != doomCanvas->destAngle) {
			z = true;
			doomCanvas->viewAngle = doomCanvas->destAngle;
			DoomCanvas_finishRotation(doomCanvas);
			DoomCanvas_updateViewTrue(doomCanvas);
			//invalidateRect();
		}

		if (key != PREVWEAPON && key != NEXTWEAPON && Game_snapMonsters(doomCanvas->game)) {
			return;
		}

		if (doomCanvas->state != ST_PLAYING && doomCanvas->state != ST_AUTOMAP) {
			DoomCanvas_renderScene(doomCanvas, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle);
			DoomCanvas_drawRGB(doomCanvas);
			DoomCanvas_handleEvent(doomCanvas, i);
			return;
		}
	}

	switch (key) {
	case AUTOMAP: {
		DoomCanvas_setState(doomCanvas, (doomCanvas->state != ST_AUTOMAP) ? ST_AUTOMAP : ST_PLAYING);
		break;
	}

	case MOVEFORWARD: {
		DoomCanvas_attemptMove(doomCanvas, doomCanvas->viewX + doomCanvas->viewStepX, doomCanvas->viewY + doomCanvas->viewStepY);
		break;
	}
	case MOVEBACK: {
		DoomCanvas_attemptMove(doomCanvas, doomCanvas->viewX - doomCanvas->viewStepX, doomCanvas->viewY - doomCanvas->viewStepY);
		break;
	}

	case MOVELEFT:
	case MOVERIGHT: {
		angle = 64;
		if (key == MOVERIGHT) {
			angle = -64;
		}
		DoomCanvas_attemptMove(doomCanvas,
			doomCanvas->viewX + (doomCanvas->render->sinTable[doomCanvas->viewAngle + angle + 64 & 0xFF] * 64 >> 16),
			doomCanvas->viewY + (-doomCanvas->render->sinTable[doomCanvas->viewAngle + angle & 0xFF] * 64 >> 16));
		break;
	}

	case TURNLEFT:
	case TURNRIGHT: {
		angle = 64;
		if (key == TURNRIGHT) {
			angle = -64;
		}
		doomCanvas->destAngle += angle;
		DoomCanvas_updateViewTrue(doomCanvas);
		doomCanvas->f438d = true;
		break;
	}

	case PREVWEAPON: {
		Player_selectPrevWeapon(doomCanvas->player);
		doomCanvas->f438d = true;
		break;
	}

	case NEXTWEAPON: {
		Player_selectNextWeapon(doomCanvas->player);
		doomCanvas->f438d = true;
		break;
	}

	case MENUOPEN:
	case MENU_OPEN: {
		doomCanvas->hud->msgCount = 0;
		MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_INGAME);
		break;
	}

	case SELECT: {
		//printf("ACTION\n");
		doomCanvas->f438d = true;

		Entity_t* ent, * traceEnt;
		Sprite_t* spr;
		boolean fireWpn;
		int i, j;

		if (!Game_executeTile(doomCanvas->game, doomCanvas->destX + doomCanvas->viewStepX, doomCanvas->destY + doomCanvas->viewStepY, 1280)) {
			Game_trace(doomCanvas->game, 
				doomCanvas->viewX, doomCanvas->viewY, 
				doomCanvas->viewX + (8 * doomCanvas->viewStepX), 
				doomCanvas->viewY + (8 * doomCanvas->viewStepY), NULL, 22151);

			//printf("numTraceEntities %d\n", doomCanvas->game->numTraceEntities);

			ent = NULL;
			fireWpn = false;
			i = 0;

			while (i < doomCanvas->game->numTraceEntities) {

				traceEnt = doomCanvas->game->traceEntities[i];
				if (traceEnt->def->eType != 14 && (traceEnt->info & 0x200000) == 0x0 && traceEnt->info != 0 && 
					(spr = &doomCanvas->render->mapSprites[(traceEnt->info & 0xFFFF) - 1])->x >> 6 == doomCanvas->viewX >> 6 && spr->y >> 6 == doomCanvas->viewY >> 6) {
					fireWpn = false;
					++i;
				}
				else {
					if (traceEnt->def->eType == 2) { // Humans
						break;
					}
					else if (traceEnt->def->eType == 12) { // Destructible Object
						if ((1 << doomCanvas->player->weapon & traceEnt->def->parm)) {
							ent = traceEnt;
							fireWpn = true;
						}
						break;
					}
					else if (traceEnt->def->eType == 10) { // Fire
						if (doomCanvas->player->weapon == 1) {
							ent = traceEnt;
							fireWpn = true;
						}
						break;
					}
					else if (traceEnt->def->eType == 1) { // Enemy
						ent = traceEnt;
						fireWpn = true;
						break;
					}
					else if (traceEnt->def->eType != 0) {
						fireWpn = false;
						break;
					}
					else {
						Entity_t* v3;
						for (j = i + 1; j < doomCanvas->game->numTraceEntities && ((v3 = doomCanvas->game->traceEntities[j])->def->eType != 1 || v3->linkIndex != traceEnt->linkIndex); ++j) {}

						if (j == doomCanvas->game->numTraceEntities) {
							break;
						}

						ent = doomCanvas->game->traceEntities[j];
						fireWpn = true;
						break;
					}
				}
			}

			if (fireWpn) {
				Player_fireWeapon(doomCanvas->doomRpg->player, ent);
			}
			else {
				Hud_addMessage(doomCanvas->hud, "Nothing to use");
				Sound_playSound(doomCanvas->doomRpg->sound, 5065, 0, 2);
			}
		}
		else if (!doomCanvas->game->skipAdvanceTurn) {
			Game_touchTile(doomCanvas->game, doomCanvas->destX, doomCanvas->destY, false);
			Game_advanceTurn(doomCanvas->game);
		}
		break;
	}

	case PASSTURN: {
		Hud_addMessage(doomCanvas->hud, "Turn passed.");
		Game_touchTile(doomCanvas->game, doomCanvas->destX, doomCanvas->destY, false);
		Game_advanceTurn(doomCanvas->game);
		break;
	}

	case 15: {
		if (doomCanvas->state == ST_AUTOMAP) {
			DoomCanvas_setState(doomCanvas, ST_PLAYING);
		}
		else {
			doomCanvas->hud->msgCount = 0;
			MenuSystem_setMenu(doomCanvas->doomRpg->menuSystem, MENU_INGAME);
		}
	}

	}

	if (z && doomCanvas->state != ST_PLAYING && doomCanvas->state != ST_AUTOMAP) {
		DoomCanvas_renderScene(doomCanvas, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle);
		DoomCanvas_drawRGB(doomCanvas);
	}
}


void DoomCanvas_initCredits(DoomCanvas_t* doomCanvas)
{
	DoomRPG_createImage(doomCanvas->doomRpg, "c.bmp", 0, &doomCanvas->imgSpaceBG);
	doomCanvas->creditsTextTime = -1;
}

void DoomCanvas_loadEpilogueText(DoomCanvas_t* doomCanvas)
{
	int overall;
	char rank[32];
	doomCanvas->epilogueTextPage = 0;
	doomCanvas->showTextDone = 0;

	overall = DoomCanvas_getOverall(doomCanvas);

	SDL_memset(rank, 0, sizeof(rank));

	if (overall >= 80) {
		strncpy_s(rank, sizeof(rank), "Master", sizeof(rank));
		strncpy_s(doomCanvas->epilogueText[1], sizeof(*doomCanvas->epilogueText), 
			"You have found\n"
			"every secret and\n"
			"killed every mon-\n"
			"ster in the game.\n"
			"This calls for a\n"
			"celebration!\n"
			"Please visit:\n"
			"\n"
			"doomrpg.com/sarge\n"
			, sizeof(*doomCanvas->epilogueText));
	}
	else if (overall >= 70) {
		strncpy_s(rank, sizeof(rank), "Baddy", sizeof(rank));
		strncpy_s(doomCanvas->epilogueText[1], sizeof(*doomCanvas->epilogueText),
			"Nice job. There\n"
			"is only a little\n"
			"more you need to\n"
			"do to achieve\n"
			"Master rank. We\n"
			"have a parade\n"
			"for you at:\n"
			"\n"
			"doomrpg.com/blues\n"
			, sizeof(*doomCanvas->epilogueText));
	}
	else if (overall >= 50) {
		strncpy_s(rank, sizeof(rank), "Average", sizeof(rank));
		strncpy_s(doomCanvas->epilogueText[1], sizeof(*doomCanvas->epilogueText),
			"You've beaten the\n"
			"demons from Hell,\n"
			"but we've seen\n"
			"better. Find out\n"
			"more by visiting:\n"
			"\n"
			"doomrpg.com/spire\n"
			, sizeof(*doomCanvas->epilogueText));
	}
	else {
		strncpy_s(rank, sizeof(rank), "Chump", sizeof(rank));
		strncpy_s(doomCanvas->epilogueText[1], sizeof(*doomCanvas->epilogueText),
			"You've finished\n"
			"the game, barely.\n"
			"There's still much\n"
			"to discover. For a\n"
			"little inspiration\n"
			"to do better next\n"
			"time, visit:\n"
			"\n"
			"doomrpg.com/hound\n"
			, sizeof(*doomCanvas->epilogueText));
	}

	SDL_snprintf(doomCanvas->epilogueText[0], sizeof(*doomCanvas->epilogueText),
		"Congratulations!\n"
		"You've shut down\n"
		"the portal to\n"
		"Hell and stopped\n"
		"the demonic inv-\n"
		"asion.\n"
		"\n"
		"Rank: %s", rank);

	DoomRPG_createImage(doomCanvas->doomRpg, "c.bmp", false, &doomCanvas->imgSpaceBG);
	Sound_playSound(doomCanvas->doomRpg->sound, 5039, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
	doomCanvas->epilogueTextTime = -1;
}


static char storyTextA[] = "You have been\ndispatched in re - \nsponse to a dis - \ntress call from\nUnion Aerospace\nCorporation's re-\nsearch facility\non Mars. The base\nis under attack";
static char storyTextB[] = "by an unknown\nforce and your\nmission is to ac-\nquire intelli-\ngence and neu-\ntralize the\nthreat.";
static char storyTextC[] = "Insertion com-\nplete. For fur-\nther instruc-\ntions, rendezvous\nwith the other\nMarines at Junc-\ntion. Expect\nheavy resistance.\nGood luck!";

void DoomCanvas_loadPrologueText(DoomCanvas_t* doomCanvas)
{
	int textLen;
	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
	DoomCanvas_drawString1(doomCanvas, "Loading...", doomCanvas->SCR_CX, doomCanvas->SCR_CY, 17);

	DoomRPG_flushGraphics(doomCanvas->doomRpg);
	Sound_playSound(doomCanvas->doomRpg->sound, 5039, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);

	textLen = SDL_strlen(storyTextA);
	doomCanvas->storyText1[0] = SDL_malloc(textLen + 1);
	strncpy_s(doomCanvas->storyText1[0], textLen + 1, storyTextA, textLen);

	textLen = SDL_strlen(storyTextB);
	doomCanvas->storyText1[1] = SDL_malloc(textLen + 1);
	strncpy_s(doomCanvas->storyText1[1], textLen + 1, storyTextB, textLen);

	textLen = SDL_strlen(storyTextC);
	doomCanvas->storyText2 = SDL_malloc(textLen + 1);
	strncpy_s(doomCanvas->storyText2, textLen + 1, storyTextC, textLen);

	DoomRPG_createImage(doomCanvas->doomRpg, "c.bmp", false, &doomCanvas->imgSpaceBG);
	DoomRPG_createImage(doomCanvas->doomRpg, "d.bmp", true, &doomCanvas->imgLinesLayer);
	DoomRPG_createImage(doomCanvas->doomRpg, "e.bmp", true, &doomCanvas->imgPlanetLayer);
	DoomRPG_createImage(doomCanvas->doomRpg, "f.bmp", true, &doomCanvas->imgSpaceship);
	doomCanvas->storyTextTime = -1;
	doomCanvas->storyAnimTime = -1;
	doomCanvas->showTextDone = 0;
	doomCanvas->storyPage = 0;
	doomCanvas->storyTextPage = 0;

	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
	DoomRPG_flushGraphics(doomCanvas->doomRpg);
}

void DoomCanvas_keyPressed(DoomCanvas_t* doomCanvas, int keyCode)
{
	byte numEvents;

	if (keyCode == AVK_UNDEFINED) {
		return;
	}

	numEvents = doomCanvas->numEvents;
	if (numEvents != 8) {
		doomCanvas->numEvents++;
		doomCanvas->events[numEvents] = keyCode;
		//printf("numEvents %d Code %d\n", numEvents, doomCanvas->events[numEvents]);
	}
}

void DoomCanvas_loadMap(DoomCanvas_t* doomCanvas, int mapID)
{
	int stateNum;

	doomCanvas->loadMapID = mapID;
	Sound_stopSounds(doomCanvas->doomRpg->sound);
	if (doomCanvas->loadMapID == MAP_END_GAME) {
		Render_freeRuntime(doomCanvas->doomRpg->render);
		Game_unloadMapData(doomCanvas->doomRpg->game);
		stateNum = ST_EPILOGUE;
	}
	else {
		doomCanvas->printMsg[0] = '\0';
		stateNum = ST_LOADING;
	}
	DoomCanvas_setState(doomCanvas, stateNum);
}

boolean DoomCanvas_loadMedia(DoomCanvas_t* doomCanvas)
{
	DoomCanvas_unloadMedia(doomCanvas);
	doomCanvas->menuSystem->imgBG = NULL;
	Render_freeRuntime(doomCanvas->render);
	Game_unloadMapData(doomCanvas->game);
	Sound_freeSounds(doomCanvas->doomRpg->sound);

	if (Render_beginLoadMap(doomCanvas->render, doomCanvas->loadMapID))
	{
		DoomRPG_setColor(doomCanvas->doomRpg, doomCanvas->render->introColor);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		DoomCanvas_drawString1(doomCanvas, "Loading", doomCanvas->SCR_CX, doomCanvas->SCR_CY - 48, 17);
		DoomCanvas_drawString2(doomCanvas, doomCanvas->render->mapName, doomCanvas->SCR_CX, doomCanvas->SCR_CY - 36, 17, -1);
		doomCanvas->fillRectIndex = 0;
		//DoomCanvas_updateLoadingBar(doomCanvas);

		if (doomCanvas->game->isSaved) {
			DoomCanvas_drawString1(doomCanvas, "Game Saved", doomCanvas->SCR_CX, doomCanvas->displayRect.h, 18);
		}
		else {
			DoomCanvas_drawString1(doomCanvas, "Game Loaded", doomCanvas->SCR_CX, doomCanvas->displayRect.h, 18);
		}

		DoomRPG_flushGraphics(doomCanvas->doomRpg);

		if (Render_beginLoadMapData(doomCanvas->render))
		{
			Game_loadMapEntities(doomCanvas->game);

			if ((doomCanvas->game->isLoaded) && (doomCanvas->game->activeLoadType == 1)) {
				Game_loadWorldState(doomCanvas->game);
			}

			Game_spawnPlayer(doomCanvas->game);
			DoomCanvas_finishRotation(doomCanvas);

			doomCanvas->hud->msgCount = 0;
			doomCanvas->hud->statBarMessage = NULL;
			doomCanvas->hud->logMessage[0] = '\0';

			if (doomCanvas->loadMapID == MAP_JUNCTION) {
				Game_givemap(doomCanvas->game);
			}
			else {
				DoomCanvas_uncoverAutomap(doomCanvas);
			}

			Player_selectWeapon(doomCanvas->player, doomCanvas->player->weapon);

			if ((doomCanvas->loadMapID != MAP_END_GAME) && (doomCanvas->game->isLoaded == false)) {
				Game_saveState(doomCanvas->game, doomCanvas->loadMapID, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle, false);
			}

			doomCanvas->game->isLoaded = false;
			doomCanvas->game->isSaved = false;
			doomCanvas->game->activeLoadType = 0;
			doomCanvas->numEvents = 0;
			ParticleSystem_freeAllParticles(doomCanvas->particleSystem);
			doomCanvas->numEvents = 0;
			doomCanvas->isUpdateView = true;
			DoomCanvas_setState(doomCanvas, ST_PLAYING);
			doomCanvas->idleTime = doomCanvas->time + 8000;
			return true;
		}
	}

	return false;
}

void DoomCanvas_LoadMenuMap(DoomCanvas_t* doomCanvas)
{
	if (Render_beginLoadMap(doomCanvas->doomRpg->render, MAP_MENU))
	{
		if (Render_beginLoadMapData(doomCanvas->doomRpg->render))
		{
			Game_loadMapEntities(doomCanvas->doomRpg->game);
			Render_setGrayPalettes(doomCanvas->doomRpg->render);

			short fColor = doomCanvas->doomRpg->render->floorColor[0];
			short cColor = doomCanvas->doomRpg->render->ceilingColor[0];

			Render_setup(doomCanvas->doomRpg->render, &doomCanvas->displayRect);

			for (int i = 0; i < doomCanvas->doomRpg->render->screenWidth; i++) {
				doomCanvas->doomRpg->render->floorColor[i] = fColor;
				doomCanvas->doomRpg->render->ceilingColor[i] = cColor;
			}

			doomCanvas->viewX = ((doomCanvas->doomRpg->render->mapSpawnIndex % 32) << 6) + 32;
			doomCanvas->viewY = ((doomCanvas->doomRpg->render->mapSpawnIndex / 32) << 6) + 32;
			doomCanvas->viewAngle = doomCanvas->doomRpg->render->mapSpawnDir;
			doomCanvas->doomRpg->render->enableViewRotation = true;
			doomCanvas->unloadMedia = true;
		}
	}
}

void DoomCanvas_loadState(DoomCanvas_t* doomCanvas, int i, char* text)
{
	doomCanvas->loadType = i;

	if (text == NULL) {
		doomCanvas->printMsg[0] = '\0';
	}
	else {
		strncpy_s(doomCanvas->printMsg, sizeof(doomCanvas->printMsg), text, sizeof(doomCanvas->printMsg));
	}

	DoomCanvas_setState(doomCanvas, ST_LOADING);
}

void DoomCanvas_legalsState(DoomCanvas_t* doomCanvas) {

	int time, ySrc;
	Image_t* img;

	time = DoomRPG_GetUpTimeMS();

	if (doomCanvas->legalsTime == 0) {
		doomCanvas->legalsTime = time;
	}
	else {

		if (doomCanvas->legalsSkip) {
			goto skip;
		}

		int v4 = time - doomCanvas->legalsTime;
		boolean v5 = (v4 > 0x400);
		if (v4 >= 0x400) {
			v5 = (v4 - 1024 > 0x1DC);
		}
		if (v5)
		{
		skip:
			doomCanvas->legalsTime = time;
			doomCanvas->legalsNextImage++;
			doomCanvas->legalsSkip = false;
		}
	}

	ySrc = doomCanvas->legalsNextImage;
	img = &doomCanvas->imgLegals;

	if (ySrc <= 3) {
		DoomCanvas_drawImageSpecial(doomCanvas, img, 0, ySrc << 7, 128, 128, 0, doomCanvas->SCR_CX - 64, doomCanvas->SCR_CY - 64, 0);
	}
	else
	{
		DoomRPG_freeImage(doomCanvas->doomRpg, img);

		if (doomCanvas->insufficientSpace) {
			DoomCanvas_setState(doomCanvas, ST_SORRY);
		}
		else {
			DoomCanvas_setupmenu(doomCanvas, true);
		}
	}
}

void DoomCanvas_menuState(DoomCanvas_t* doomCanvas)
{
	if (doomCanvas->doomRpg->menuSystem->menu >= MENU_INGAME) {
		Hud_drawTopBar(doomCanvas->hud);
		Hud_drawBottomBar(doomCanvas->hud);
	}
	MenuSystem_paint(doomCanvas->doomRpg->menuSystem);

	if (doomCanvas->doomRpg->menuSystem->oldMenu == -1) {
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
	}
	else {
		DoomCanvas_drawSoftKeys(doomCanvas, "Back", NULL);
	}
}

void DoomCanvas_handleEpilogueInput(DoomCanvas_t* doomCanvas)
{
	if (!doomCanvas->showTextDone) {
		doomCanvas->showTextDone = true;
	}
	else {
		doomCanvas->showTextDone = false;
		doomCanvas->epilogueTextPage++;
		doomCanvas->epilogueTextTime = -1;
		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
}

void DoomCanvas_handleStoryInput(DoomCanvas_t* doomCanvas)
{
	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
	DoomRPG_setColor(doomCanvas->doomRpg, 0xffffff);

	if (doomCanvas->storyPage == 1) {
		DoomCanvas_changeStoryPage(doomCanvas);
	}
	else if (doomCanvas->showTextDone) {
		doomCanvas->storyTextPage++;
		doomCanvas->storyTextTime = -1;
		doomCanvas->showTextDone = false;
	}
	else {
		doomCanvas->showTextDone = true;
	}
}

void DoomCanvas_changeStoryPage(DoomCanvas_t* doomCanvas)
{
	doomCanvas->storyPage++;
	if (doomCanvas->storyPage == 3) {
		DoomCanvas_disposeIntro(doomCanvas);
		return;
	}
	doomCanvas->storyTextPage = 0;
	doomCanvas->storyAnimTime = -1;
	doomCanvas->storyTextTime = -1;
}

void DoomCanvas_playingState(DoomCanvas_t* doomCanvas)
{
	Entity_t* activeMonsters;
	Game_t* game;
	int i, j, snd;

	game = doomCanvas->game;
	if (doomCanvas->renderOnly) {
		DoomCanvas_renderOnlyState(doomCanvas);
	}
	else if (CombatEntity_getHealth(&doomCanvas->player->ce) <= 0) {
		Player_died(doomCanvas->player);
	}
	else {
		if (game->waitTime != 0 && doomCanvas->time >= game->waitTime) {
			game->waitTime = 0;
			Game_runEvent(game, game->tileEvent, game->tileEventIndex + 1, game->tileEventFlags);
		}

		boolean applyBerserk = false;
		if (game->activeSprites || !game->monstersTurn || doomCanvas->openDoorsCount != 0 || Game_updateMonsters(game))
		{
			if (!(doomCanvas->viewX == doomCanvas->destX && doomCanvas->viewY == doomCanvas->destY && doomCanvas->viewAngle == doomCanvas->destAngle)) {
				ParticleSystem_freeAllParticles(doomCanvas->particleSystem);
			}

			DoomCanvas_updateView(doomCanvas);
			applyBerserk = true;
		} // <- Agregué el corchete aquí, ya que necesito que los gráficos se actualicen siempre en cada cuadro, 
		  //    sin que intervengan las actualizaciones del movimiento del jugador.
		  // <- I added the bracket here as I need the graphics to always update on every frame, 
		  //    without player movement updates intervening.

			DoomCanvas_drawRGB(doomCanvas);

			boolean renderParticle = true;
			if (doomCanvas->particleSystem->particleCount > 0) {
				if (!doomCanvas->slowBlit || doomCanvas->time - doomCanvas->particleSystem->endTime >= 250) {
					DoomCanvas_invalidateRectAndUpdateView(doomCanvas);
				}
				else {
					renderParticle = false;
				}
			}

			DoomCanvas_drawRGB(doomCanvas);

			// En el código original esta función está en la función "Hud_drawEffects", pero decidí moverla aquí, 
			// esto evita que se superponga a otros objetos dibujados previamente.
			// 
			// In the original code this function is in the "Hud_drawEffects" function, but I decided to move it here, 
			// this prevents it from overlapping other previously drawn objects
			{
				if (doomCanvas->doomRpg->player->berserkerTics && applyBerserk) {
					Render_setBerserkColor(doomCanvas->doomRpg->render);
				}
			}

			if (renderParticle) {
				ParticleSystem_render(doomCanvas->particleSystem, true);
			}

			Hud_drawTopBar(doomCanvas->hud);
			Hud_drawBottomBar(doomCanvas->hud);
			Hud_drawEffects(doomCanvas->hud);

			activeMonsters = doomCanvas->game->activeMonsters;
			if (activeMonsters && (doomCanvas->time > doomCanvas->idleTime))
			{
				i = 0;
				j = 0;
				do
				{
					++i;
					activeMonsters = activeMonsters->monster->nextOnList;
					if (doomCanvas->game->activeMonsters == activeMonsters)
					{
						j = i;
						i = (-DoomRPG_randNextByte(&doomCanvas->doomRpg->random) % i);
					}
				} while (i);

				doomCanvas->idleTime = (DoomRPG_randNextByte(&doomCanvas->doomRpg->random) << 6) - 1000 * j;
				if (doomCanvas->idleTime < 0) {
					doomCanvas->idleTime = 0;
				}

				doomCanvas->idleTime = doomCanvas->idleTime + doomCanvas->time + 6000;

				// Idle Sound
				snd = EntityMonster_getSoundID(activeMonsters->monster, 0);
				if (snd != 0) {
					printf("Playing idle sound %d at %d\n", snd, doomCanvas->time);
					Sound_playSound(doomCanvas->doomRpg->sound, snd, 0, 1);
				}
			}
		//} // Old
	}
}

void DoomCanvas_prepareDialog(DoomCanvas_t* doomCanvas, char* str, boolean dialogBackSoftKey)
{
	int strLen, i, j;

	i = 0;
	j = 0;
	doomCanvas->numDialogLines = 0;
	strcpy_s(doomCanvas->dialogBuffer, sizeof(doomCanvas->dialogBuffer), str);

	strLen = SDL_strlen(doomCanvas->dialogBuffer);
	while (j < strLen) {
		if (str[j] == '|') {
			doomCanvas->dialogIndexes[(doomCanvas->numDialogLines * 2) + 0] = (short)i;
			doomCanvas->dialogIndexes[(doomCanvas->numDialogLines * 2) + 1] = (short)(j - i);
			doomCanvas->numDialogLines++;
			i = j + 1;
		}
		++j;
	}
	doomCanvas->dialogIndexes[(doomCanvas->numDialogLines * 2) + 0] = (short)i;
	doomCanvas->dialogIndexes[(doomCanvas->numDialogLines * 2) + 1] = (short)(strLen - i);
	doomCanvas->numDialogLines++;

	if (doomCanvas->state == ST_DIALOGPASSWORD) {
		strLen = SDL_strlen(doomCanvas->game->passCode);
		SDL_memset(doomCanvas->strPassCode, '_', strLen);
		doomCanvas->strPassCode[strLen] = '\0';
	}

	doomCanvas->currentDialogLine = 0;
	doomCanvas->dialogLineStartTime = doomCanvas->time;
	doomCanvas->dialogTypeLineIdx = 0;
	doomCanvas->dialogBackSoftKey = dialogBackSoftKey;
}

void DoomCanvas_restoreSoftKeys(DoomCanvas_t* doomCanvas)
{
	char* softKeyLeft, * softKeyRight;

	softKeyLeft = doomCanvas->softKeyLeft[0] == '\0' ? NULL : doomCanvas->softKeyLeft;
	softKeyRight = doomCanvas->softKeyRight[0] == '\0' ? NULL : doomCanvas->softKeyRight;

	DoomCanvas_drawSoftKeys(doomCanvas, softKeyLeft, softKeyRight);
}

void DoomCanvas_renderOnlyState(DoomCanvas_t* doomCanvas)
{
	char text[128];

	if (doomCanvas->benchmarkString == false) {

		if (doomCanvas->viewX == doomCanvas->destX &&
			doomCanvas->viewY == doomCanvas->destY &&
			doomCanvas->viewAngle == doomCanvas->destAngle) {
			return;
		}

		if (doomCanvas->viewX < doomCanvas->destX) {
			doomCanvas->viewX += doomCanvas->animPos;
			if (doomCanvas->viewX > doomCanvas->destX) {
				doomCanvas->viewX = doomCanvas->destX;
			}
		}
		else if (doomCanvas->viewX > doomCanvas->destX) {
			doomCanvas->viewX -= doomCanvas->animPos;
			if (doomCanvas->viewX < doomCanvas->destX) {
				doomCanvas->viewX = doomCanvas->destX;
			}
		}

		if (doomCanvas->viewY < doomCanvas->destY) {
			doomCanvas->viewY += doomCanvas->animPos;
			if (doomCanvas->viewY > doomCanvas->destY) {
				doomCanvas->viewY = doomCanvas->destY;
			}
		}
		else if (doomCanvas->viewY > doomCanvas->destY) {
			doomCanvas->viewY -= doomCanvas->animPos;
			if (doomCanvas->viewY < doomCanvas->destY) {
				doomCanvas->viewY = doomCanvas->destY;
			}
		}

		if (doomCanvas->viewAngle < doomCanvas->destAngle) {
			doomCanvas->viewAngle += doomCanvas->animAngle;
			if (doomCanvas->viewAngle > doomCanvas->destAngle) {
				doomCanvas->viewAngle = doomCanvas->destAngle;
			}
		}
		else if (doomCanvas->viewAngle > doomCanvas->destAngle) {
			doomCanvas->viewAngle -= doomCanvas->animAngle;
			if (doomCanvas->viewAngle < doomCanvas->destAngle) {
				doomCanvas->viewAngle = doomCanvas->destAngle;
			}
		}
	}
	else {
		doomCanvas->viewAngle = (doomCanvas->viewAngle + doomCanvas->animAngle) & 0xff;
	}

	doomCanvas->lastFrameTime = doomCanvas->time;
	Render_render(doomCanvas->render, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewZ, doomCanvas->viewAngle);
	DoomCanvas_invalidateRectAndUpdateView(doomCanvas); 
	DoomCanvas_drawRGB(doomCanvas);

	if (doomCanvas->benchmarkString) {
		int avg1, avg2;

		avg1 = (doomCanvas->renderAvgMs * 100) / doomCanvas->st_count;
		avg2 = doomCanvas->renderAvgMs / doomCanvas->st_count;
		SDL_snprintf(text, sizeof(text), "Render Avg: %d.%02dms", avg2, avg1 - (avg2 * 100));
		DoomCanvas_drawString2(doomCanvas, text, 0, doomCanvas->doomRpg->hud->statusTopBarHeight, 0, -1);

		avg1 = (doomCanvas->horizAvgMs * 100) / doomCanvas->st_count;
		avg2 = doomCanvas->horizAvgMs / doomCanvas->st_count;
		SDL_snprintf(text, sizeof(text), " Horiz Avg: %d.%02dms", avg2, avg1 - (avg2 * 100));
		DoomCanvas_drawString2(doomCanvas, text, 0, doomCanvas->doomRpg->hud->statusTopBarHeight + 12, 0, -1);

		avg1 = (doomCanvas->loopAvgMs * 100) / doomCanvas->st_count;
		avg2 = doomCanvas->loopAvgMs / doomCanvas->st_count;
		SDL_snprintf(text, sizeof(text), " Total Avg: %d.%02dms", avg2, avg1 - (avg2 * 100));
		DoomCanvas_drawString2(doomCanvas, text, 0, doomCanvas->doomRpg->hud->statusTopBarHeight + 24, 0, -1);
	}

	doomCanvas->loopEnd = DoomRPG_GetUpTimeMS();
}

void DoomCanvas_renderScene(DoomCanvas_t* doomCanvas, int x, int y, int angle)
{
	doomCanvas->lastFrameTime = doomCanvas->time;
	doomCanvas->beforeRender = DoomRPG_GetUpTimeMS();
	Render_render(doomCanvas->render, x, y, doomCanvas->viewZ, angle);
	if (doomCanvas->state != ST_CAST) {
		Combat_drawWeapon(doomCanvas->combat, doomCanvas->shakeX, doomCanvas->shakeY - (doomCanvas->captureState == 2 ? 10 : 0));
	}
	doomCanvas->afterRender = DoomRPG_GetUpTimeMS();
}

void DoomCanvas_resume(DoomCanvas_t* doomCanvas)
{

}

void DoomCanvas_runInputEvents(DoomCanvas_t* doomCanvas)
{
	int i;

	for (i = 0; i < doomCanvas->numEvents; i++) {
		DoomCanvas_handleEvent(doomCanvas, doomCanvas->events[i]);
	}
	doomCanvas->numEvents = 0;
}


void DoomCanvas_run(DoomCanvas_t* doomCanvas)
{
	int sound;

	// New Code Lines
	//{
	DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
	DoomRPG_clearGraphics(doomCanvas->doomRpg);
	//}

	//DoomCanvas_updateLoadingBar(doomCanvas);

	doomCanvas->oldState = doomCanvas->state;
	DoomRPG_setRand(&doomCanvas->doomRpg->random);

	doomCanvas->doomRpg->sound->nextplay = 0;
	if (doomCanvas->doomRpg->graphSetCliping != 0) {
		DoomRPG_setClipFalse(doomCanvas->doomRpg);
	}

	if (doomCanvas->benchmarkString != 0) {
		doomCanvas->st_count++;
		doomCanvas->renderAvgMs += (doomCanvas->render->frameTime - doomCanvas->render->currentFrameTime);
		doomCanvas->horizAvgMs += (doomCanvas->render->horizonFrameTime - doomCanvas->render->currentFrameTime);
		doomCanvas->loopAvgMs += (doomCanvas->loopEnd - doomCanvas->loopStart);
	}

	doomCanvas->loopStart = doomCanvas->time = DoomRPG_GetUpTimeMS();
	DoomCanvas_runInputEvents(doomCanvas);
	DoomCanvas_updatePlayerAnimDoors(doomCanvas);
	Game_gsprite_update(doomCanvas->game);

	if ((doomCanvas->state == ST_SAVING || doomCanvas->state == ST_LOADING) && (doomCanvas->state != doomCanvas->oldState)) {
		return;
	}

	if (DoomRPG_getErrorID(doomCanvas->doomRpg)) {
		return;
	}

	switch (doomCanvas->state)
	{
		case ST_LEGALS: {
			DoomCanvas_legalsState(doomCanvas);
			break;
		}

		case ST_UNK1: {
			break;
		}

		case ST_MENU: {
			DoomCanvas_menuState(doomCanvas);
			break;
		}

		case ST_PLAYING: {
			DoomCanvas_playingState(doomCanvas);
			doomCanvas->restoreSoftKeys = true; // New Code Line
			break;
		}

		case ST_COMBAT: {
			DoomCanvas_combatState(doomCanvas);
			doomCanvas->restoreSoftKeys = true; // New Code Line
			break;
		}

		case ST_AUTOMAP: {
			DoomCanvas_automapState(doomCanvas);
			break;
		}

		case ST_LOADING: {
			if (doomCanvas->loadType == 0) {
				if (DoomCanvas_loadMedia(doomCanvas)) {
					if (doomCanvas->loadMapID == MAP_END_GAME) {
						short fColor = doomCanvas->doomRpg->render->floorColor[0];
						short cColor = doomCanvas->doomRpg->render->ceilingColor[0];

						Render_setup(doomCanvas->render, &doomCanvas->displayRect);

						for (int i = 0; i < doomCanvas->doomRpg->render->screenWidth; i++) {
							doomCanvas->doomRpg->render->floorColor[i] = fColor;
							doomCanvas->doomRpg->render->ceilingColor[i] = cColor;
						}
						DoomCanvas_setState(doomCanvas, ST_CAST);
					}
				}
			}
			else {
				Game_loadState(doomCanvas->doomRpg->game, doomCanvas->loadType);
				Hud_addMessage(doomCanvas->doomRpg->hud, "Game Loaded");
				doomCanvas->loadType = 0;
			}
			break;
		}

		case ST_DIALOG: {
			DoomCanvas_dialogState(doomCanvas);
			doomCanvas->restoreSoftKeys = true; // New Code Line
			break;
		}

		case ST_DIALOGPASSWORD: {

			if (doomCanvas->passwordTime != 0 && doomCanvas->time > doomCanvas->passwordTime) {
				DoomCanvas_closeDialog(doomCanvas);
				if (SDL_strcmp(doomCanvas->passCode, doomCanvas->game->passCode) == 0) {
					Hud_addMessageForce(doomCanvas->hud, "Correct code!", true);
					Game_runEvent(doomCanvas->game, doomCanvas->game->tileEvent, doomCanvas->game->tileEventIndex + 1, doomCanvas->game->tileEventFlags);
				}
				else if (doomCanvas->passCode[0] != '\0') {
					Hud_addMessageForce(doomCanvas->hud, "Invalid code!", true);
				}
			}

			DoomCanvas_dialogState(doomCanvas);
			break;
		}

		case ST_INTRO: {
			DoomCanvas_drawStory(doomCanvas);
			doomCanvas->restoreSoftKeys = false;
			break;
		}

		case ST_BENCHMARK: {
			DoomCanvas_playingState(doomCanvas);
			break;
		}

		case ST_BENCHMARKDONE: {
			break;
		}

		case ST_CAPTUREDOG: {
			DoomCanvas_playingState(doomCanvas);
			DoomCanvas_captureDogState(doomCanvas);
			doomCanvas->restoreSoftKeys = true; // New Code Line
			break;
		}

		case ST_DYING: {
			DoomCanvas_dyingState(doomCanvas);
			break;
		}

		case ST_PARTICLE: {
			if (doomCanvas->particleSystem->nodeListA.next == &doomCanvas->particleSystem->nodeListA) {
				DoomCanvas_setState(doomCanvas, ST_PLAYING);
			}
			if (!doomCanvas->slowBlit || doomCanvas->time - doomCanvas->doomRpg->particleSystem->endTime >= 250) {
				DoomCanvas_invalidateRectAndUpdateView(doomCanvas);
			}
			DoomCanvas_playingState(doomCanvas);
			doomCanvas->restoreSoftKeys = true; // New Code Line
			break;
		}

		case ST_EPILOGUE: {
			DoomCanvas_drawEpilogue(doomCanvas);
			doomCanvas->restoreSoftKeys = false;
			break;
		}

		case ST_CAST: {
			DoomCanvas_castState(doomCanvas);
			doomCanvas->restoreSoftKeys = false;
			break;
		}

		case ST_CREDITS: {
			DoomCanvas_drawCredits(doomCanvas);
			doomCanvas->restoreSoftKeys = false;
			break;
		}

		case ST_SAVING: {
			if (doomCanvas->saveType & 4) {
				Game_saveState(doomCanvas->doomRpg->game, MAP_JUNCTION, (51 * 32), (41 * 32), 0, false);
				Hud_addMessage(doomCanvas->doomRpg->hud, "Game Saved");
			}
			else {
				int v14 = (doomCanvas->saveType & 2) == 0;
				if ((doomCanvas->saveType & 2) == 0)
					v14 = (doomCanvas->saveType & 1) == 0;
				if (!v14) {
					Game_saveState(doomCanvas->doomRpg->game, doomCanvas->loadMapID, doomCanvas->destX, doomCanvas->destY, doomCanvas->destAngle, (doomCanvas->saveType & 2));
					Hud_addMessage(doomCanvas->doomRpg->hud, "Game Saved");
				}

			}
			if (doomCanvas->saveType & 8) {
				DoomCanvas_setupmenu(doomCanvas, 0);
			}
			else {
				DoomCanvas_setState(doomCanvas, ST_PLAYING);
			}
			doomCanvas->saveType = 0;
			break;
		}

		case ST_SORRY: {
			DoomCanvas_sorryState(doomCanvas);
			break;
		}
	}

	if ((doomCanvas->state == ST_PLAYING || doomCanvas->state == ST_AUTOMAP)) {
		if (doomCanvas->game->soundMonster) {
			// Sight Sound
			sound = EntityMonster_getSoundRnd(doomCanvas->game->soundMonster->monster, 1);
			Sound_playSound(doomCanvas->doomRpg->sound, sound, 0, 3);
			doomCanvas->game->soundMonster = NULL;
		}
	}

	// New Code Lines
	//{

	if (doomCanvas->state == ST_MENU) {
		if (doomCanvas->doomRpg->graphSetCliping != 0) {
			DoomRPG_setClipFalse(doomCanvas->doomRpg);
		}

		if (doomCanvas->doomRpg->menuSystem->menu >= MENU_INGAME) {
			Hud_drawTopBar(doomCanvas->hud);
			Hud_drawBottomBar(doomCanvas->hud);
			DoomCanvas_restoreSoftKeys(doomCanvas);
		}
	}

	if (doomCanvas->state == ST_BENCHMARK) {
		if (doomCanvas->doomRpg->graphSetCliping != 0) {
			DoomRPG_setClipFalse(doomCanvas->doomRpg);
		}

		Hud_drawTopBar(doomCanvas->hud);
		Hud_drawBottomBar(doomCanvas->hud);
		DoomCanvas_restoreSoftKeys(doomCanvas);
	}

	if (doomCanvas->restoreSoftKeys) {
		if (doomCanvas->doomRpg->graphSetCliping != 0) {
			DoomRPG_setClipFalse(doomCanvas->doomRpg);
		}
		DoomCanvas_restoreSoftKeys(doomCanvas);
	}
	//}


	doomCanvas->loopEnd = DoomRPG_GetUpTimeMS();

	if (doomCanvas->speeds && (doomCanvas->state != ST_MENU))
	{
		if (doomCanvas->doomRpg->graphSetCliping != 0) {
			DoomRPG_setClipFalse(doomCanvas->doomRpg);
		}

		int beforeRender = doomCanvas->beforeRender;
		if (doomCanvas->lastFrameTime == doomCanvas->time) {
			doomCanvas->beforeRender = 0;
			doomCanvas->lastLoopTime = (doomCanvas->loopEnd - doomCanvas->loopStart);
			doomCanvas->lastRenderTime = (doomCanvas->afterRender - beforeRender);
			doomCanvas->afterRender = 0;
		}
		sprintf_s(doomCanvas->printMsg, sizeof(doomCanvas->printMsg), "%d: %d / %d", doomCanvas->state, doomCanvas->lastRenderTime, doomCanvas->lastLoopTime);
		DoomCanvas_drawString2(doomCanvas, doomCanvas->printMsg, 0, doomCanvas->doomRpg->hud->statusTopBarHeight, 0, -1);
	}

	// New Code Lines
	//{
	if (doomCanvas->renderOnly && (doomCanvas->state == ST_PLAYING)) {
		if (doomCanvas->lastFrameTime == doomCanvas->time) {
			DoomRPG_flushGraphics(doomCanvas->doomRpg);
		}
	}
	else {
		DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
	
	//}
}

void DoomCanvas_saveState(DoomCanvas_t* doomCanvas, int i, char* text)
{
	doomCanvas->saveType = i;

	if (text == NULL) {
		doomCanvas->printMsg[0] = '\0';
	}
	else {
		strncpy_s(doomCanvas->printMsg, sizeof(doomCanvas->printMsg), text, sizeof(doomCanvas->printMsg));
	}

	DoomCanvas_setState(doomCanvas, ST_SAVING);
}

void DoomCanvas_setAnimFrames(DoomCanvas_t* doomCanvas, int i)
{
	doomCanvas->animFrames = i;
	doomCanvas->animPos = ((64 + doomCanvas->animFrames) - 1) / doomCanvas->animFrames;
	doomCanvas->animAngle = ((64 + doomCanvas->animFrames) - 1) / doomCanvas->animFrames;
}

void DoomCanvas_setState(DoomCanvas_t* doomCanvas, int stateNum)
{
	int oldState, len, width;
	char* msg;

	//printf("DoomCanvas_setState %d\n", stateNum);
	if (doomCanvas->state == ST_AUTOMAP) {
		doomCanvas->isUpdateView = true;

		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->clipRect.w, doomCanvas->clipRect.h);
		//DoomRPG_flushGraphics(doomCanvas->doomRpg);

		if (stateNum == ST_DIALOG || stateNum == ST_DIALOGPASSWORD) {
			doomCanvas->render->skipStretch = false;
			DoomCanvas_renderScene(doomCanvas, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle);
			doomCanvas->doomRpg->hud->isUpdate = true;
			Hud_drawTopBar(doomCanvas->doomRpg->hud);
			Hud_drawBottomBar(doomCanvas->doomRpg->hud);
		}
	}
	else if (doomCanvas->state == ST_MENU) {
		if (stateNum == ST_MENU) {
			if (doomCanvas->doomRpg->doomCanvas->unloadMedia)
			{
				DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
				DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->clipRect.w, doomCanvas->clipRect.h);
				//DoomRPG_flushGraphics(doomCanvas->doomRpg);
			}
		}
		else {
			Sound_stopSounds(doomCanvas->doomRpg->sound);
		}
	}

	oldState = doomCanvas->state;
	doomCanvas->state = stateNum;

	if (doomCanvas->state != oldState) {
		doomCanvas->restoreSoftKeys = false;
	}

	if (stateNum == ST_SORRY) {
		DoomRPG_createImage(doomCanvas->doomRpg, "c.bmp", false, &doomCanvas->imgSpaceBG);
	}
	else if (stateNum == ST_COMBAT) {
		if (oldState != ST_CAST) {
			DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
		}
		doomCanvas->combatDone = false;
	}
	else if (stateNum == ST_PLAYING) {
		if (!doomCanvas->doomRpg->game->monstersTurn) {
			DoomCanvas_drawSoftKeys(doomCanvas, "Menu", "Map");
		}
		doomCanvas->skipCheckState = true;
	}
	else if (stateNum == ST_DIALOG || stateNum == ST_DIALOGPASSWORD) {
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
		doomCanvas->passwordTime = 0;
		doomCanvas->numEvents = 0;
	}
	else if (stateNum == ST_DYING) {
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
		doomCanvas->deathTime = doomCanvas->time;
		doomCanvas->player->weapon = 0;
		doomCanvas->player->weapons = 0;
		Hud_drawBottomBar(doomCanvas->hud);
		return;
	}
	else if (stateNum == ST_EPILOGUE) {
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
		DoomCanvas_loadEpilogueText(doomCanvas);
	}
	else if (stateNum == ST_CREDITS) {
		DoomCanvas_initCredits(doomCanvas);
	}
	else if (stateNum == ST_INTRO) {
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);
		DoomCanvas_loadPrologueText(doomCanvas);
	}
	else if (stateNum == ST_LOADING || stateNum == ST_SAVING) {
		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_fillRect(doomCanvas->doomRpg, 0, 0, doomCanvas->clipRect.w, doomCanvas->softKeyY);

		len = SDL_strlen(justAMoment);
		width = ((len * 7) + 10);

		DoomRPG_setColor(doomCanvas->doomRpg, 0xffffff);
		DoomRPG_drawRect(doomCanvas->doomRpg, doomCanvas->SCR_CX - (width >> 1), doomCanvas->SCR_CY - 24, width, 48);

		if (doomCanvas->printMsg[0] == '\0') {
			msg = processing;
		}
		else {
			msg = doomCanvas->printMsg;
		}

		DoomCanvas_drawString1(doomCanvas, msg, doomCanvas->SCR_CX, doomCanvas->SCR_CY -12, 0x11);
		DoomCanvas_drawString1(doomCanvas, justAMoment, doomCanvas->SCR_CX, doomCanvas->SCR_CY, 0x11);
		DoomCanvas_drawSoftKeys(doomCanvas, NULL, NULL);

		DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
	else if (stateNum == ST_PARTICLE)
	{
		doomCanvas->skipCheckState = true;
	}
	else if (stateNum == ST_AUTOMAP) {
		if (doomCanvas->openDoorsCount > 0 || doomCanvas->game->activeSprites || doomCanvas->isUpdateView) {
			doomCanvas->f438d = true;
		}
		else {
			doomCanvas->f438d = false;
		}
		doomCanvas->automapDrawn = false;
		doomCanvas->staleView = true;
		doomCanvas->isUpdateView = true;
		DoomCanvas_drawSoftKeys(doomCanvas, "Menu", "Leave");
	}
	else if (stateNum == ST_CAST) {
		doomCanvas->castSeq = -1;
		doomCanvas->castTime = 0;
		doomCanvas->castEntity = NULL;
		doomCanvas->castEntityX = 0;
		doomCanvas->castEntityY = 28;
	}
	else if (stateNum == ST_MENU)
	{
		if (oldState == ST_PLAYING)
		{
			MenuSystem_playSound(doomCanvas->menuSystem);
			Sound_playSound(doomCanvas->doomRpg->sound, 5067, 0, 3);
		}

		if (doomCanvas->state != ST_MENU) {
			doomCanvas->numEvents = 0;
		}
	}
}

byte DoomCanvas_checkState(DoomCanvas_t* doomCanvas)
{

}

void DoomCanvas_invalidateRectAndUpdateView(DoomCanvas_t* doomCanvas)
{
	doomCanvas->staleView = true;
	doomCanvas->isUpdateView = true;
}

void DoomCanvas_updateViewTrue(DoomCanvas_t* doomCanvas)
{
	doomCanvas->isUpdateView = true;
}

void DoomCanvas_startDialog(DoomCanvas_t* doomCanvas, char* text, boolean dialogBackSoftKey)
{
	DoomCanvas_setState(doomCanvas, ST_DIALOG);
	DoomCanvas_prepareDialog(doomCanvas, text, dialogBackSoftKey);

	if (dialogBackSoftKey) {
		DoomCanvas_drawSoftKeys(doomCanvas, "Back", NULL);
	}
}

void DoomCanvas_startDialogPassword(DoomCanvas_t* doomCanvas, char* text)
{
	DoomCanvas_setState(doomCanvas, ST_DIALOGPASSWORD);
	DoomCanvas_prepareDialog(doomCanvas, text, false);
	doomCanvas->passCode[0] = '\0';
}

void DoomCanvas_startShake(DoomCanvas_t* doomCanvas, int i, int i2, int i3)
{
	if (!doomCanvas->slowBlit) {
		if (i2 != 0) {
			doomCanvas->shaketime = doomCanvas->time + i;
			doomCanvas->shakeVal = i2;
		}
		if (i3 != 0 && doomCanvas->vibrateEnabled) {
			DoomCanvas_vibrate(doomCanvas, i3);
		}
	}
	else if (doomCanvas->vibrateEnabled) {
		DoomCanvas_vibrate(doomCanvas, i3 == 0 ? i : i3);
	}
}


void DoomCanvas_startSpeedTest(DoomCanvas_t* doomCanvas, boolean b)
{
		doomCanvas->renderOnly = true;
		doomCanvas->benchmarkString = true;
		doomCanvas->st_count = 1;
		doomCanvas->renderAvgMs = 0;
		doomCanvas->horizAvgMs = 0;
		doomCanvas->loopAvgMs = 0;

		if (b) {
			doomCanvas->doomRpg->menuSystem->field_0xc58 = 4;
		}
		else {
			doomCanvas->animAngle = 4;
			doomCanvas->destAngle = doomCanvas->viewAngle;
			DoomCanvas_setState(doomCanvas, ST_BENCHMARK);
		}
}


void DoomCanvas_vibrate(DoomCanvas_t* doomCanvas, int i)
{
	DoomRPG_t* doomRpg;
	doomRpg = doomCanvas->doomRpg;

	if (i > 500) {
		i = 500;
	}

	// Use game controller
	if (sdlController.gGameController) {
		SDL_GameControllerRumble(sdlController.gGameController, 0, 0, 1); // Stop
		SDL_GameControllerRumble(sdlController.gGameController, 0xFFFF, 0xFFFF, i);
	}
	//Use haptics
	else if (sdlController.gJoyHaptic) {
		SDL_HapticRumbleStop(sdlController.gJoyHaptic);
		SDL_HapticRumblePlay(sdlController.gJoyHaptic, 1.0, i);
	}
}

void DoomCanvas_startup(DoomCanvas_t* doomCanvas)
{
	//Customs
	int viewSize;
	int frames;
	int map;
	//-------
	int width, height;
	int displayH, clipH, deltaH;
	int softKeyY;
	DoomRPG_t* doomRpg;

	doomRpg = doomCanvas->doomRpg;
	doomCanvas->render = doomRpg->render;
	doomCanvas->player = doomRpg->player;
	doomCanvas->game = doomRpg->game;
	doomCanvas->entityDef = doomRpg->entityDef;
	doomCanvas->combat = doomRpg->combat;
	doomCanvas->hud = doomRpg->hud;
	doomCanvas->menuSystem = doomRpg->menuSystem;
	doomCanvas->particleSystem = doomRpg->particleSystem;

	doomCanvas->displayRect.w = 0;
	doomCanvas->displayRect.h = 0;

	width = doomCanvas->clipRect.w;

	if (width & 1) {
		doomCanvas->clipRect.w = width - 1;
	}

	// DOOMRPG-VIEWSIZE
	viewSize = 0;

	if (viewSize == 0) {
		doomCanvas->displayRect.w = doomCanvas->clipRect.w;
		doomCanvas->displayRect.h = doomCanvas->clipRect.h;
	}
	else {
		doomCanvas->displayRect.w = viewSize;
		doomCanvas->displayRect.h = viewSize;
	}
	if (doomCanvas->displayRect.w < 0x80) {
		doomCanvas->displayRect.w = 0x80;
	}
	if (doomCanvas->displayRect.h < 0x80) {
		doomCanvas->displayRect.h = 0x80;
	}
	doomCanvas->softKeyY = doomCanvas->clipRect.h - 20;
	doomCanvas->largeStatus = (doomCanvas->displayRect.w >= 176) ? true : false;

	Hud_startup(doomCanvas->hud, doomCanvas->largeStatus);

	displayH = doomCanvas->displayRect.h;
	clipH = doomCanvas->clipRect.h;

	height = (displayH - (doomRpg->hud->statusBarHeight) - (doomRpg->hud->statusTopBarHeight));

	if (sdlVideo.displaySoftKeys) { // <- New line Code
		if (clipH >= 148) {
			deltaH = clipH - displayH;
			if (deltaH < 20) {
				height -= (20 - deltaH);
			}
			doomCanvas->displaySoftKeys = true;
		}
	}

	if (height & 1) {
		--height;
	}

	doomCanvas->displayRect.h = (doomRpg->hud->statusBarHeight + height + doomRpg->hud->statusTopBarHeight);
	doomCanvas->displayRect.x = (doomCanvas->clipRect.w - doomCanvas->displayRect.w + (doomCanvas->clipRect.w < doomCanvas->displayRect.w)) / 2;

	if (doomCanvas->displaySoftKeys) {
		clipH = doomCanvas->softKeyY;
	}

	doomCanvas->displayRect.y = ((clipH - doomCanvas->displayRect.h) / 2);
	doomCanvas->SCR_CY = doomCanvas->displayRect.h / 2;
	doomCanvas->SCR_CX = doomCanvas->displayRect.w / 2;
	doomCanvas->screenRect.x = doomCanvas->displayRect.x;
	doomCanvas->screenRect.y = doomCanvas->displayRect.y + doomRpg->hud->statusTopBarHeight;
	doomCanvas->screenRect.w = doomCanvas->displayRect.w;
	doomCanvas->screenRect.h = height;
#if 0
	printf("doomCanvas->displayRect.x %d\n", doomCanvas->displayRect.x);
	printf("doomCanvas->displayRect.y %d\n", doomCanvas->displayRect.y);
	printf("doomCanvas->displayRect.w %d\n", doomCanvas->displayRect.w);
	printf("doomCanvas->displayRect.h %d\n", doomCanvas->displayRect.h);
	printf("doomCanvas->clipRect.x %d\n", doomCanvas->clipRect.x);
	printf("doomCanvas->clipRect.y %d\n", doomCanvas->clipRect.y);
	printf("doomCanvas->clipRect.w %d\n", doomCanvas->clipRect.w);
	printf("doomCanvas->clipRect.h %d\n", doomCanvas->clipRect.h);
	printf("doomCanvas->screenRect.x %d\n", doomCanvas->screenRect.x);
	printf("doomCanvas->screenRect.y %d\n", doomCanvas->screenRect.y);
	printf("doomCanvas->screenRect.w %d\n", doomCanvas->screenRect.w);
	printf("doomCanvas->screenRect.h %d\n", doomCanvas->screenRect.h);
	printf("doomCanvas->SCR_CX %d\n", doomCanvas->SCR_CX);
	printf("doomCanvas->SCR_CY %d\n", doomCanvas->SCR_CY);
#endif

	Render_setup(doomCanvas->render, &doomCanvas->screenRect);

	softKeyY = doomCanvas->softKeyY - 1;
	if ((doomCanvas->displayRect.y + doomCanvas->displayRect.h) == softKeyY) {
		doomCanvas->softKeyY = softKeyY;
	}

	// DOOMRPG-FRAMES
	frames = 0;

	if (frames == 0) {
		frames = 4;
	}
	else {
		//frames = getNumFromString(resString);
		if (frames < 2) {
			frames = 2;
		}
		else if (frames >= 64) {
			frames = 64;
		}
	}
	DoomCanvas_setAnimFrames(doomCanvas, frames);

	// DOOMRPG-MAP
	map = 1;

	if (map == 0) {
		map = 0;
	}
	else {
		//map = getNumFromString(resString);
	}
	doomCanvas->startupMap = map;

	// DOOMRPG-SKIPINTRO
	doomCanvas->skipIntro = false;

	// DOOMRPG-SKIPSHAKEX
	doomCanvas->skipShakeX = false;

	// DOOMRPG-SNDFXONLY
	doomCanvas->sndFXOnly = false;

	DoomRPG_createImage(doomCanvas->doomRpg, "g.bmp", false, &doomCanvas->imgLegals);
	DoomRPG_createImage(doomCanvas->doomRpg, "a.bmp", true, &doomCanvas->imgFont);
	DoomRPG_createImage(doomCanvas->doomRpg, "larger_font.bmp", true, &doomCanvas->imgLargerFont);
	DoomRPG_createImage(doomCanvas->doomRpg, "b.bmp", true, &doomCanvas->imgMapCursor);
}

void DoomCanvas_uncoverAutomap(DoomCanvas_t* doomCanvas)
{
	int dx, dy, x, y;

	dx = doomCanvas->destX >> 6;
	dy = doomCanvas->destY >> 6;

	if (dx >= 0 && dx < 32 && dy >= 0 && dy < 32 && (doomCanvas->render->mapFlags[dx + (dy * 32)] & BIT_AM_VISITED) == 0)
	{
		for (y = dy - 1; y <= dy + 1; y++)
		{
			if (y >= 0 && y < 31) {
				for (x = dx - 1; x <= dx + 1; x++)
				{
					if (x >= 0 && x < 31 && (!(doomCanvas->render->mapFlags[(y * 32) + x] & BIT_AM_SECRET) || (x == dx && y == dy))) {
						doomCanvas->render->mapFlags[(y * 32) + x] |= BIT_AM_VISITED;
					}
				}
			}
		}
	}
}

void DoomCanvas_unloadMedia(DoomCanvas_t* doomCanvas)
{
	if (doomCanvas->unloadMedia)
	{
		doomCanvas->unloadMedia = false;
		Render_loadPalettes(doomCanvas->doomRpg->render);
		Render_freeRuntime(doomCanvas->doomRpg->render);
		Game_unloadMapData(doomCanvas->doomRpg->game);
		doomCanvas->doomRpg->render->enableViewRotation = false;
		Render_setup(doomCanvas->doomRpg->render, &doomCanvas->screenRect);
	}
}

void DoomCanvas_checkFacingEntity(DoomCanvas_t* doomCanvas)
{
	Entity_t* entity;
	Sprite_t* sprite;
	int x, y, i;

	x = doomCanvas->destX + (( doomCanvas->viewCos * 31) >> 16);
	y = doomCanvas->destY + ((-doomCanvas->viewSin * 31) >> 16);

	Game_trace(doomCanvas->game, x, y, x + (3 * doomCanvas->viewStepX), y + (3 * doomCanvas->viewStepY), NULL, 128767);

	doomCanvas->player->facingEntity = NULL;
	if (doomCanvas->game->numTraceEntities) {
		for (i = 0; i < doomCanvas->game->numTraceEntities; ++i) {
			if ((entity = doomCanvas->game->traceEntities[i])->def->eType == 14 ||
				(entity->info & 0x200000) || 
				(entity->info == 0) || 
				(sprite = &doomCanvas->render->mapSprites[(entity->info & 65535) - 1])->x >> 6 != x >> 6 || sprite->y >> 6 != y >> 6) {
				doomCanvas->player->facingEntity = entity;
				return;
			}
		}
	}
}

void DoomCanvas_updateLoadingBar(DoomCanvas_t* doomCanvas)
{
	int x, y, i;

	int currentTimeMillis = DoomRPG_GetTimeMS();
	if (currentTimeMillis - doomCanvas->lastPacifierUpdate >= 75) {
		doomCanvas->lastPacifierUpdate = currentTimeMillis;

		//int i = doomCanvas->SCR_CX - 17;
		//int i2 = doomCanvas->SCR_CY - 4;
		x = (sdlVideo.rendererW / 2) - 17;
		y = (sdlVideo.rendererH / 2) - 4;

		DoomRPG_setColor(doomCanvas->doomRpg, 0x000000);
		DoomRPG_clearGraphics(doomCanvas->doomRpg);

		DoomRPG_fillRect(doomCanvas->doomRpg, x, y, 35, 8);
		DoomRPG_setColor(doomCanvas->doomRpg, 0xAAAAAA);
		for (i = 0; i < 5; i++) {
			if (i == doomCanvas->fillRectIndex) {
				DoomRPG_fillRect(doomCanvas->doomRpg, x, y, 5, 8);
			}
			else {
				DoomRPG_drawRect(doomCanvas->doomRpg, x, y, 4, 7);
			}
			x += 7;
		}
		doomCanvas->fillRectIndex = (doomCanvas->fillRectIndex + 1) % 5;
		DoomRPG_flushGraphics(doomCanvas->doomRpg);
	}
}

boolean DoomCanvas_updatePlayerAnimDoors(DoomCanvas_t* doomCanvas)
{
	Line_t* lineDoor;
	Vertex_t* vert1, * vert2;
	int flags;

	if (doomCanvas->openDoorsCount == 0) {
		return false;
	}

	DoomCanvas_updateViewTrue(doomCanvas);

	boolean foundSecret = false;
	boolean changeMap = false;

	for (int i = 0; i < doomCanvas->openDoorsCount; i++) {

		lineDoor = doomCanvas->openDoors[i];

		flags = lineDoor->flags;
		vert1 = &lineDoor->vert1;
		vert2 = &lineDoor->vert2;

		if (lineDoor->texture == 7 && (flags & 64) != 0) {
			changeMap = true;
		}

		if ((flags & 4) != 0) {
			int i3 = (flags & 64) != 0 ? doomCanvas->animPos : -doomCanvas->animPos;
			if ((flags & 512) != 0) {
				vert1->y += i3;
				vert2->z -= i3;
			}
			else {
				vert1->x += i3;
				vert2->z -= i3;
			}
		}
		else {
			foundSecret = true;
			if (doomCanvas->animFrameCount >= doomCanvas->animFrames / 2) {
				int i4 = 64 / (doomCanvas->animFrames - (doomCanvas->animFrames / 2));
				if ((flags & 512) != 0) {
					if ((flags & 8) != 0) {
						vert1->y += i4;
						vert2->z -= i4;
					}
					else {
						vert2->y += i4;
						vert1->z -= i4;
					}
				}
				else if ((flags & 8) != 0) {
					vert2->x += i4;
					vert1->z -= i4;
				}
				else {
					vert1->x += i4;
					vert2->z -= i4;
				}
			}
			else if ((flags & 512) != 0) {
				if ((flags & 8) != 0) {
					vert1->x += doomCanvas->animPos;
					vert2->x += doomCanvas->animPos;
				}
				else {
					vert1->x -= doomCanvas->animPos;
					vert2->x -= doomCanvas->animPos;
				}
			}
			else if ((flags & 8) != 0) {
				vert1->y += doomCanvas->animPos;
				vert2->y += doomCanvas->animPos;
			}
			else {
				vert1->y -= doomCanvas->animPos;
				vert2->y -= doomCanvas->animPos;
			}
		}
	}

	++doomCanvas->animFrameCount;
	if (doomCanvas->animFrameCount != doomCanvas->animFrames) {
		return true;
	}

	if (foundSecret) {
		Hud_addMessage(doomCanvas->hud, "Found Secret!");
		Player_addXP(doomCanvas->player, 5);
		Sound_playSound(doomCanvas->doomRpg->sound, 5133, 0, 3);
	}

	if (changeMap) {
		Sound_playSound(doomCanvas->doomRpg->sound, 5068, 0, 3);
		Game_changeMap(doomCanvas->game);
	}

	DoomCanvas_checkFacingEntity(doomCanvas);
	doomCanvas->openDoorsCount = 0;
	doomCanvas->animFrameCount = 0;
	return false;
}

void DoomCanvas_updateView(DoomCanvas_t* doomCanvas)
{
	if (!doomCanvas->isUpdateView) {
		return;
	}

	doomCanvas->isUpdateView = false;

	boolean z = doomCanvas->viewX == doomCanvas->destX && doomCanvas->viewY == doomCanvas->destY;
	boolean z2 = doomCanvas->viewAngle == doomCanvas->destAngle;

	if (doomCanvas->viewX < doomCanvas->destX) {
		doomCanvas->viewX += doomCanvas->animPos;
		if (doomCanvas->viewX > doomCanvas->destX) {
			doomCanvas->viewX = doomCanvas->destX;
		}
	}
	else if (doomCanvas->viewX > doomCanvas->destX) {
		doomCanvas->viewX -= doomCanvas->animPos;
		if (doomCanvas->viewX < doomCanvas->destX) {
			doomCanvas->viewX = doomCanvas->destX;
		}
	}
	if (doomCanvas->viewY < doomCanvas->destY) {
		doomCanvas->viewY += doomCanvas->animPos;
		if (doomCanvas->viewY > doomCanvas->destY) {
			doomCanvas->viewY = doomCanvas->destY;
		}
	}
	else if (doomCanvas->viewY > doomCanvas->destY) {
		doomCanvas->viewY -= doomCanvas->animPos;
		if (doomCanvas->viewY < doomCanvas->destY) {
			doomCanvas->viewY = doomCanvas->destY;
		}
	}
	if (doomCanvas->viewAngle < doomCanvas->destAngle) {
		doomCanvas->viewAngle += doomCanvas->animAngle;
		if (doomCanvas->viewAngle > doomCanvas->destAngle) {
			doomCanvas->viewAngle = doomCanvas->destAngle;
		}
	}
	else if (doomCanvas->viewAngle > doomCanvas->destAngle) {
		doomCanvas->viewAngle -= doomCanvas->animAngle;
		if (doomCanvas->viewAngle < doomCanvas->destAngle) {
			doomCanvas->viewAngle = doomCanvas->destAngle;
		}
	}

	if (doomCanvas->state == ST_AUTOMAP) {
		doomCanvas->viewX = doomCanvas->destX;
		doomCanvas->viewY = doomCanvas->destY;
		doomCanvas->viewAngle = doomCanvas->destAngle;
	}

	DoomCanvas_renderScene(doomCanvas, doomCanvas->viewX, doomCanvas->viewY, doomCanvas->viewAngle);
		
	if (!z && doomCanvas->viewX == doomCanvas->destX && doomCanvas->viewY == doomCanvas->destY) {
		DoomCanvas_finishMovement(doomCanvas);
	}

	if (!z2 && doomCanvas->viewAngle == doomCanvas->destAngle) {
		DoomCanvas_finishRotation(doomCanvas);
	}

	if (!(doomCanvas->viewAngle == doomCanvas->destAngle && doomCanvas->viewY == doomCanvas->destY && doomCanvas->viewX == doomCanvas->destX)) {
		//updateViewTrue();
		doomCanvas->isUpdateView = true;
	}
	//invalidateRect();
	doomCanvas->staleView = true;
	doomCanvas->isUpdateView = true;
}

