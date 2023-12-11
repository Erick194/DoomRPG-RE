#ifndef MENUSYSTEM_H__
#define MENUSYSTEM_H__

#include "MenuItem.h"

struct DoomRPG_s;
struct Image_s;

#define MAX_MENUITEMS 96

typedef struct MenuSystem_s
{
	struct DoomRPG_s* doomRpg;
	int memory;
	struct Image_s imgHand;
	struct Image_s imgArrowUpDown;
	struct Image_s imgLogo;
	struct Image_s* imgBG;
	struct MenuItem_s items[MAX_MENUITEMS];
	int numItems;
	int field_0xc58;
	int menu;
	int oldMenu;
	int selectedIndex;
	int scrollIndex;
	int type;
	int maxItems;
	int f749g;
	int cheatCombo;
	int digitCount;
	boolean paintMenu;
	char stringBuffer[32];

	boolean setBind;// new
	int bindIndx;// new
	int nextMsgTime; // New
	int nextMsg;// new

} MenuSystem_t;


MenuSystem_t* MenuSystem_init(MenuSystem_t* menuSystem, DoomRPG_t* doomRpg);
void MenuSystem_free(MenuSystem_t* menuSystem, boolean freePtr);
void MenuSystem_back(MenuSystem_t* menuSystem);
char* MenuSystem_buildDivider(MenuSystem_t* menuSystem, char* str);
void MenuSystem_select(MenuSystem_t* menuSystem);
boolean MenuSystem_checkMenu(MenuSystem_t* menuSystem);
boolean MenuSystem_enterDigit(MenuSystem_t* menuSystem, int n);
void MenuSystem_moveDir(MenuSystem_t* menuSystem, int i);
void MenuSystem_paint(MenuSystem_t* menuSystem);
void MenuSystem_scrollDown(MenuSystem_t* menuSystem);
void MenuSystem_scrollPageDown(MenuSystem_t* menuSystem);
void MenuSystem_scrollPageUp(MenuSystem_t* menuSystem);
void MenuSystem_scrollUp(MenuSystem_t* menuSystem);
void MenuSystem_setMenu(MenuSystem_t* menuSystem, int menu);
void MenuSystem_playSound(MenuSystem_t* menuSystem);

void MenuSystem_startup(MenuSystem_t* menuSystem);

#endif
