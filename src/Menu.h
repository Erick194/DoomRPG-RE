#ifndef MENU_H__
#define MENU_H__


struct DoomRPG_s;

enum menus {
	MENU_NONE = 0,
	MENU_MAIN, 
	MENU_MAIN_HELP_ABOUT,
	MENU_MAIN_EXIT,
	MENU_MAIN_ERASE,
	MENU_MAIN_SURE,
	MENU_MAIN_CONTINUE,
	MENU_MAIN_OPTIONS,
	MENU_VIDEO, // NEW OPTION
	MENU_INPUT, // NEW OPTION
	MENU_SOUND, // NEW OPTION
	MENU_BINDINGS, // NEW OPTION
	MENU_MOUSE, // NEW OPTION
	MENU_CONTROLLER, // NEW OPTION
	MENU_ENABLE_SOUNDS,
	MENU_MAP_STATS,
	MENU_MAP_STATS_OVERALL,
	MENU_INGAME_BEGIN,
	MENU_GOTO_JUNCTION,
	MENU_QUIT_TO_MAIN_MENU,
	MENU_INGAME_NOTEBOOK,
	MENU_INGAME_HELP_ABOUT,
	MENU_NONE2,
	MENU_NONE3,
	MENU_NONE4,
	MENU_NONE5,
	MENU_INGAME,
	MENU_NONE6,
	MENU_INGAME_STATUS,
	MENU_INGAME_OPTIONS,
	MENU_INGAME_EXIT,
	MENU_ITEMS,
	MENU_ITEMS_CONFIRM,
	MENU_INGAME_DEAD,
	MENU_CONFIRM_LOAD,
	MENU_DEBUG,
	MENU_DEVELOPER_VARS,
	MENU_NONE7,
	MENU_DEBUG_MAPS,
	MENU_DEBUG_CHEATS,
	MENU_DEBUG_STATS,
	MENU_STORE_CONFIRM,
	MENU_STORE,
	MENU_STORE_BUY,
	MENU_DEVELOPER,
	MENU_INGAME_SAVE,
	MENU_INGAME_LOAD,
	MENU_INGAME_LOADNOSAVE,
	MENU_INGAME_VIDEO, // NEW OPTION
	MENU_INGAME_INPUT, // NEW OPTION
	MENU_INGAME_SOUND, // NEW OPTION
	MENU_INGAME_BINDINGS, // NEW OPTION
	MENU_INGAME_MOUSE, // NEW OPTION
	MENU_INGAME_CONTROLLER, // NEW OPTION
};


typedef struct Menu_s
{
	struct DoomRPG_s* doomRpg;
	int memory;
	int f702a;
	int f708b;
	int mArg0;
	int mArg1;
	int mArg2;
	short mapNameId;
	byte f701a;
	int f711c;
	int f712d;
	int f713e;
	int f714f;
	int f715g;
} Menu_t;

void Menu_LoadHelpResource(Menu_t* menu);
void Menu_setNotes(Menu_t* menu);
void Menu_setStore(Menu_t* menu);
void Menu_setYesNo(Menu_t* menu, char* str);
void Menu_textVolume(Menu_t* menu, int volume);
Menu_t* Menu_init(Menu_t* menu, DoomRPG_t* doomRpg);
void Menu_fillStatus(Menu_t* menu, int i, int i2, int i3);
void Menu_initMenu(Menu_t* menu, int i);
int Menu_select(Menu_t* menu, int menuId, int itemId);
void Menu_startGame(Menu_t* menu, int i);

#endif
