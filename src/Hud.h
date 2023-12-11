#ifndef HUD_H__
#define HUD_H__

#define SCROLL_START_DELAY 750
#define MSG_DISPLAY_TIME 1200
#define MS_PER_CHAR 64
#define MAX_MESSAGES 5

struct DoomRPG_s;
struct Image_s;

typedef struct Hud_s
{
	int memory;
	short statusBarHeight;
	short statusTopBarHeight;
	int msgMaxChars;
	struct Image_s imgStatusBar;
	struct Image_s imgStatusBarLarge;
	struct Image_s imgIconSheet;
	struct Image_s imgHudFaces;
	struct Image_s imgAttArrow;
	struct Image_s imgStatusArrow;
	int hudFaceWidth;
	int hudFaceHeight;
	int iconSheetWidth;
	int iconSheetHeight;
	int statusHealthXpos;
	int statusArmorXpos;
	int statusHudFacesXpos;
	int unk1;
	int statusAmmoXpos;
	int statusOrientationXpos;
	int statusOrientationArrowXpos;
	int statusLine1Xpos;
	int statusLine2Xpos;
	boolean largeHud;
	char messages[MAX_MESSAGES][MS_PER_CHAR];
	int msgCount;
	int msgTime;
	int msgDuration;
	char logMessage[MS_PER_CHAR];
	char* statBarMessage;
	int gotFaceTime;
	int damageTime;
	int damageCount;
	int damageDir;
	boolean isUpdate;
	char healthNum[4];
	char armorNum[4];
	char ammoNum[3];
	struct DoomRPG_s* doomRpg;
} Hud_t;

Hud_t* Hud_init(Hud_t* hud, DoomRPG_t* doomRpg);
void Hud_free(Hud_t* hud, boolean freePtr);
void Hud_addMessage(Hud_t* hud, char* str);
void Hud_addMessageForce(Hud_t* hud, char* str, boolean force);
void Hud_calcMsgTime(Hud_t* hud);
void Hud_drawBarTiles(Hud_t* hud, int x, int y, int width, boolean isLargerStatusBar);
void Hud_drawBottomBar(Hud_t* hud);
void Hud_drawEffects(Hud_t* hud);
void Hud_drawTopBar(Hud_t* hud);
void Hud_finishMessageBufferForce(Hud_t* hud, boolean force);
void Hud_finishMessageBuffer(Hud_t* hud);
char* Hud_getMessageBufferForce(Hud_t* hud, boolean force);
char* Hud_getMessageBuffer(Hud_t* hud);
void Hud_shiftMsgs(Hud_t* hud);
void Hud_startup(Hud_t* hud, boolean largeStatus);

#endif
