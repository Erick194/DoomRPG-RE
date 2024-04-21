
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Game.h"
#include "Menu.h"
#include "MenuItem.h"
#include "MenuSystem.h"
#include "Hud.h"
#include "Player.h"
#include "Combat.h"
#include "EntityDef.h"
#include "Sound.h"
#include "SDL_Video.h"

#define MAXSTORES 4
#define MAXSTORELINES 17 * 2

#define STORE_NAME_TAG 0
#define STORE_PRICE_TAG 17

int vendingMenuTable[MAXSTORELINES * MAXSTORES] = {
	// Store 0
	 0,  1, 2,  3,  6,  7,  8,  9, 11, 10, 12, 13, 14, 15, 16,  0,  0, // Name tags
	-1, -1, 8, 35, -1,  5,  5, 10, 10, 15, -1, 15, 15, 20, 20,  0,  0, // Price tags

	// Store 1
	 0,  1, 2,  6,  8,  7,  9, 12, 13, 14, 15, 16,  0,  0,  0,  0,  0, // Name tags
	-1, -1, 6, -1,  3,  4,  6, -1, 15, 15, 20, 20,  0,  0,  0,  0,  0, // Price tags

	// Store 2
	 0,  1, 2,  3,  4,  6,  7,  8, 11,  9, 10, 12, 13, 14, 15, 16,  0, // Name tags
	-1, -1, 6, 25, 50, -1,  3,  4,  6,  8, 12, -1, 15, 15, 20, 20,  0, // Price tags

	// Store 3
	 0,  1, 2,  3,  4,  5,  6,  7,  8,  9, 11, 10, 12, 13, 14, 15, 16, // Name tags
	-1, -1, 4, 15, 30, 40, -1,  2,  2,  4,  5,  8, -1,  8,  8, 10, 10  // Price tags
};

void Menu_LoadHelpResource(Menu_t* menu)
{
	byte* fData;
	int dataPos, i, j;
	byte c;
	char textLine[32];

	fData = DoomRPG_fileOpenRead(menu->doomRpg, "/help.txt");

	dataPos = 3;
	for (i = 0; i < (fData[1] + (fData[0] * 10) - 528); i++) {
		textLine[0] = 0;
		j = 0;
		while (fData[dataPos] != 0) {
			c = fData[dataPos];
			dataPos++;

			if (c == 10) {
				if (j >= 18) { j = 17; } // Previene desbordaminetos de memoria
				textLine[j] = '\0';
				MenuItem_Set(&menu->doomRpg->menuSystem->items[menu->doomRpg->menuSystem->numItems], textLine, 0, 0);
				menu->doomRpg->menuSystem->numItems++;
				break;
			}

			if (c == '~') {
				textLine[j] = 0x80;
			}
			else if (c == '\r') {
				textLine[j] = 0;
			}
			else {
				textLine[j] = c;
			}

			j++;
		}
	}

	SDL_free(fData);
}

void Menu_setNotes(Menu_t* menu)
{
	MenuSystem_t* menuSystem;
	MenuItem_t* item;
	char* tmpStr;
	char* nbStr;
	char text[32];

	menuSystem = menu->doomRpg->menuSystem;

	nbStr = menu->doomRpg->player->NotebookString;
	SDL_snprintf(text, sizeof(text), "%s notes...", menu->doomRpg->render->mapName);

	MenuItem_Set(&menuSystem->items[menuSystem->numItems++], text, 0, 0);
	MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);

	
	while (tmpStr = SDL_strchr(nbStr, 0x7c), tmpStr != NULL) {

		if(tmpStr == nbStr) {
			item = &menuSystem->items[menuSystem->numItems++];
			nbStr = (char*)0x0;
		}
		else {
			strncpy(text, nbStr, (int)(tmpStr - (int)nbStr));
			text[(int)(tmpStr -(int)nbStr)] = '\0';
			item = &menuSystem->items[menuSystem->numItems++];
			nbStr = text;
		}

		MenuItem_Set(item, nbStr, 0, 0);
		nbStr = tmpStr + 1;
	}
}

void Menu_setStore(Menu_t* menu)
{
	MenuSystem_t* menuSystem;
	MenuItem_t items[17];
	char text[32];
	int i,* vTbl, vNtag;

	menuSystem = menu->doomRpg->menuSystem;

	MenuItem_Set(&items[0], "Back", 0, 0);

	MenuItem_Set(&items[1], MenuSystem_buildDivider(menuSystem, "ITEMS"), 3, 0);
	MenuItem_Set2(&items[2], "Sm Medkit", NULL, 0, 0x190001);
	MenuItem_Set2(&items[3], "Lg Medkit", NULL, 0, 0x1A0001);
	MenuItem_Set2(&items[4], "Soul Sphere", NULL, 0, 0x1B0001);
	MenuItem_Set2(&items[5], "Berserker", NULL, 0, 0x1C0001);

	MenuItem_Set(&items[6], MenuSystem_buildDivider(menuSystem, "AMMO"), 3, 0);
	MenuItem_Set2(&items[7], "10x Hal. Cans", NULL, 0, 2);
	MenuItem_Set2(&items[8], "10x Bullets", NULL, 0, 0x10002);
	MenuItem_Set2(&items[9], "10x Shells", NULL, 0, 0x20002);
	MenuItem_Set2(&items[10], "10x Cells", NULL, 0, 0x40002);
	MenuItem_Set2(&items[11], "3x Rockets", NULL, 0, 0x30002);

	MenuItem_Set(&items[12], MenuSystem_buildDivider(menuSystem, "STATS"), 3, 0);
	MenuItem_Set2(&items[13], "+1 Accuracy", NULL, 0, 0x40003);
	MenuItem_Set2(&items[14], "+1 Agility", NULL, 0, 0x20003);
	MenuItem_Set2(&items[15], "+1 Strength", NULL, 0, 0x30003);
	MenuItem_Set2(&items[16], "+1 Defense", NULL, 0, 0x10003);


	vTbl = &vendingMenuTable[menuSystem->f749g * MAXSTORELINES];
	for (i = 0; i < 17; ++i) {
		vNtag = vTbl[i + STORE_NAME_TAG];
		if ((i > 1) && (vNtag == 0)) {
			break;
		}

		if (vTbl[i + STORE_PRICE_TAG] > 0) {
			SDL_snprintf(text, sizeof(text), "%d", vTbl[i + STORE_PRICE_TAG]);
		}
		else {
			text[0] = '\0';
		}

		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], items[vNtag].textField, text, items[vNtag].flags, items[vNtag].action);
	}
}

void Menu_setYesNo(Menu_t* menu, char* str)
{
	MenuSystem_t* menuSystem;

	menuSystem = menu->doomRpg->menuSystem;
	MenuItem_Set(&menuSystem->items[menuSystem->numItems++], str, 3, 0);
	MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 3, 0);
	MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Yes", NULL, 2, 1);
	MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "No ", NULL, 2, 0);
}

void Menu_textVolume(Menu_t* menu, int volume)
{
	int itemId;

	//itemId = (menu->doomRpg->doomCanvas->sndFXOnly) ? 2 : 3; // Old
	itemId = (menu->doomRpg->doomCanvas->sndFXOnly) ? 1 : 2;

	SDL_snprintf(menu->doomRpg->menuSystem->items[itemId].textField2, 
		sizeof(menu->doomRpg->menuSystem->items[itemId].textField2), "%d%%", (volume * 100) / 100);

	menu->doomRpg->menuSystem->paintMenu = true;
}

Menu_t* Menu_init(Menu_t* menu, DoomRPG_t* doomRpg)
{
	printf("Menu_init\n");

	if (menu == NULL)
	{
		menu = SDL_malloc(sizeof(Menu_t));
		if (menu == NULL) {
			return NULL;
		}
	}
	SDL_memset(menu, 0, sizeof(Menu_t));

	menu->f714f = 0;
	menu->doomRpg = doomRpg;
	menu->f715g = 0;

	return menu;
}

void Menu_fillStatus(Menu_t* menu, int i, int i2, int i3)
{
	MenuSystem_t* menuSystem;
	Player_t* player;
	char* textDivider, text[32];
	int curTime, cnt;
	int current, total;

	curTime = DoomRPG_GetUpTimeMS() - menu->doomRpg->player->time;
	menuSystem = menu->doomRpg->menuSystem;
	player = menu->doomRpg->player;

	if (i2 != 0) {
		textDivider = MenuSystem_buildDivider(menuSystem, "PLAYER");
		MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

		SDL_snprintf(text, sizeof(text), "%d/%d", CombatEntity_getHealth(&player->ce), CombatEntity_getMaxHealth(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Health:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d/%d", CombatEntity_getArmor(&player->ce), CombatEntity_getMaxArmor(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Armor:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->credits);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Credits:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->level);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Level:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d/%d", player->currentXP, player->nextLevelXP);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "XP:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", CombatEntity_getDefense(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Defense:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", CombatEntity_getStrength(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Strength:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", CombatEntity_getAgility(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Agility:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", CombatEntity_getAccuracy(&player->ce));
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Accuracy:", text, 0, 0);
	}

	if (i != 0) {
		textDivider = MenuSystem_buildDivider(menuSystem, "THIS SECTOR");
		MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

		Player_formatTime(player, text, sizeof(text), curTime);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Time:", text, 0, 0);

		Player_fillSecretStats(player, &current, &total);
		SDL_snprintf(text, sizeof(text), "%d/%d", current, total);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Secrets:", text, 0, 0);


		Player_fillMonsterStats(player, &current, &total);
		SDL_snprintf(text, sizeof(text), "%d/%d", current, total);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Monsters:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->moves);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Moves:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->xpGained);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "XP Gained:", text, 0, 0);
	}

	if (i3 != 0) {
		textDivider = MenuSystem_buildDivider(menuSystem, "OVERALL");
		MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

		Player_formatTime(player, text, sizeof(text), player->totalTime + curTime);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Time:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->totalMoves + player->moves);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Moves:", text, 0, 0);

		SDL_snprintf(text, sizeof(text), "%d", player->totalDeaths);
		MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Deaths:", text, 0, 0);

		textDivider = MenuSystem_buildDivider(menuSystem, "100% KILLS");
		MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

		if (player->killedMonstersLevels != 0) {
			for (cnt = 0; cnt < MAPNAME_MAX; cnt++) {
				if ((player->killedMonstersLevels & 1 << (cnt & 0xff)) != 0) {
					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], menu->doomRpg->game->mapNames[cnt], "", 0, 0);
				}
			}
		}
		else {
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "None", 0, 0);
		}

		SDL_snprintf(text, sizeof(text), "%c 100%% SECRETS %c", 0x80, 0x80);
		MenuItem_Set(&menuSystem->items[menuSystem->numItems++], text, 3, 0);

		if (player->foundSecretsLevels != 0) {
			for (cnt = 0; cnt < MAPNAME_MAX; cnt++) {
				if ((player->foundSecretsLevels & 1 << (cnt & 0xff)) != 0) {
					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], menu->doomRpg->game->mapNames[cnt], "", 0, 0);
				}
			}
		}
		else {
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "None", 0, 0);
		}
	}
}

void Menu_initMenu(Menu_t* menu, int i)
{
	MenuSystem_t* menuSystem;
	EntityDef_t* ent;
	char* textDivider, text[32];
	boolean wpn;

	//printf("initMenu %d\n", i);

	menu->doomRpg->hud->logMessage[0] = '\0';

	menuSystem = menu->doomRpg->menuSystem;
	menuSystem->scrollIndex = 0;
	menuSystem->selectedIndex = 0;
	menuSystem->numItems = 0;
	menuSystem->setBind = false; // new

	switch (i) {
		case MENU_NONE: {
			break;
		}
	
		case MENU_MAIN: {
			menuSystem->type = 4; // MENUTYPE_MAIN
			menuSystem->imgBG = &menuSystem->imgLogo;
			menuSystem->oldMenu = -1;
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Start Game", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Options   ", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Help/About", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Exit      ", 2, 0);
			break;
		}

		case MENU_MAIN_HELP_ABOUT: {
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = MENU_MAIN;
			menuSystem->type = 5; // MENUTYPE_HELP
			Menu_LoadHelpResource(menu);
			break;
		}

		case MENU_MAIN_EXIT: {
			Menu_setYesNo(menu, "Exit Doom RPG?");
			menuSystem->type = 6;
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = MENU_MAIN;
			menuSystem->selectedIndex = 3;
			break;
		}

		case MENU_MAIN_ERASE: {
			Menu_setYesNo(menu, "Erase saved game?");
			menuSystem->type = 6;
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = 1;
			menuSystem->selectedIndex = 3;
			break;
		}

		case MENU_MAIN_SURE: {
			Menu_setYesNo(menu, "Are you sure?");
			menuSystem->type = 6;
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = MENU_MAIN;
			menuSystem->selectedIndex = 3;
			break;
		}
			 
		case MENU_MAIN_CONTINUE: {
			menuSystem->type = 4;
			menuSystem->imgBG = &menuSystem->imgLogo;
			menuSystem->oldMenu = MENU_MAIN;
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Continue", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "New Game", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back    ", 2, 0);
			break;
		}

		case MENU_MAIN_OPTIONS:
		case MENU_INGAME_OPTIONS: {
			if (i == MENU_INGAME_OPTIONS) {
				strncpy(menu->doomRpg->hud->logMessage, "Options", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->type = 1; // MENUTYPE_LIST
				menuSystem->oldMenu = MENU_INGAME;
			}
			else {
				//menuSystem->type = 4; // MENUTYPE_MAIN
				menuSystem->type = 7; // MENUTYPE_MAIN2
				menuSystem->oldMenu = MENU_MAIN;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			// New Option
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Video", 0, (i == MENU_INGAME_OPTIONS) ? true : false);
			// New Option
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Input", 0, (i == MENU_INGAME_OPTIONS) ? true : false);
			// New Option
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sound", 0, (i == MENU_INGAME_OPTIONS) ? true : false);


#if 0 // Original Code
			if (menu->doomRpg->doomCanvas->sndFXOnly == false) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Vibrate:",
					menu->doomRpg->doomCanvas->vibrateEnabled ? "on" : "off", 0, 0);

				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Sound:",
					menu->doomRpg->sound->soundEnabled ? "on" : "off", 0, 0);
			}
			else {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "FX:",
					menu->doomRpg->sound->soundEnabled ? "Sound" :
					menu->doomRpg->doomCanvas->vibrateEnabled ? "Vibrate" : "None", 0, 0);
			}

			if (menu->doomRpg->sound->soundEnabled) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems], "Volume:", "", 0, 0);

				SDL_snprintf(menuSystem->items[menuSystem->numItems].textField2,
					sizeof(menuSystem->items[menuSystem->numItems].textField2), "%d%%", (menu->doomRpg->sound->volume * 100) / 100);
				menuSystem->numItems++;
			}
			else {
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			}
#endif

			break;
		}

		case MENU_ENABLE_SOUNDS: {
			Menu_setYesNo(menu, "Enable sounds?");
			menuSystem->selectedIndex = 2;
			menuSystem->scrollIndex = 0;
			menuSystem->oldMenu = -1;
			menuSystem->type = 4;
			menuSystem->imgBG = &menuSystem->imgLogo;
			break;
		}

		case MENU_MAP_STATS:
		{
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], menu->doomRpg->render->mapName, 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Completed!", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			Menu_fillStatus(menu, 1, 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Press OK to", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "continue   ", 2, 0);
			Sound_playSound(menu->doomRpg->sound, 5043, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
			menu->doomRpg->doomCanvas->numEvents = 0;
			menuSystem->type = 5;
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = -1;
			break;
		}

		case MENU_MAP_STATS_OVERALL:
		{
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], menu->doomRpg->render->mapName, 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Completed!", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			Menu_fillStatus(menu, 1, 0, 1);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Press OK to", 2, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "continue   ", 2, 0);
			Sound_playSound(menu->doomRpg->sound, 5043, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
			menuSystem->type = 5;
			menuSystem->imgBG = NULL;
			menuSystem->oldMenu = -1;
			break;
		}

		case MENU_GOTO_JUNCTION: {
			menuSystem->type = 6;
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Go To Junction", NULL, 2, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Main Menu     ", NULL, 2, 1);
			break;
		}

		case MENU_QUIT_TO_MAIN_MENU: {
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Quit to Main Menu?", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Quit  ", NULL, 2, 1);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Cancel", NULL, 2, 2);
			menuSystem->oldMenu = MENU_GOTO_JUNCTION;
			break;
		}

		case MENU_INGAME_NOTEBOOK: {
			strncpy(menu->doomRpg->hud->logMessage, "Notebook", MS_PER_CHAR);
			menuSystem->type = 5;
			menuSystem->imgBG = (Image_t*)0x0;
			menuSystem->oldMenu = MENU_ITEMS;
			Menu_setNotes(menu);
			break;
		}

		case MENU_INGAME_HELP_ABOUT: {
			strncpy(menu->doomRpg->hud->logMessage, "Help", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_INGAME;
			menuSystem->selectedIndex = 1;
			menuSystem->scrollIndex = 0;
			menuSystem->type = 5;
			Menu_LoadHelpResource(menu);
			break;
		}

		case MENU_INGAME: {
			strncpy(menu->doomRpg->hud->logMessage, "Game Menu", MS_PER_CHAR);
			menuSystem->selectedIndex = 1;
			menuSystem->type = 1;
			menuSystem->oldMenu = MENU_NONE;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Resume Game", 0, MENU_NONE);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Inventory", 0, MENU_ITEMS);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Save Game", 0, MENU_INGAME_SAVE);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Load Game", 0, MENU_INGAME_LOAD);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Automap", 0, MENU_NONE);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Status", 0, MENU_INGAME_STATUS);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Help/About", 0, MENU_INGAME_HELP_ABOUT);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Options", 0, MENU_INGAME_OPTIONS);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Main Menu", 0, MENU_INGAME_EXIT);
			break;
		}

		case MENU_INGAME_STATUS: {
			strncpy(menu->doomRpg->hud->logMessage, "Status", MS_PER_CHAR);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			Menu_fillStatus(menu, menu->doomRpg->render->loadMapID != 2, 1, 1);
			menuSystem->type = 1;
			menuSystem->oldMenu = MENU_INGAME;
			break;
		}

		case MENU_INGAME_EXIT: {
			strncpy(menu->doomRpg->hud->logMessage, "Confirm Quit", MS_PER_CHAR);
			menuSystem->type = 6;

			if (menuSystem->oldMenu == MENU_INGAME) {
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Go to Main Menu?", 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Save Game ", NULL, 2, 0);
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Don't Save", NULL, 2, 1);
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Cancel    ", NULL, 2, 2);
			}
			else {
				Menu_setYesNo(menu, "Quit to Main Menu?");
			}
			break;
		}

		case MENU_ITEMS: {
			strncpy(menu->doomRpg->hud->logMessage, "Inventory", MS_PER_CHAR);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			wpn = menu->doomRpg->player->weapons != 0;
			if (wpn) {
				textDivider = MenuSystem_buildDivider(menuSystem, "WEAPONS");
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);
				menu->f712d = menuSystem->numItems;

				for (int i10 = 0; i10 <= 11; i10++) {
					if ((menu->doomRpg->player->weapons & (1 << i10)) != 0) {
						ent = EntityDef_find(menu->doomRpg->entityDef, 5, (byte)i10);

						if (i10 != 0) {
							SDL_snprintf(text, sizeof(text), "%d", menu->doomRpg->player->ammo[menu->doomRpg->combat->weaponInfo[i10].ammoType]);
							MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], ent->name, text, 0, i10);
						}
						else {
							MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], ent->name, "--", 0, i10);
						}
					}
				}
			}
			else {
				menu->f712d = 999;
			}

			textDivider = MenuSystem_buildDivider(menuSystem, "ITEMS");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Notebook", "--", 0, 0);
			menu->f711c = menuSystem->numItems;

			for (int i13 = 25; i13 <= 29; i13++) {
				if (menu->doomRpg->player->inventory[i13 - 25] != 0) {
					SDL_snprintf(text, sizeof(text), "%d", menu->doomRpg->player->inventory[i13 - 25]);
					ent = EntityDef_find(menu->doomRpg->entityDef, 4, (byte)i13);

					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], ent->name, text, 0, i13 - 25);
				}
			}

			textDivider = MenuSystem_buildDivider(menuSystem, "OTHER");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);
			menu->f713e = menuSystem->numItems;

			SDL_snprintf(text, sizeof(text), "%d", menu->doomRpg->player->credits);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Credits", text, 0, 0);

			if ((menu->doomRpg->player->keys & 1) != 0) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Green Key", "--", 0, 0);
			}
			if ((menu->doomRpg->player->keys & 2) != 0) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Yellow Key", "--", 0, 0);
			}
			if ((menu->doomRpg->player->keys & 4) != 0) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Blue Key", "--", 0, 0);
			}
			if ((menu->doomRpg->player->keys & 8) != 0) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Red Key", "--", 0, 0);
			}

			menuSystem->type = 1;
			menuSystem->scrollIndex = menu->f715g;
			menuSystem->selectedIndex = menu->f714f;
			if (menuSystem->selectedIndex >= menuSystem->numItems || (menuSystem->items[menuSystem->selectedIndex].flags & 0x1) != 0x0) {
				do {
					--menuSystem->selectedIndex;
				} while (menuSystem->selectedIndex >= menuSystem->numItems || (menuSystem->items[menuSystem->selectedIndex].flags & 0x1) != 0x0);
				if (menuSystem->scrollIndex > menuSystem->selectedIndex) {
					menuSystem->scrollIndex = menuSystem->selectedIndex;
				}
			}
			if (menuSystem->selectedIndex + menuSystem->scrollIndex > menuSystem->numItems) {
				menuSystem->scrollIndex = menuSystem->selectedIndex - menuSystem->maxItems + 1;
				if (menuSystem->scrollIndex < 0) {
					menuSystem->scrollIndex = 0;
				}
			}
			menuSystem->oldMenu = MENU_INGAME;
			break;
		}

		case MENU_ITEMS_CONFIRM: {
			boolean itemUse = true;

			ent = EntityDef_find(menu->doomRpg->entityDef, 4, menu->f701a + 25);

			if (ent->eSubType == 25 || ent->eSubType == 26) {
				if (CombatEntity_getHealth(&menu->doomRpg->player->ce) == CombatEntity_getMaxHealth(&menu->doomRpg->player->ce)) {
					itemUse = false;
				}
			}
			else if (ent->eSubType == 27 && 
				CombatEntity_getHealth(&menu->doomRpg->player->ce) == CombatEntity_getMaxHealth(&menu->doomRpg->player->ce) && 
				CombatEntity_getArmor(&menu->doomRpg->player->ce) == CombatEntity_getMaxArmor(&menu->doomRpg->player->ce)) {
				itemUse = false;
			}

			if (itemUse) {
				strncpy(menu->doomRpg->hud->logMessage, "Confirm item use", MS_PER_CHAR);
				SDL_snprintf(text, sizeof(text), "Use %s?", ent->name);
				Menu_setYesNo(menu, text);
				menuSystem->type = 6;
				menuSystem->scrollIndex = 0;
				menuSystem->selectedIndex = 2;
			}
			else {
				strncpy(menu->doomRpg->hud->logMessage, "Unable to use", MS_PER_CHAR);

				if (ent->eSubType == 25 || ent->eSubType == 26) {
					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Health full.", NULL, 3, 0);
					MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
					MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 2, 0);
				}
				else if (ent->eSubType == 27) {
					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Health and armor", NULL, 3, 0);
					MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "full.           ", NULL, 3, 0);
					MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
					MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 2, 0);
				}

				menuSystem->type = 6;
				menuSystem->scrollIndex = 0;
				menuSystem->selectedIndex = menuSystem->numItems - 1;
			}
			menuSystem->oldMenu = MENU_ITEMS;
			break;
		}

		case MENU_INGAME_DEAD: {
			strncpy(menu->doomRpg->hud->logMessage, "You died!", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_INGAME_DEAD;
			menuSystem->type = 6;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Try again?", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Load Saved Game", NULL, 2, 0);
			if (menu->doomRpg->render->loadMapID != MAP_INTRO) {
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Go To Junction ", NULL, 2, 1);
			}
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Retry Sector   ", NULL, 2, 2);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Main Menu      ", NULL, 2, 3);
			break;
		}

		case MENU_CONFIRM_LOAD: {
			strncpy(menu->doomRpg->hud->logMessage, "Confirm load", MS_PER_CHAR);
			menuSystem->type = 27;
			menuSystem->type = 6;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "This will take ", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "you to Junction", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "before the 2nd ", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "invasion. Are  ", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "you sure?      ", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Yes ", NULL, 2, 1);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "No  ", NULL, 2, 4);
			break;
		}

		case MENU_DEBUG: {
			strncpy(menu->doomRpg->hud->logMessage, "Debug", MS_PER_CHAR);
			menuSystem->type = 1;

			if ((menu->doomRpg->render->mapSprites == NULL) || (menu->doomRpg->doomCanvas->unloadMedia)) {
				menuSystem->oldMenu = MENU_MAIN;
			}
			else {
				menuSystem->oldMenu = MENU_INGAME;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Cheats", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Change Map", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Stats", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Developer", 0, 0);
			break;
		}

		case MENU_DEVELOPER_VARS: {
			strncpy(menu->doomRpg->hud->logMessage, "Vars", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_DEVELOPER;
			menuSystem->type = 1;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			SDL_snprintf(text, sizeof(text), "%d", menu->doomRpg->doomCanvas->animFrames);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "r_frames", text, 0, 0);

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_speeds", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_skipCull", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_skipStretch", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_skipBSP", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_skipLines", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_skipSprites", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "r_onlyRender", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "s_debug", 0, 0);
			break;
		}

		case MENU_DEBUG_MAPS: {
			strncpy(menu->doomRpg->hud->logMessage, "Maps", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_DEBUG;
			menuSystem->type = 1;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Intro", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Junction", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 1", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 2", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 3", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 4", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 5", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 6", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Sector 7", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Des. Junction", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Reactor", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Credits", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Items", 0, 0);
			break;
		}

		case MENU_DEBUG_CHEATS: {
			strncpy(menu->doomRpg->hud->logMessage, "Cheats", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_DEBUG;
			menuSystem->type = 1;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Noclip", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Disable AI", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Give all", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Give ammo", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "God mode", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Level up", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Give map", 0, 0);
			break;
		}

		case MENU_DEBUG_STATS: {
			strncpy(menu->doomRpg->hud->logMessage, "Statistics", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_DEBUG;
			menuSystem->type = 1;
			SDL_snprintf(text, sizeof(text), "%d %d", menuSystem->doomRpg->doomCanvas->viewX >> 6, menuSystem->doomRpg->doomCanvas->viewY >> 6);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Pos:", text, 0, 0);

			
			// En el código fuente original se obtiene la memoria RAM del dispositivo
			// In the actual source code, the device’s RAM memory is obtained
			// SDL_snprintf(text, sizeof(text), "%dK", ((menu->doomRpg->m_DeviceInfo).dwRAM + 1023) / 1024);
			
			// Actualmente se obtiene el total de toda la momoria inicializada
			// Currently, it has the total of all initialized memory
			SDL_snprintf(text, sizeof(text), "%dK", (menu->doomRpg->memoryBeg + DoomRPG_freeMemory() + 1023) / 1024);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Total:", text, 0, 0);

			SDL_snprintf(text, sizeof(text), "%dK", (menu->doomRpg->memoryBeg + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Init:", text, 0, 0);

			SDL_snprintf(text, sizeof(text), "%dK", (DoomRPG_freeMemory() + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Cur:", text, 0, 0);

			SDL_snprintf(text, sizeof(text), "%dK", (DoomRPG_freeMemory() - menu->doomRpg->memoryBeg + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Alloc:", text, 0, 0);

			SDL_snprintf(text, sizeof(text), "%dK", (menu->doomRpg->render->mapMemory + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Map:", text, 0, 0);

			int iVar8 = 0;
			int iVar3 = 0;
			do {
				iVar8 = menu->doomRpg->sound->soundChannel[iVar3].size + iVar8;
			} while (++iVar3 < 10);

			SDL_snprintf(text, sizeof(text), "%dK", (iVar8 + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Sound:", text, 0, 0);

			SDL_snprintf(text, sizeof(text), "%dK", (menu->doomRpg->imageMemory + 1023) / 1024);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Image:", text, 0, 0);
			break;
		}

		case MENU_STORE_CONFIRM: {
			strncpy(menu->doomRpg->hud->logMessage, "Item Vendor", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_NONE;
			menuSystem->type = 6;

			if ((menuSystem->f749g < 0) || ((menuSystem->f749g + 1) > MAXSTORES)) {
				menuSystem->f749g = 0;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Would you like", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "to make a pur-", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "chase?        ", 3, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Yes", NULL, 2, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "No ", NULL, 2, 1);
			break;
		}

		case MENU_STORE: {
			SDL_snprintf(menu->doomRpg->hud->logMessage, sizeof(menu->doomRpg->hud->logMessage), "%d Credits", menu->doomRpg->player->credits);
			menuSystem->scrollIndex = menu->f702a;
			menuSystem->selectedIndex = menu->f708b;
			menuSystem->oldMenu = MENU_NONE;
			menuSystem->type = 1;
			Menu_setStore(menu);
			break;
		}

		case MENU_STORE_BUY: {

			byte b2 = 0;
			int num = SDL_atoi(menuSystem->items[menu->f708b].textField2);
			int i18 = menuSystem->items[menu->f708b].action & 255;
			int i19 = menuSystem->items[menu->f708b].action >> 16;

			//printf("Purchasing: %s\n", menuSystem->items[menu->f708b].textField);

			menu->f701a = -1;

			if (i18 == 1) {
				menu->f701a = (byte)(i19 - 25);
				b2 = menu->doomRpg->player->inventory[menu->f701a];
			}
			else if (i18 == 2) {
				b2 = menu->doomRpg->player->ammo[i19];
			}
			else {
				switch (i19) {
				case 1:
					b2 = CombatEntity_getDefense(&menu->doomRpg->player->ce);
					break;
				case 2:
					b2 = CombatEntity_getAgility(&menu->doomRpg->player->ce);
					break;
				case 3:
					b2 = CombatEntity_getStrength(&menu->doomRpg->player->ce);
					break;
				case 4:
					b2 = CombatEntity_getAccuracy(&menu->doomRpg->player->ce);
					break;
				}
			}

			if (num > menu->doomRpg->player->credits) {
				strncpy(menu->doomRpg->hud->logMessage, "Not enough credits", MS_PER_CHAR);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "You do not have", 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "enough credits!", 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 2, 0);
				menuSystem->selectedIndex = 3;
				menuSystem->type = 6;
			}
			else if (b2 == 99) {
				strncpy(menu->doomRpg->hud->logMessage, "Cannot purchase", MS_PER_CHAR);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "You have too ", 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "many already!", 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 2, 0);
				menuSystem->selectedIndex = 3;
				menuSystem->type = 6;
			}
			else {
				strncpy(menu->doomRpg->hud->logMessage, "Confirm Purchase", MS_PER_CHAR);
				SDL_snprintf(text, sizeof(text), "Buy %s?", menuSystem->items[menu->f708b].textField);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], text, 3, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Yes", 2, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "No ", 2, 0);
				menuSystem->type = 6;
			}

			menuSystem->oldMenu = MENU_STORE;
			break;
		}

		case MENU_DEVELOPER: {
			strncpy(menu->doomRpg->hud->logMessage, "Developer", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_DEBUG;
			menuSystem->type = 1;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Vars", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Benchmark", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Store0", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Store1", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Store2", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Store3", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Force Error", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Print Monsters", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Print Memory", 0, 0);
			break;
		}

		case MENU_INGAME_LOAD: {
			strncpy(menu->doomRpg->hud->logMessage, "Confirm Load", MS_PER_CHAR);
			menuSystem->oldMenu = MENU_INGAME;
			menuSystem->selectedIndex = 2;
			menuSystem->scrollIndex = 0;
			menuSystem->type = 6;
			Menu_setYesNo(menu, "Load Game?");
			break;
		}

		case MENU_INGAME_LOADNOSAVE: {
			menuSystem->selectedIndex = 2;
			menuSystem->numItems = 1;
			menuSystem->type = 6;
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "No Saved Game", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back         ", 2, 0);
			break;
		}

		// NEW MENU OPTIONS
		case MENU_VIDEO:
		case MENU_INGAME_VIDEO: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Video Options", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_OPTIONS;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_MAIN_OPTIONS;
				menuSystem->type = 7;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "FullScreen:", sdlVideo.fullScreen ? "on" : "off", 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "VSync:", sdlVideo.vSync ? "on" : "off", 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "IntScaling:", sdlVideo.integerScaling ? "on" : "off", 0, 0);
			textDivider = MenuSystem_buildDivider(menuSystem, "Resolution");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);
			SDL_snprintf(text, sizeof(text), "(%dx%d)", sdlVideoModes[sdlVideo.resolutionIndex].width, sdlVideoModes[sdlVideo.resolutionIndex].height);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], text, NULL, 2, 0);

			textDivider = MenuSystem_buildDivider(menuSystem, "Display");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Softkeys:", sdlVideo.displaySoftKeys ? "on" : "off", 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Floor/Ceil:", menu->doomRpg->doomCanvas->renderFloorCeilingTextures ? "on" : "off", 0, 0);
			
			break;
		}

		// NEW MENU OPTIONS
		case MENU_INPUT:
		case MENU_INGAME_INPUT: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Input Options", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_OPTIONS;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_MAIN_OPTIONS;
				menuSystem->type = 7;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Bindings", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Mouse", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Controller", 0, 0);
			break;
		}

		// NEW MENU OPTIONS
		case MENU_SOUND:
		case MENU_INGAME_SOUND: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Sound Options", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_OPTIONS;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_MAIN_OPTIONS;
				menuSystem->type = 7;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Sound:",
				menu->doomRpg->sound->soundEnabled ? "on" : "off", 0, 0);
			if (menu->doomRpg->sound->soundEnabled) {
				MenuItem_Set2(&menuSystem->items[menuSystem->numItems], "Volume:", "", 0, 0);

				SDL_snprintf(menuSystem->items[menuSystem->numItems].textField2,
					sizeof(menuSystem->items[menuSystem->numItems].textField2), "%d%%", (menu->doomRpg->sound->volume * 100) / 100);
				menuSystem->numItems++;

				MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Priority:",
					menu->doomRpg->doomCanvas->sndPriority ? "on" : "off", 0, 0);
			}
			else {
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
				MenuItem_Set(&menuSystem->items[menuSystem->numItems++], NULL, 0, 0);
			}

			break;
		}

		// NEW MENU OPTIONS
		case MENU_BINDINGS:
		case MENU_INGAME_BINDINGS: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Bindings", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_INPUT;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_INPUT;
				menuSystem->type = 7;
			}

			menuSystem->nextMsgTime = 0;
			menuSystem->nextMsg = 0;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			textDivider = MenuSystem_buildDivider(menuSystem, "MOVEMENT");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 3, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Move Fwd:", NULL, 0, 0);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Move Back:", NULL, 0, 1);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 1);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Move Left:", NULL, 0, 4);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 4);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Move Right:", NULL, 0, 5);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 5);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Turn Left:", NULL, 0, 2);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 2);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Turn Right:", NULL, 0, 3);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 3);

			textDivider = MenuSystem_buildDivider(menuSystem, " OTHER  ");
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], textDivider, 1, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Atk/Talk/Use:", NULL, 0, 8);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 8);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Next Weapon:", NULL, 0, 6);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 6);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Prev Weapon:", NULL, 0, 7);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 7);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Pass Turn:", NULL, 0, 9);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 9);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Automap:", NULL, 0, 10);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 10);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Menu Open/Back:", NULL, 0, 11);
			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "", NULL, 1 | 8, 11);

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Reset Binds", 0, 0);
			break;
		}

		// NEW MENU OPTIONS
		case MENU_MOUSE:
		case MENU_INGAME_MOUSE: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Mouse", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_INPUT;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_INPUT;
				menuSystem->type = 7;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			SDL_snprintf(text, sizeof(text), "Sensitivi%c:", 0x7F);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems], text, 0, 0);
			SDL_snprintf(menuSystem->items[menuSystem->numItems].textField2,
				sizeof(menuSystem->items[menuSystem->numItems].textField2), "%d%%", (menu->doomRpg->doomCanvas->mouseSensitivity * 100) / 100);
			menuSystem->numItems++;

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Y Movement:", menu->doomRpg->doomCanvas->mouseYMove ? "on" : "off", 0, 0);
			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Reset Defaults", 0, 0);
			break;
		}

		// NEW MENU OPTIONS
		case MENU_CONTROLLER:
		case MENU_INGAME_CONTROLLER: {
			if (menuSystem->type == 1) {
				strncpy(menu->doomRpg->hud->logMessage, "Controller", sizeof(menu->doomRpg->hud->logMessage));
				menuSystem->oldMenu = MENU_INGAME_INPUT;
				menuSystem->type = 1;
			}
			else {
				menuSystem->oldMenu = MENU_INPUT;
				menuSystem->type = 7;
			}

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Back", 0, 0);

			MenuItem_Set2(&menuSystem->items[menuSystem->numItems++], "Vibrate:",
				menu->doomRpg->doomCanvas->vibrateEnabled ? "on" : "off", 0, 0);

			MenuItem_Set(&menuSystem->items[menuSystem->numItems], "D-Zone L:", 0, 0);
			SDL_snprintf(menuSystem->items[menuSystem->numItems].textField2,
				sizeof(menuSystem->items[menuSystem->numItems].textField2), "%d%%", (sdlController.deadZoneLeft * 100) / 100);
			menuSystem->numItems++;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems], "D-Zone R:", 0, 0);
			SDL_snprintf(menuSystem->items[menuSystem->numItems].textField2,
				sizeof(menuSystem->items[menuSystem->numItems].textField2), "%d%%", (sdlController.deadZoneRight * 100) / 100);
			menuSystem->numItems++;

			MenuItem_Set(&menuSystem->items[menuSystem->numItems++], "Reset Defaults", 0, 0);

			break;
		}
	}
}

int Menu_select(Menu_t* menu, int menuId, int itemId)
{
	MenuSystem_t* menuSystem;
	DoomCanvas_t* doomCanvas;
	DoomRPG_t* doomRpg;
	EntityDef_t* ent;
	char text[32];

	int action, animFrames;

	//printf("Menu %d, Item %d\n", menuId, itemId);

	doomRpg = menu->doomRpg;
	doomCanvas = doomRpg->doomCanvas;
	menuSystem = doomRpg->menuSystem;

	action = 0;
	if (menuSystem->items[itemId].textField[0] != '\0') {
		action = menuSystem->items[itemId].action;
	}

	//printf("action %d\n", action);

	switch (menuId)
	{
		case MENU_NONE: {
			break;
		}

		case MENU_MAIN: {
			if (itemId == 0) { // Start Game
				if (Game_checkConfigVersion(menu->doomRpg->game)) {
					return MENU_MAIN_CONTINUE;
				}
				Menu_startGame(menu, 1);
				return MENU_NONE;
			}
			else if (itemId == 1) {
				return MENU_MAIN_OPTIONS; // Options
			}
			else if (itemId == 2) {
				return MENU_MAIN_HELP_ABOUT; // Help/About
			}
			else if (itemId == 3) {
				return MENU_MAIN_EXIT; // Exit
			}
			else {
				return MENU_NONE;
			}
			break;
		}

		case MENU_MAIN_HELP_ABOUT: {
			return MENU_MAIN;
			break;
		}

		case MENU_MAIN_EXIT: {
			if (itemId == 2) {
				DoomRPG_notifyDestroyed(menu->doomRpg);
				return MENU_MAIN_EXIT;
			}
			else if (itemId == 3) {
				return MENU_MAIN;
			}
			break;
		}

		case MENU_MAIN_ERASE: {
			if (itemId == 2) { // Yes
				return MENU_MAIN_SURE;
			}
			else if (itemId == 3) { // No
				return MENU_MAIN;
			}
			break;
		}

		case MENU_MAIN_SURE: {
			if (itemId == 2) { // Yes
				Game_deleteSaveFiles(menu->doomRpg->game);
				Menu_startGame(menu, 1);
			}
			else if (itemId == 3) { // No
				return MENU_MAIN;
			}
			break;
		}

		case MENU_MAIN_CONTINUE: {
			if (itemId == 0) {
				Menu_startGame(menu, 0);
			}
			else if (itemId == 1) {
				if (Game_checkConfigVersion(menu->doomRpg->game)) {
					return MENU_MAIN_ERASE;
				}
				Menu_startGame(menu, 1);
			}
			else {
				return menu->doomRpg->menuSystem->oldMenu;
			}
			break;
		}

		case MENU_MAIN_OPTIONS:    // MENU_MAIN_OPTIONS
		case MENU_INGAME_OPTIONS: { // MENU_INGAME_OPTIONS



				if (itemId == 0) {
					return menuSystem->oldMenu;
				}
				else if (itemId == 1) { // New video Option
					if (menuSystem->items[itemId].action == true) {
						return MENU_INGAME_VIDEO;
					}
					else{
						return MENU_VIDEO;
					}
				}
				else if (itemId == 2) { // New Input Option
					if (menuSystem->items[itemId].action == true) {
						return MENU_INGAME_INPUT;
					}
					else {
						return MENU_INPUT;
					}
				}
				else if (itemId == 3) { // New Input Option
					if (menuSystem->items[itemId].action == true) {
						return MENU_INGAME_SOUND;
					}
					else {
						return MENU_SOUND;
					}
				}

#if 0 // Original Code
			if (doomCanvas->sndFXOnly == false) {
				if (itemId == 0) {
					return menuSystem->oldMenu;
				}
				else if (itemId == 1) {

					doomCanvas->vibrateEnabled ^= true;

					if (doomCanvas->vibrateEnabled) {
						DoomCanvas_startShake(doomCanvas, 0, 0, 500);
						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "on");
					}
					else {
						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "off");
					}
				}
				else if (itemId == 2) {

					doomRpg->sound->soundEnabled ^= true;

					if (doomRpg->sound->soundEnabled) {
						if (menuSystem->oldMenu == MENU_INGAME) {
							MenuSystem_playSound(menuSystem);
						}
						else {
							Sound_playSound(doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
						}
						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "on");

						MenuItem_Set2(&menuSystem->items[itemId + 1], "Volume:", "", 0, 0);

						SDL_snprintf(menuSystem->items[itemId + 1].textField2,
							sizeof(menuSystem->items[itemId + 1].textField2), "%d%%", (doomRpg->sound->volume * 100) / 100);
					}
					else {
						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "off");
						Sound_stopSounds(doomRpg->sound);
						Sound_freeSounds(doomRpg->sound);
						MenuItem_Set(&menuSystem->items[itemId + 1], NULL, 0, 0);
					}
				}
				else if (itemId == 3) {

					if (doomRpg->sound->volume == 100) {
						doomRpg->sound->volume = 0;
						Sound_updateVolume(menu->doomRpg->sound);
					}
					else {
						Sound_addVolume(menu->doomRpg->sound, 10);
					}
				}
			}
			else {
				if (itemId == 0) {
					return menuSystem->oldMenu;
				}
				if (itemId == 1) {

					if (doomRpg->sound->soundEnabled == false) {

						printf("Sound Only\n");
						doomRpg->sound->soundEnabled = true;
						doomCanvas->vibrateEnabled = false;

						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "Sound");

						if (menuSystem->oldMenu == MENU_INGAME) {
							MenuSystem_playSound(menuSystem);
						}
						else {
							Sound_playSound(doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
						}
					}
					else {
						printf("None\n");
						doomRpg->sound->soundEnabled = false;
						doomCanvas->vibrateEnabled = false;

						strcpy_s(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "None");
						Sound_stopSounds(doomRpg->sound);
						Sound_freeSounds(doomRpg->sound);
					}

					if (menu->doomRpg->sound->soundEnabled) {
						MenuItem_Set2(&menuSystem->items[itemId + 1], "Volume:", "", 0, 0);

						SDL_snprintf(menuSystem->items[itemId + 1].textField2,
							sizeof(menuSystem->items[itemId + 1].textField2), "%d%%", (doomRpg->sound->volume * 100) / 100);
					}
					else {
						MenuItem_Set(&menuSystem->items[itemId + 1], NULL, 0, 0);
					}
				}
				else if (itemId == 2) {
					if (doomRpg->sound->volume == 100) {
						doomRpg->sound->volume = 0;
						Sound_updateVolume(menu->doomRpg->sound);
					}
					else {
						Sound_addVolume(menu->doomRpg->sound, 10);
					}
				}
			}
#endif

			Game_saveConfig(menu->doomRpg->game, 0);
			menuSystem->paintMenu = true;
			return menuSystem->menu;
			break;
		}

		case MENU_ENABLE_SOUNDS: {
			if (itemId != 2) {
				return MENU_MAIN;
			}
			menu->doomRpg->sound->soundEnabled = true;
			if (doomCanvas->sndFXOnly) {
				//doomCanvas->vibrateEnabled = false;
			}

			Sound_playSound(menu->doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
			return MENU_MAIN;
			break;
		}

		case MENU_MAP_STATS:
		case MENU_MAP_STATS_OVERALL:
		{ 
			DoomCanvas_loadMap(doomCanvas, menu->mapNameId);
			break;
		}

		case MENU_GOTO_JUNCTION: {	// MENU_GOTO_JUNCTION
			if (action == 0) {
				DoomCanvas_loadMap(menu->doomRpg->doomCanvas, MAP_JUNCTION);
			}
			else if(action == 1) {
				return MENU_QUIT_TO_MAIN_MENU;
			}
			break;
		}

		case MENU_QUIT_TO_MAIN_MENU: {

			if (action == 2) {
				return MENU_GOTO_JUNCTION;
			}

			if (action == 0) {
				DoomCanvas_saveState(menu->doomRpg->doomCanvas, 12, "Saving Game...");
				return MENU_NONE;
			}
			else {
				Sound_playSound(menu->doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
				return MENU_MAIN;
			}
		}

		case MENU_INGAME_NOTEBOOK: {
			return MENU_ITEMS;
			break;
		}

		case MENU_INGAME_HELP_ABOUT: {
			return MENU_INGAME;
			break;
		}

		case MENU_INGAME: {
			menuSystem->oldMenu = MENU_NONE;

			if (itemId == 2) {
				DoomCanvas_saveState(doomCanvas, 3, "Saving Game...");
				break;
			}
			else if(itemId == 3) {
				return Game_checkConfigVersion(menu->doomRpg->game) ? MENU_INGAME_LOAD : MENU_INGAME_LOADNOSAVE;
			}
			else if (itemId == 4) {
				DoomCanvas_setState(doomCanvas, ST_AUTOMAP);
				return action;
			}
			else if (itemId == 8) {
				menuSystem->oldMenu = MENU_INGAME;
				return action;
			}

			return action;
			break;
		}

		case MENU_INGAME_STATUS: {
			return MENU_INGAME;
			break;
		}

		case MENU_INGAME_EXIT: {
			if (menuSystem->oldMenu == MENU_INGAME) {
				if (action == 0) { // Save Game
					DoomCanvas_saveState(doomCanvas, 11, "Saving Game...");
				}
				else if(action == 1) { // Don't Save
					DoomCanvas_setupmenu(menu->doomRpg->doomCanvas, 0);
					return MENU_MAIN;
				}
				else if (action == 2) { // Cancel
					return menuSystem->oldMenu;
				}
			}
			else {
				if (action == 1) { // Yes
					DoomCanvas_setupmenu(menu->doomRpg->doomCanvas, 0);
					return MENU_MAIN;
				}
				else if (action == 0) { // No
					return menuSystem->oldMenu;
				}
			}
			break;
		}

		case MENU_ITEMS: {
			menu->f714f = menuSystem->selectedIndex;
			menu->f715g = menuSystem->scrollIndex;
			if (itemId == 0) {
				return MENU_INGAME;
			}
			if (menu->f713e <= itemId) {
				return MENU_ITEMS;
			}
			if (menu->f711c - 1 == itemId) {
				return MENU_INGAME_NOTEBOOK;
			}
			if (menu->f711c <= itemId) {
				menu->f701a = (byte)menuSystem->items[itemId].action;
				return MENU_ITEMS_CONFIRM;
			}
			if (menu->f712d <= itemId) {
				Player_selectWeapon(menu->doomRpg->player, menuSystem->items[itemId].action);
			}
			break;
		}

		case MENU_ITEMS_CONFIRM: {

			if (menuSystem->type == 1) {
				return MENU_ITEMS;
			}

			if (action == 1) {
				ent = EntityDef_find(menu->doomRpg->entityDef, 4, 25 + menu->f701a);
				Player_useItem(menu->doomRpg->player, ent->eSubType);
				return MENU_NONE;
			}
			menuSystem->selectedIndex = menu->f714f;
			menuSystem->scrollIndex = menu->f715g;

			return MENU_ITEMS;
			break;
		}

		case MENU_INGAME_DEAD: {

			switch (menuSystem->items[itemId].action) {
				case 0: {
					if (!Game_checkConfigVersion(menu->doomRpg->game)) {
						return MENU_INGAME_LOADNOSAVE;
					}
					DoomCanvas_loadState(doomCanvas, 1, "Game Loaded");
					return MENU_NONE;
				}
				case 1: {
					if (menu->doomRpg->render->loadMapID >= MAP_JUNCTION_DESTROYED) {
						return MENU_CONFIRM_LOAD;
					}
					DoomCanvas_loadState(doomCanvas, 3, "Loading Junction");
					return MENU_NONE;
				}
				case 2: {
					if (menu->doomRpg->render->loadMapID == MAP_INTRO) {
						Player_reset(menu->doomRpg->player);
						DoomCanvas_loadMap(doomCanvas, MAP_INTRO);
						return MENU_NONE;
					}
					DoomCanvas_loadState(doomCanvas, 2, "Loading Sector");
					return MENU_NONE;
				}
				case 3: {
					return MENU_INGAME_EXIT;
				}
			}

			return MENU_NONE;
			break;
		}

		case MENU_CONFIRM_LOAD: {
			if (action != 1) {
				return MENU_INGAME_DEAD;
			}
			DoomCanvas_loadState(menu->doomRpg->doomCanvas, 3, "Loading Junction");
			break;
		}

		case MENU_DEBUG: {
			switch (itemId) {
			case 0:
				return menu->doomRpg->menuSystem->oldMenu;
			case 1:
				return MENU_DEBUG_CHEATS;
			case 2:
				return MENU_DEBUG_MAPS;
			case 3:
				return MENU_DEBUG_STATS;
			case 4:
				return MENU_DEVELOPER;
			}
			break;
		}

		case MENU_DEVELOPER_VARS: {
			switch (itemId) {
			case 0:
				return menuSystem->oldMenu;
			case 1:
				animFrames = doomCanvas->animFrames + 1;
				if (animFrames >= 16) {
					animFrames = 2;
				}

				DoomCanvas_setAnimFrames(doomCanvas, animFrames);
				SDL_snprintf(menuSystem->items[itemId].textField2, sizeof(menuSystem->items[itemId].textField2), "%d", animFrames);
				menuSystem->paintMenu = true;
				Game_saveConfig(menu->doomRpg->game, 0);
				return MENU_DEVELOPER_VARS;
			case 2:
				doomCanvas->speeds = !doomCanvas->speeds;
				SDL_snprintf(text, sizeof(text), "r_speeds %s", doomCanvas->speeds ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 3:
				menu->doomRpg->render->skipCull = !menu->doomRpg->render->skipCull;
				SDL_snprintf(text, sizeof(text), "r_skipCull %s", menu->doomRpg->render->skipCull ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 4:
				menu->doomRpg->render->skipStretch = !menu->doomRpg->render->skipStretch;
				SDL_snprintf(text, sizeof(text), "r_skipStretch %s", menu->doomRpg->render->skipStretch ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 5:
				menu->doomRpg->render->skipBSP = !menu->doomRpg->render->skipBSP;
				SDL_snprintf(text, sizeof(text), "r_skipBSP %s", menu->doomRpg->render->skipBSP ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 6:
				menu->doomRpg->render->skipLines = !menu->doomRpg->render->skipLines;
				SDL_snprintf(text, sizeof(text), "r_skipLines %s", menu->doomRpg->render->skipLines ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 7:
				menu->doomRpg->render->skipSprites = !menu->doomRpg->render->skipSprites;
				SDL_snprintf(text, sizeof(text), "r_skipSprites %s", menu->doomRpg->render->skipSprites ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 8:
				doomCanvas->renderOnly = !doomCanvas->renderOnly;
				SDL_snprintf(text, sizeof(text), "r_onlyRender %s", doomCanvas->renderOnly ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				doomCanvas->skipCheckState = true;
				return MENU_DEVELOPER_VARS;
			case 9:
				doomCanvas->s_debug = !doomCanvas->s_debug;
				SDL_snprintf(text, sizeof(text), "s_debug %s", doomCanvas->s_debug ? "on" : "off");
				Hud_addMessage(menu->doomRpg->hud, text);
				return MENU_DEVELOPER_VARS;
			}

			return MENU_NONE;
			break;
		}

		case MENU_DEBUG_MAPS: { // MENU_DEBUG_MAPS
			switch (itemId) {
			case 0:
				return menuSystem->oldMenu;
				break;
			case 1:
				DoomCanvas_loadMap(doomCanvas, MAP_INTRO);
				break;
			case 2:
				DoomCanvas_loadMap(doomCanvas, MAP_JUNCTION);
				break;
			case 3:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR01);
				break;
			case 4:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR02);
				break;
			case 5:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR03);
				break;
			case 6:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR04);
				break;
			case 7:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR05);
				break;
			case 8:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR06);
				break;
			case 9:
				DoomCanvas_loadMap(doomCanvas, MAP_SECTOR07);
				break;
			case 10:
				DoomCanvas_loadMap(doomCanvas, MAP_JUNCTION_DESTROYED);
				break;
			case 11:
				DoomCanvas_loadMap(doomCanvas, MAP_REACTOR);
				break;
			case 12:
				DoomCanvas_loadMap(doomCanvas, MAP_END_GAME);
				break;
			case 13:
				DoomCanvas_loadMap(doomCanvas, MAP_ITEMS);
				break;
			}
			
			return MENU_NONE;
			break;
		}

		case MENU_DEBUG_CHEATS: {
			switch (itemId) {
			case 0:
				return menuSystem->oldMenu;
			case 1:
				menu->doomRpg->player->noclip = !menu->doomRpg->player->noclip;
				Hud_addMessage(menu->doomRpg->hud, menu->doomRpg->player->noclip ? "noclip on" : "noclip off");
				break;
			case 2:
				menu->doomRpg->game->disableAI = !menu->doomRpg->game->disableAI;
				Hud_addMessage(menu->doomRpg->hud, menu->doomRpg->game->disableAI ? "AI off" : "AI on");
				break;
			case 3:
				menu->doomRpg->player->weapons = 2559;
				menu->doomRpg->player->keys = 15;
				for (int i6 = 0; i6 < 6; i6++) {
					Player_addAmmo(menu->doomRpg->player, i6, 10);
				}
				for (int i7 = 25; i7 <= 29; i7++) {
					Player_addItem(menu->doomRpg->player, i7, 5);
				}
				Player_addCredits(menu->doomRpg->player, 20);
				Hud_addMessage(menu->doomRpg->hud, "give all");
				break;
			case 4:

				for (int i8 = 0; i8 < 6; i8++) {
					Player_addAmmo(menu->doomRpg->player, i8, 10);
				}
				Hud_addMessage(menu->doomRpg->hud, "give ammo");
				break;
			case 5:
				menu->doomRpg->player->god = !menu->doomRpg->player->god;
				Hud_addMessage(menu->doomRpg->hud, menu->doomRpg->player->god ? "God on" : "God off");
				break;
			case 6:
				Player_nextLevel(menu->doomRpg->player);
				snprintf(menuSystem->items[itemId].textField, sizeof(menuSystem->items[itemId].textField), "Level up: %d", menu->doomRpg->player->level);
				menuSystem->paintMenu = true;
				break;
			case 7:
				Game_givemap(menu->doomRpg->game);
				break;
			default:
				return MENU_NONE;
				break;
			}

			return MENU_DEBUG_CHEATS;
			break;
		}

		case MENU_DEBUG_STATS: {
			return MENU_DEBUG;
			break;
		}

		case MENU_STORE_CONFIRM: {
			if (menuSystem->items[itemId].action != 0) {
				return MENU_NONE;
			}
			menu->f708b = 0;
			menu->f702a = 0;
			return MENU_STORE;
			break;
		}

		case MENU_STORE: {
			if (itemId == 0) {
				return MENU_NONE;
			}

			menu->f708b = menuSystem->selectedIndex;
			menu->f702a = menuSystem->scrollIndex;
			menu->mArg0 = vendingMenuTable[(MAXSTORELINES * menuSystem->f749g) + STORE_NAME_TAG  +  menuSystem->selectedIndex];
			menu->mArg1 = menuSystem->items[menuSystem->selectedIndex].action;
			menu->mArg2 = vendingMenuTable[(MAXSTORELINES * menuSystem->f749g) + STORE_PRICE_TAG + menuSystem->selectedIndex];
			return MENU_STORE_BUY;
			break;
		}

		case MENU_STORE_BUY: {
			if (itemId == 2) {
				int i9 = menu->mArg1 & 0xff;
				int i10 = (unsigned int)(menu->mArg1 << 8) >> 0x18;

				Player_addCredits(menu->doomRpg->player, - menu->mArg2);

				if (i9 == 1) {
					Player_addItem(menu->doomRpg->player, i10, 1);
				}
				else if (i9 == 2) {
					if (i10 == 3) {
						Player_addAmmo(menu->doomRpg->player, i10, 3);
					}
					else {
						Player_addAmmo(menu->doomRpg->player, i10, 10);
					}
				}
				else if (i9 == 3) {
					switch (i10) {
					case 1:
						CombatEntity_setDefense(&menu->doomRpg->player->ce, CombatEntity_getDefense(&menu->doomRpg->player->ce) + 1);
						break;
					case 2:
						CombatEntity_setAgility(&menu->doomRpg->player->ce, CombatEntity_getAgility(&menu->doomRpg->player->ce) + 1);
						break;
					case 3:
						CombatEntity_setStrength(&menu->doomRpg->player->ce, CombatEntity_getStrength(&menu->doomRpg->player->ce) + 1);
						break;
					case 4:
						CombatEntity_setAccuracy(&menu->doomRpg->player->ce, CombatEntity_getAccuracy(&menu->doomRpg->player->ce) + 1);
						break;
					}
				}
			}
			return MENU_STORE;
			break;
		}

		case MENU_DEVELOPER: {
			switch (itemId) {
			case 0:
				return menuSystem->oldMenu;
			case 1:
				return MENU_DEVELOPER_VARS;
			case 2:
				DoomCanvas_startSpeedTest(doomCanvas, false);
				return MENU_NONE;
			case 3:
			case 4:
			case 5:
			case 6:
				menuSystem->f749g = itemId - 3;
				return MENU_STORE_CONFIRM;
			case 7:
				DoomRPG_Error("Menu.select: Forced!");
				//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Menu.select:", "Forced!", sdlVideo.window);
				return MENU_NONE;

			case 8:
				return MENU_NONE;
			case 9:
				return MENU_NONE;
			}
			break;
		}

		case MENU_INGAME_LOAD: {
			if(itemId != 2) {
				return MENU_INGAME;
			}
			DoomCanvas_loadState(menu->doomRpg->doomCanvas, 1, "Loading Game...");
			Hud_addMessage(menu->doomRpg->hud, "Game Loaded");
			break;

		case MENU_INGAME_LOADNOSAVE:
			return menuSystem->oldMenu;
			break;


		// NEW MENU OPTIONS
		case MENU_VIDEO:
		case MENU_INGAME_VIDEO:

			if (itemId == 0) {
				return menuSystem->oldMenu;
			}
			else if (itemId == 1) { // New Full Screen Option
				sdlVideo.fullScreen ^= true;
				strncpy(menuSystem->items[itemId].textField2, sdlVideo.fullScreen ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));

				SDL_ShowCursor(sdlVideo.fullScreen ? SDL_DISABLE : SDL_ENABLE);
				SDL_SetWindowFullscreen(sdlVideo.window, sdlVideo.fullScreen ? SDL_WINDOW_FULLSCREEN : 0);
			}
			else if (itemId == 2) { // New vSync Option
				sdlVideo.vSync ^= true;
				strncpy(menuSystem->items[itemId].textField2, sdlVideo.vSync ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));

				SDL_SetHint(SDL_HINT_RENDER_VSYNC, sdlVideo.vSync ? "1" : "0");
			}
			else if (itemId == 3) { // New integer scaling Option
				sdlVideo.integerScaling ^= true;
				strncpy(menuSystem->items[itemId].textField2, sdlVideo.integerScaling ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));

				SDL_RenderSetIntegerScale(sdlVideo.renderer, sdlVideo.integerScaling);
			}
			else if (itemId == 5) { // New resolution Option
				sdlVideo.resolutionIndex++;
				if (sdlVideo.resolutionIndex >= (sizeof(sdlVideoModes) / sizeof(SDLVidModes_t))) {
					sdlVideo.resolutionIndex = 0;
				}
				SDL_snprintf(text, sizeof(text), "(%dx%d)", sdlVideoModes[sdlVideo.resolutionIndex].width, sdlVideoModes[sdlVideo.resolutionIndex].height);
				strncpy(menuSystem->items[itemId].textField, text, sizeof(menuSystem->items[itemId].textField));
			}
			else if (itemId == 7) { // New display SoftKeys Option
				sdlVideo.displaySoftKeys ^= true;
				strncpy(menuSystem->items[itemId].textField2, sdlVideo.displaySoftKeys ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));
			}
			else if (itemId == 8) { // New display SoftKeys Option
				menu->doomRpg->doomCanvas->renderFloorCeilingTextures ^= true;
				strncpy(menuSystem->items[itemId].textField2, menu->doomRpg->doomCanvas->renderFloorCeilingTextures ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));
			}

			Game_saveConfig(menu->doomRpg->game, 0);
			menuSystem->paintMenu = true;
			return menuSystem->menu;
			break;

			// NEW MENU OPTIONS
		case MENU_INPUT:
		case MENU_INGAME_INPUT:

			if (itemId == 0) { // Back
				return menuSystem->oldMenu;
			}
			else if (itemId == 1) { // Bindings
				return (menuSystem->type == 1) ? MENU_INGAME_BINDINGS : MENU_BINDINGS;
			}
			else if (itemId == 2) { // Mouse
				return (menuSystem->type == 1) ? MENU_INGAME_MOUSE : MENU_MOUSE;
			}
			else if (itemId == 3) { // Controller
				return (menuSystem->type == 1) ? MENU_INGAME_CONTROLLER : MENU_CONTROLLER;
			}

			return menuSystem->menu;
			break;

			// NEW MENU OPTIONS
		case MENU_SOUND:
		case MENU_INGAME_SOUND:

			if (itemId == 0) { // Back
				// Apply changes
				Game_saveConfig(menuSystem->doomRpg->game, 0);
				return menuSystem->oldMenu;
			}
			if (itemId == 1) {

				doomRpg->sound->soundEnabled ^= true;

				if (doomRpg->sound->soundEnabled) {
					if (menuSystem->oldMenu == MENU_INGAME) {
						MenuSystem_playSound(menuSystem);
					}
					else {
						Sound_playSound(doomRpg->sound, 5040, SND_FLG_LOOP | SND_FLG_STOPSOUNDS | SND_FLG_ISMUSIC, 5);
					}
					strncpy(menuSystem->items[itemId].textField2, "on", sizeof(menuSystem->items[itemId].textField2));

					MenuItem_Set2(&menuSystem->items[itemId + 1], "Volume:", "", 0, 0);

					SDL_snprintf(menuSystem->items[itemId + 1].textField2,
						sizeof(menuSystem->items[itemId + 1].textField2), "%d%%", (doomRpg->sound->volume * 100) / 100);


					MenuItem_Set2(&menuSystem->items[itemId + 2], "Priority:", "", 0, 0);
					strncpy(menuSystem->items[itemId + 2].textField2, menu->doomRpg->doomCanvas->sndPriority ? "on" : "off", sizeof(menuSystem->items[itemId + 2].textField2));
				}
				else {
					strncpy(menuSystem->items[itemId].textField2, "off", sizeof(menuSystem->items[itemId].textField2));
					Sound_stopSounds(doomRpg->sound);
					Sound_freeSounds(doomRpg->sound);
					MenuItem_Set(&menuSystem->items[itemId + 1], NULL, 0, 0);
					MenuItem_Set(&menuSystem->items[itemId + 2], NULL, 0, 0);
				}
			}
			else if (itemId == 2) {

				if (doomRpg->sound->volume == 100) {
					doomRpg->sound->volume = 0;
					Sound_updateVolume(menu->doomRpg->sound);
				}
				else {
					Sound_addVolume(menu->doomRpg->sound, 10);
				}
			}
			else if (itemId == 3) {
				menu->doomRpg->doomCanvas->sndPriority ^= true;
				strncpy(menuSystem->items[itemId].textField2, menu->doomRpg->doomCanvas->sndPriority ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));
			}

			return menuSystem->menu;
			break;

		// NEW MENU OPTIONS
		case MENU_BINDINGS:
		case MENU_INGAME_BINDINGS:

			if (!menuSystem->setBind) {
				if (itemId == 0) {
					// Apply changes
					SDL_memcpy(keyMapping, keyMappingTemp, sizeof(keyMapping));
					Game_saveConfig(menuSystem->doomRpg->game, 0);
					return menuSystem->oldMenu;
				}
				else if (itemId == 27) {
					// Apply changes to default
					SDL_memcpy(keyMappingTemp, keyMappingDefault, sizeof(keyMapping));
				}
				else {
					menuSystem->setBind = true;
					menuSystem->bindIndx = itemId;
				}
			}
			menuSystem->paintMenu = true;
			return menuSystem->menu;
			break;

		// NEW MENU OPTIONS
		case MENU_MOUSE:
		case MENU_INGAME_MOUSE:
			if (itemId == 0) { // Back
				// Apply changes
				Game_saveConfig(menuSystem->doomRpg->game, 0);
				return menuSystem->oldMenu;
			}
			else if (itemId == 1) { // Sensitivity
				menu->doomRpg->doomCanvas->mouseSensitivity += 5;
				if (menu->doomRpg->doomCanvas->mouseSensitivity > 100) {
					menu->doomRpg->doomCanvas->mouseSensitivity = 0;
				}

				SDL_snprintf(menuSystem->items[itemId].textField2,
					sizeof(menuSystem->items[itemId].textField2), "%d%%", (menu->doomRpg->doomCanvas->mouseSensitivity * 100) / 100);
			}
			else if (itemId == 2) { // Y Movement
				menu->doomRpg->doomCanvas->mouseYMove ^= true;
				strncpy(menuSystem->items[itemId].textField2, menu->doomRpg->doomCanvas->mouseYMove ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));
			}
			else if (itemId == 3) { // Reset Defaults
				menu->doomRpg->doomCanvas->mouseSensitivity = 50;
				menu->doomRpg->doomCanvas->mouseYMove = true;

				SDL_snprintf(menuSystem->items[itemId-2].textField2,
					sizeof(menuSystem->items[itemId-2].textField2), "%d%%", (menu->doomRpg->doomCanvas->mouseSensitivity * 100) / 100);

				strncpy(menuSystem->items[itemId-1].textField2, menu->doomRpg->doomCanvas->mouseYMove ? "on" : "off", sizeof(menuSystem->items[itemId].textField2));
			}

			return menuSystem->menu;
			break;

		// NEW MENU OPTIONS
		case MENU_CONTROLLER:
		case MENU_INGAME_CONTROLLER:
			if (itemId == 0) { // Back
				// Apply changes
				Game_saveConfig(menuSystem->doomRpg->game, 0);
				return menuSystem->oldMenu;
			}
			else if (itemId == 1) { // Vibrate

				doomCanvas->vibrateEnabled ^= true;

				if (doomCanvas->vibrateEnabled) {
					DoomCanvas_startShake(doomCanvas, 0, 0, 500);
					strncpy(menuSystem->items[itemId].textField2, "on", sizeof(menuSystem->items[itemId].textField2));
				}
				else {
					strncpy(menuSystem->items[itemId].textField2, "off", sizeof(menuSystem->items[itemId].textField2));
				}
			}
			else if (itemId == 2) { // Death Zone Left
				sdlController.deadZoneLeft += 5;
				if (sdlController.deadZoneLeft > 100) {
					sdlController.deadZoneLeft = 5;
				}

				SDL_snprintf(menuSystem->items[itemId].textField2,
					sizeof(menuSystem->items[itemId].textField2), "%d%%", (sdlController.deadZoneLeft * 100) / 100);
			}
			else if (itemId == 3) { // Death Zone Right
				sdlController.deadZoneRight += 5;
				if (sdlController.deadZoneRight > 100) {
					sdlController.deadZoneRight = 5;
				}

				SDL_snprintf(menuSystem->items[itemId].textField2,
					sizeof(menuSystem->items[itemId].textField2), "%d%%", (sdlController.deadZoneRight * 100) / 100);
			}
			else if (itemId == 4) { // Reset Defaults
				doomCanvas->vibrateEnabled = true;
				sdlController.deadZoneLeft = 25;
				sdlController.deadZoneRight = 25;

				SDL_snprintf(menuSystem->items[itemId - 2].textField2,
					sizeof(menuSystem->items[itemId - 2].textField2), "%d%%", (sdlController.deadZoneLeft * 100) / 100);

				SDL_snprintf(menuSystem->items[itemId - 1].textField2,
					sizeof(menuSystem->items[itemId - 1].textField2), "%d%%", (sdlController.deadZoneRight * 100) / 100);
			}

			return menuSystem->menu;
			break;
		}
	}

	return MENU_NONE;
}

void Menu_startGame(Menu_t* menu, int i)
{
	DoomCanvas_t* doomCanvas;
	DoomRPG_t* doomRpg;

	doomRpg = menu->doomRpg;
	doomCanvas = doomRpg->doomCanvas;

	doomRpg->menuSystem->imgBG = NULL;

	if (i) {
		Player_reset(doomRpg->player);
		doomRpg->player->totalDeaths = 0;
		if (!doomCanvas->skipIntro) {
			DoomCanvas_setState(doomCanvas, ST_INTRO);
		}
		else  {
			DoomCanvas_loadMap(doomCanvas, doomCanvas->startupMap);
		}
	}
	else {
		DoomCanvas_loadState(doomCanvas, 1, "Loading Game");
	}
}