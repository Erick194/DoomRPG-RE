
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Game.h"
#include "EntityDef.h"
#include "Hud.h"
#include "Player.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "Sound.h"
#include "SDL_Video.h"

MenuSystem_t* MenuSystem_init(MenuSystem_t* menuSystem, DoomRPG_t* doomRpg)
{
	printf("MenuSystem_init\n");

	if (menuSystem == NULL)
	{
		menuSystem = SDL_malloc(sizeof(MenuSystem_t));
		if (menuSystem == NULL) {
			return NULL;
		}
	}

	SDL_memset(menuSystem, 0, sizeof(MenuSystem_t));

	menuSystem->doomRpg = doomRpg;
	SDL_memset(menuSystem->items, 0, (sizeof(MenuItem_t)* MAX_MENUITEMS));
	menuSystem->numItems = 0;
	menuSystem->imgBG = NULL;
	menuSystem->oldMenu = -1;
	menuSystem->menu = MENU_NONE;
	menuSystem->selectedIndex = 0;
	menuSystem->scrollIndex = 0;
	menuSystem->type = 0;
	menuSystem->maxItems = 0;
	menuSystem->paintMenu = false;
	menuSystem->f749g = 0;
	menuSystem->cheatCombo = 0;
	menuSystem->digitCount = 0;
	menuSystem->field_0xc58 = 0x50;
	menuSystem->setBind = false; // new

	return menuSystem;
}

void MenuSystem_free(MenuSystem_t* menuSystem, boolean freePtr)
{
	DoomRPG_freeImage(menuSystem->doomRpg, &menuSystem->imgHand);
	DoomRPG_freeImage(menuSystem->doomRpg, &menuSystem->imgArrowUpDown);
	DoomRPG_freeImage(menuSystem->doomRpg, &menuSystem->imgLogo);
	menuSystem->imgBG = NULL;
	if (freePtr) {
		SDL_free(menuSystem);
	}
}

void MenuSystem_back(MenuSystem_t* menuSystem)
{
	if (menuSystem->oldMenu != -1) {

		if ((menuSystem->menu == MENU_BINDINGS) || (menuSystem->menu == MENU_INGAME_BINDINGS)) {
			// Apply changes
			SDL_memcpy(keyMapping, keyMappingTemp, sizeof(keyMapping));
			Game_saveConfig(menuSystem->doomRpg->game, 0);
		}

		MenuSystem_playSound(menuSystem);
		MenuSystem_setMenu(menuSystem, menuSystem->oldMenu);
	}
}

char* MenuSystem_buildDivider(MenuSystem_t* menuSystem, char* str)
{
	int i, idnx, len, cnt;

	menuSystem->stringBuffer[0] = '\0';
	len = SDL_strlen(str);

	cnt = (15 - (len + 2)) / 2;
	idnx = 0;
	for (i = 0; i < cnt; i++) {
		menuSystem->stringBuffer[idnx] = (char)0x80;
		idnx++;
	}
	menuSystem->stringBuffer[idnx] = ' ';

	strncpy(&menuSystem->stringBuffer[idnx+1], str, 32);

	idnx = idnx + 1 + len;
	menuSystem->stringBuffer[idnx] = ' ';
	for (i = 0; i < cnt; i++) {
		menuSystem->stringBuffer[idnx + 1] = (char)0x80;
		idnx++;
	}
	menuSystem->stringBuffer[idnx + 1] = '\0';

	return menuSystem->stringBuffer;
}

void MenuSystem_select(MenuSystem_t* menuSystem)
{
	int menu;
	menuSystem->cheatCombo = 0;
	menuSystem->digitCount = 0;
	Sound_playSound(menuSystem->doomRpg->sound, 5046, 0, 3);

	menu = Menu_select(menuSystem->doomRpg->menu, menuSystem->menu, menuSystem->selectedIndex);
	if (menuSystem->menu != menu) {
		MenuSystem_setMenu(menuSystem, menu);
	}
}

boolean MenuSystem_checkMenu(MenuSystem_t* menuSystem)
{
	return (menuSystem->menu >= MENU_INGAME_BEGIN) ? false : true;
}

boolean MenuSystem_enterDigit(MenuSystem_t* menuSystem, int n)
{
	int i;
	int multNum;

	// Nuevo: Evita que se inserten codigos en la pantalla de nivel completado
	// New: Prevents codes from being inserted into the completed level screen
	if (menuSystem->menu == MENU_MAP_STATS) {
		return false;
	}

	multNum = 1;
	for (i = 0; i < menuSystem->digitCount; i++) {
		multNum *= 10;
	}
	menuSystem->cheatCombo += (multNum * n);

	if (menuSystem->cheatCombo == 6663) {
		menuSystem->cheatCombo = 0;
		menuSystem->digitCount = 0;
		menuSystem->selectedIndex = 0;
		menuSystem->scrollIndex = 0;
		MenuSystem_setMenu(menuSystem, MENU_DEBUG);
		return true;
	}
	else if (menuSystem->cheatCombo == 6633) { // New cheat From BREW Version
		menuSystem->cheatCombo = 0;
		menuSystem->digitCount = 0;
		if (menuSystem->menu < MENU_INGAME) {
			if (menuSystem->menu == MENU_MAIN) {
				DoomCanvas_startSpeedTest(menuSystem->doomRpg->doomCanvas, true);
			}
		}
		else {
			MenuSystem_setMenu(menuSystem, MENU_NONE);
			DoomCanvas_startSpeedTest(menuSystem->doomRpg->doomCanvas, false);
		}
		
		return true;
	}
	else if (menuSystem->cheatCombo == 92634) {
		menuSystem->cheatCombo = 0;
		menuSystem->digitCount = 0;
		Hud_addMessage(menuSystem->doomRpg->hud, "Health & Armor!");
		CombatEntity_setMaxHealth(&menuSystem->doomRpg->player->ce, 99);
		CombatEntity_setHealth(&menuSystem->doomRpg->player->ce, 99);
		CombatEntity_setMaxArmor(&menuSystem->doomRpg->player->ce, 99);
		CombatEntity_setArmor(&menuSystem->doomRpg->player->ce, 99);
		return true;
	}
	else if (menuSystem->cheatCombo == 2334) {
		menuSystem->cheatCombo = 0;
		menuSystem->digitCount = 0;
		Hud_addMessage(menuSystem->doomRpg->hud, "Weapons & Ammo!");
		menuSystem->doomRpg->player->weapons = 2559;
		for (i = 0; i < 6; i++) {
			Player_addAmmo(menuSystem->doomRpg->player, i, 99);
		}
		return true;
	}
	else {
		menuSystem->digitCount++;
		return false;
	}
}

void MenuSystem_moveDir(MenuSystem_t* menuSystem, int i)
{
	if (menuSystem->type == 5) {
		if (i < 0 && menuSystem->scrollIndex > 0) {
			menuSystem->scrollIndex += i;
		}
		else if (i > 0 && menuSystem->scrollIndex < menuSystem->numItems - menuSystem->maxItems) {
			menuSystem->scrollIndex += i;
		}
		menuSystem->selectedIndex = menuSystem->scrollIndex;
	}
	else
	{
		do {
			menuSystem->selectedIndex += i;
			if (menuSystem->selectedIndex >= menuSystem->numItems || menuSystem->selectedIndex < 0) {
				if (menuSystem->selectedIndex < 0) {
					menuSystem->selectedIndex = menuSystem->numItems - 1;
				}
				else {
					menuSystem->selectedIndex = 0;
				}
				while (menuSystem->items[menuSystem->selectedIndex].textField[0] == '\0' || (menuSystem->items[menuSystem->selectedIndex].flags & 0x1) != 0x0) {
					menuSystem->selectedIndex += i;
				}
				break;
			}
		} while (menuSystem->items[menuSystem->selectedIndex].textField[0] == '\0' || (menuSystem->items[menuSystem->selectedIndex].flags & 0x1) != 0x0);


		if (menuSystem->maxItems != 0 && i < 0) {
			if (menuSystem->selectedIndex - menuSystem->maxItems + 1 > menuSystem->scrollIndex) {
				menuSystem->scrollIndex = menuSystem->selectedIndex - menuSystem->maxItems + 1;
			}
			else if (menuSystem->selectedIndex < menuSystem->scrollIndex) {
				menuSystem->scrollIndex = menuSystem->selectedIndex;
			}
		}
		else if (menuSystem->maxItems != 0) {
			if (menuSystem->selectedIndex > menuSystem->scrollIndex + menuSystem->maxItems - 1) {
				menuSystem->scrollIndex = menuSystem->selectedIndex - menuSystem->maxItems + 1;
			}
			else if (menuSystem->scrollIndex > menuSystem->selectedIndex) {
				menuSystem->scrollIndex = menuSystem->selectedIndex;
			}
		}
	}
}

void MenuSystem_paint(MenuSystem_t* menuSystem)
{
	int i, i2;

	DoomRPG_t* doomRpg;
	DoomCanvas_t* doomCanvas;
	MenuItem_t *mItem;
	EntityDef_t* ent;
	char textField[32]; // original 18
	char textField2[16]; // original 8

	doomRpg = menuSystem->doomRpg;
	doomCanvas = doomRpg->doomCanvas;

	if (MenuSystem_checkMenu(menuSystem) || (menuSystem->menu != 0 && menuSystem->paintMenu))
	{
		//menuSystem->paintMenu = false;
		DoomRPG_setColor(doomRpg, 0x000000);
		DoomRPG_setFontColor(menuSystem->doomRpg, 0xffffffff);

		i = 0;
		i2 = 0;

		if (menuSystem->menu >= MENU_INGAME) {
			i2 = doomRpg->hud->statusTopBarHeight + ((doomCanvas->screenRect.h - (menuSystem->maxItems * 12)) / 2);
			DoomRPG_fillRect(doomRpg, 0, doomRpg->hud->statusTopBarHeight, doomCanvas->displayRect.w, doomCanvas->displayRect.h);

			if (!(menuSystem->menu == MENU_ITEMS_CONFIRM || menuSystem->menu == MENU_STORE_BUY)) {
				DoomRPG_setColor(doomRpg, 0x050A4A);

				int i3 = (doomCanvas->displayRect.h % 8) >> 1;
				int i4 = (doomCanvas->displayRect.w % 8) >> 1;
				for (i4; i4 < doomCanvas->displayRect.w; i4 += 8) {
					DoomRPG_drawLine(doomRpg, i4, doomRpg->hud->statusTopBarHeight, i4, (doomCanvas->displayRect.h - doomRpg->hud->statusBarHeight) - 1);
				}

				int i5 = doomRpg->hud->statusTopBarHeight + i3;
				for (i5; i5 < doomCanvas->displayRect.h - 20; i5 += 8) {
					DoomRPG_drawLine(doomRpg, 0, i5, doomCanvas->displayRect.w, i5);
				}
				DoomRPG_setColor(doomRpg, 0x000000);
			}
		}
		else {
			DoomRPG_fillRect(doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);

			if (MenuSystem_checkMenu(menuSystem)) {
				doomCanvas->viewAngle = (doomCanvas->time / menuSystem->field_0xc58) & 0xff;

				Render_render(menuSystem->doomRpg->render, doomCanvas->viewX, doomCanvas->viewY, 36, doomCanvas->viewAngle);

				// Port new line
				DoomCanvas_drawRGB(doomCanvas);
			}

			if (doomCanvas->benchmarkString) {
				int AvgMs1 = (doomCanvas->renderAvgMs * 100) / doomCanvas->st_count;
				int AvgMs2 = doomCanvas->renderAvgMs / doomCanvas->st_count;
				SDL_snprintf(doomCanvas->printMsg, sizeof(doomCanvas->printMsg), "Avg: %d.%dms", AvgMs2, AvgMs1 - (AvgMs2 * 100));
				DoomCanvas_drawString2(doomCanvas, doomCanvas->printMsg, 0, doomCanvas->displayRect.h - 12, 0, -1);
			}

			if (menuSystem->imgBG) {
				DoomCanvas_drawImage(doomCanvas, menuSystem->imgBG, doomCanvas->SCR_CX, 0, 17);
			}

			menuSystem->maxItems = doomCanvas->displayRect.h / 12;
		}

		if (menuSystem->menu == MENU_ITEMS_CONFIRM || (menuSystem->menu == MENU_STORE_BUY && (doomRpg->menu->f701a != (byte)-1))) {
			SDL_memset(doomRpg->render->framebuffer, 0, doomRpg->render->pitch * sdlVideo.rendererH);

			ent = EntityDef_find(menuSystem->doomRpg->entityDef, (byte)4, (byte)(25 + doomRpg->menu->f701a));

			int i9 = ((0x4000 * ((doomRpg->render->screenWidth << 16) / 0x8000)) + 65280) >> 16;
			Render_draw2DSprite(doomRpg->render, ent->tileIndex, 0, 0, (doomRpg->render->screenHeight - i9) - (i9 / 10), 0, 0);
			DoomCanvas_drawRGB(doomCanvas);
			DoomCanvas_updateViewTrue(doomCanvas);
		}

		if (menuSystem->type == 2) {
			i = 90;
		}
		else if (menuSystem->type == 3) {
			i = 50;
		}
		else if (menuSystem->type == 1) { // MENUTYPE_LIST
			i = 12;
		}
		else if (menuSystem->type == 5) { // MENUTYPE_HELP
			i = 0;
			i2 = 0;
		}
		else if (menuSystem->type == 6) { // MENUTYPE_VCENTER
			i = 0;
			i2 = doomRpg->hud->statusTopBarHeight + (doomCanvas->screenRect.h - (menuSystem->numItems * 12)) / 2;
		}
		else if (menuSystem->type == 4) { // MENUTYPE_MAIN
			i = 40;
			i2 = 80;
		}
		else if (menuSystem->type == 7) { // MENUTYPE_MAIN2 New Type
			i = 12;
			i2 = 80;
			menuSystem->maxItems = (doomCanvas->displayRect.h - i2) / 12;
		}

		int i101 = doomCanvas->SCR_CX + i - 64;
		if (menuSystem->maxItems > 0 && menuSystem->numItems > menuSystem->maxItems) {
			DoomCanvas_drawScrollBar(doomCanvas, i2, menuSystem->maxItems * 12, menuSystem->scrollIndex, menuSystem->scrollIndex + menuSystem->maxItems, menuSystem->numItems);
		}

		int local_34 = 9;
		int local_38 = 7;
		int local_28 = 12;
		boolean isLargerFont = false;

		if ((menuSystem->menu > MENU_NONE) && (menuSystem->menu < MENU_MAIN_OPTIONS)) {
			if ((menuSystem->menu != MENU_MAIN_HELP_ABOUT) && menuSystem->doomRpg->doomCanvas->largeStatus) {
				local_34 = 13;
				local_38 = 10;
				local_28 = 17;
				isLargerFont = true;
			}
		}

		int local_2c = local_28 >> 1;

		for (int i11 = menuSystem->scrollIndex; i11 < menuSystem->numItems; i11++) {
			int i10 = i101;
			mItem = &menuSystem->items[i11];

			SDL_memcpy(textField, mItem->textField, sizeof(mItem->textField));
			SDL_memcpy(textField2, mItem->textField2, sizeof(mItem->textField2));

			if (textField != '\0')
			{
				if ((mItem->flags & 8) && ((menuSystem->menu == MENU_BINDINGS) || (menuSystem->menu == MENU_INGAME_BINDINGS))) { // New Flag
					if (doomCanvas->time > menuSystem->nextMsgTime) {
						menuSystem->nextMsgTime = doomCanvas->time + 1000;
						menuSystem->nextMsg++;
					}

					int j;
					for (j = 0; j < KEYBINDS_MAX; j++)
					{
						if (keyMappingTemp[mItem->action].keyBinds[j] == -1) {
							break;
						}
					}

					if (j == 0) {
						SDL_snprintf(textField, sizeof(textField), "Unbound");
					}
					else {
						if (keyMappingTemp[mItem->action].keyBinds[menuSystem->nextMsg % j] & IS_CONTROLLER_BUTTON) {
							SDL_snprintf(textField, sizeof(textField), "%s", SDL_GameControllerGetNameButton(keyMappingTemp[mItem->action].keyBinds[menuSystem->nextMsg % j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON)));
						}
						else if (keyMappingTemp[mItem->action].keyBinds[menuSystem->nextMsg % j] & IS_MOUSE_BUTTON) {
							SDL_snprintf(textField, sizeof(textField), "%s", SDL_MouseGetNameButton(keyMappingTemp[mItem->action].keyBinds[menuSystem->nextMsg % j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON)));
						}
						else{
							SDL_snprintf(textField, sizeof(textField), "%s", SDL_GetScancodeName(keyMappingTemp[mItem->action].keyBinds[menuSystem->nextMsg % j]));
						}
					}
					DoomRPG_setFontColor(menuSystem->doomRpg, 0xff80C0FF);
				}
				else {
					DoomRPG_setFontColor(menuSystem->doomRpg, 0xffffffff);
				}

				//if ((mItem->flags & 1) != 0) {
				//	DoomRPG_setColor(menuSystem->doomRpg, -3355444);
				//}
				//else {
				//	DoomRPG_setColor(menuSystem->doomRpg, -1);
				//}

				if ((textField[0] != '\0') && (mItem->flags & 2) != 0) {
					int length = (((strlen(textField) << 16) >> 9) * local_38) >> 8;
					i10 = (menuSystem->maxItems == 0 || menuSystem->numItems <= menuSystem->maxItems) ? doomCanvas->SCR_CX - length : (doomCanvas->SCR_CX - 6) - length;
				}
				else if (textField2[0] != '\0') {
					int i12 = (doomCanvas->SCR_CX + 64) - 2;

					if (menuSystem->maxItems != 0) {
						i12 -= local_34;
					}

					DoomCanvas_drawFont(doomCanvas, textField2, i12, i2, 9, 0, -1, isLargerFont);
				}

				if (menuSystem->type != 5 && i11 == menuSystem->selectedIndex) {
					DoomCanvas_drawImage(doomCanvas, &menuSystem->imgHand, i10, i2 + local_2c, 40);
					i10 += 2;
				}
				if (textField[0] != '\0') {
					DoomCanvas_drawFont(doomCanvas, textField, i10, i2, 0, 0, -1, isLargerFont);
				}
			}

			i2 += local_28;
			if (menuSystem->maxItems != 0 && i11 == (menuSystem->scrollIndex + menuSystem->maxItems) - 1) {
				break;
			}
		}

		if (menuSystem->setBind) {
			SDL_SetRenderDrawBlendMode(sdlVideo.renderer, SDL_BLENDMODE_BLEND);
			DoomRPG_setColor(menuSystem->doomRpg, 0xC0000000);
			DoomRPG_fillRect(menuSystem->doomRpg, 0, 0, doomCanvas->displayRect.w, doomCanvas->displayRect.h);
			SDL_SetRenderDrawBlendMode(sdlVideo.renderer, SDL_BLENDMODE_NONE);


			isLargerFont = ((doomCanvas->displayRect.w > 128) && menuSystem->doomRpg->doomCanvas->largeStatus) ? true : false;
			local_28 = 12;
			if (isLargerFont) {
				local_28 = 17;
			}

			DoomRPG_setFontColor(menuSystem->doomRpg, 0xffffffff);
			DoomCanvas_drawFont(doomCanvas, "Press New Key For", doomCanvas->SCR_CX, doomCanvas->SCR_CY, 16|32, 0, -1, isLargerFont);
			SDL_snprintf(textField, sizeof(textField), "%s", menuSystem->items[menuSystem->bindIndx].textField);
			textField[strlen(textField) - 1] = '\0'; // remove :

			DoomRPG_setFontColor(menuSystem->doomRpg, 0xff80C0FF);
			DoomCanvas_drawFont(doomCanvas, textField, doomCanvas->SCR_CX, doomCanvas->SCR_CY + local_28, 16 | 32, 0, -1, isLargerFont);
			DoomRPG_setFontColor(menuSystem->doomRpg, 0xffffffff);
			
		}

		DoomRPG_setFontColor(menuSystem->doomRpg, 0xffffffff);
	}
}

void MenuSystem_scrollDown(MenuSystem_t* menuSystem)
{
	MenuSystem_moveDir(menuSystem, 1);
	menuSystem->paintMenu = true;
}

void MenuSystem_scrollPageDown(MenuSystem_t* menuSystem)
{
	// Original Code
	//for (int i = 0; i < menuSystem->maxItems && (menuSystem->numItems + -1 != menuSystem->selectedIndex); i++) {
	//	MenuSystem_moveDir(menuSystem, 1);
	//}

	// Original Code From Doom II RPG
	int selectedIndex = menuSystem->selectedIndex;
	for (int n = 0; n < menuSystem->maxItems && menuSystem->selectedIndex != menuSystem->numItems - 1; ++n) {
		MenuSystem_moveDir(menuSystem, 1);
		if (menuSystem->selectedIndex < selectedIndex) {
			MenuSystem_moveDir(menuSystem, -1);
			break;
		}
	}
	menuSystem->paintMenu = true;
}

void MenuSystem_scrollPageUp(MenuSystem_t* menuSystem)
{
	// Original Code
	//for (int i = 0; i < menuSystem->maxItems && (menuSystem->selectedIndex != 0); i++) {
	//	MenuSystem_moveDir(menuSystem, -1);
	//}

	// Original Code From Doom II RPG
	int selectedIndex = menuSystem->selectedIndex;
	for (int n = 0; n < menuSystem->maxItems && menuSystem->selectedIndex != 0; ++n) {
		MenuSystem_moveDir(menuSystem, -1);
		if (menuSystem->selectedIndex >= selectedIndex) {
			MenuSystem_moveDir(menuSystem, 1);
			break;
		}
	}
	menuSystem->paintMenu = true;
}

void MenuSystem_scrollUp(MenuSystem_t* menuSystem)
{
	MenuSystem_moveDir(menuSystem, -1);
	menuSystem->paintMenu = true;
}

void MenuSystem_setMenu(MenuSystem_t* menuSystem, int menu)
{
	MenuItem_t* item;
	menuSystem->cheatCombo = 0;
	menuSystem->digitCount = 0;

	int v6, v11, iVar2;


	v6 = menu < MENU_NONE;
	if (menu) {
		v6 = menu < MENU_INGAME_BEGIN;
	}

	if (!v6 || (menuSystem->menu != MENU_NONE && menuSystem->menu <= MENU_INGAME_BEGIN))
	{
		if (menu > MENU_INGAME_BEGIN || !menu)
		{
			v11 = menuSystem->menu < MENU_NONE;
			if (menuSystem->menu) {
				v11 = menuSystem->menu < MENU_INGAME_BEGIN;
			}

			if (v11)
			{
				if (menuSystem->menu < MENU_MAP_STATS)
				{
					DoomRPG_setColor(menuSystem->doomRpg, 0x000000);
					DoomRPG_fillRect(menuSystem->doomRpg, 0, 0, menuSystem->doomRpg->doomCanvas->clipRect.w, menuSystem->doomRpg->doomCanvas->clipRect.h);
					DoomRPG_flushGraphics(menuSystem->doomRpg);
					DoomCanvas_unloadMedia(menuSystem->doomRpg->doomCanvas);
				}
				else {
					Render_loadPalettes(menuSystem->doomRpg->render);
					menuSystem->doomRpg->render->skipViewNudge = false;
					Render_setup(menuSystem->doomRpg->render, &menuSystem->doomRpg->doomCanvas->screenRect);
				}
			}
		}
	}
	else if (menu < MENU_MAP_STATS) {
		DoomCanvas_LoadMenuMap(menuSystem->doomRpg->doomCanvas);
	}
	else {
		Render_setGrayPalettes(menuSystem->doomRpg->render);
		Render_setup(menuSystem->doomRpg->render, &menuSystem->doomRpg->doomCanvas->displayRect);
		iVar2 = menuSystem->doomRpg->render->mapCameraSpawnIndex;
		if (iVar2 != 0) {
			menuSystem->doomRpg->doomCanvas->viewX = ((iVar2 % 32) << 6) + 32;
			menuSystem->doomRpg->doomCanvas->viewY = ((iVar2 / 32) << 6) + 32;
		}
		menuSystem->doomRpg->render->skipViewNudge = true;
	}

	menuSystem->menu = menu;
	if (menu != 0) {
		Menu_initMenu(menuSystem->doomRpg->menu, menu);
		menuSystem->maxItems = menuSystem->doomRpg->doomCanvas->screenRect.h / 12;

		item = &menuSystem->items[menuSystem->selectedIndex];
		if (item->textField[0] == '\0' || (item->flags & 1) != 0) {
			MenuSystem_moveDir(menuSystem, 1);
		}
		DoomCanvas_setState(menuSystem->doomRpg->doomCanvas, ST_MENU);
		menuSystem->paintMenu = true;
	}
	else {
		menuSystem->numItems = 0;
		menuSystem->doomRpg->hud->logMessage[0] = '\0';
		if (menuSystem->doomRpg->doomCanvas->state == ST_MENU) {
			DoomCanvas_setState(menuSystem->doomRpg->doomCanvas, ST_PLAYING);
			Player_selectWeapon(menuSystem->doomRpg->player, menuSystem->doomRpg->player->weapon);
		}
		DoomCanvas_invalidateRectAndUpdateView(menuSystem->doomRpg->doomCanvas);
	}
}

void MenuSystem_playSound(MenuSystem_t* menuSystem)
{
	Sound_playSound(menuSystem->doomRpg->sound, 5042, 0, 3);
}

void MenuSystem_startup(MenuSystem_t* menuSystem) {
	DoomRPG_createImage(menuSystem->doomRpg, "p.bmp", true, &menuSystem->imgHand);
	DoomRPG_createImage(menuSystem->doomRpg, "q.bmp", false, &menuSystem->imgArrowUpDown);
	DoomRPG_createImage(menuSystem->doomRpg, "j.bmp", true, &menuSystem->imgLogo);
	menuSystem->imgBG = NULL;
}