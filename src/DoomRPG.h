#ifndef DOOMRPG_H__
#define DOOMRPG_H__

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum { false, true } boolean;
typedef unsigned char byte;
#endif

#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)
#define MAXINT	((int)0x7fffffff)	// max pos 32-bit int 
#define MAXLONG ((long)0x7fffffff)

#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)
#define MININT 	((int)0x80000000)	// max negative 32-bit integer
#define MINLONG ((long)0x80000000)

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,min,max) MIN(max, MAX(min,x))

#ifndef NULL
#define	NULL	0
#endif

#define KEYBINDS_MAX 10
#define IS_MOUSE_BUTTON			0x10000
#define IS_CONTROLLER_BUTTON	0x20000
typedef struct keyMapping_s
{
	int avk_action;
	int keyBinds[KEYBINDS_MAX];
} keyMapping_t;

extern keyMapping_t keyMapping[12];
extern keyMapping_t keyMappingTemp[12];
extern keyMapping_t keyMappingDefault[12];

enum _AVKType {
	AVK_UNDEFINED = 0,	// hex 0xE010; dec 57360
	AVK_FIRST = 1,		// hex 0xE020; dec 57376

	AVK_0,		// hex 0xE021; dec 57377
	AVK_1,		// hex 0xE022; dec 57378
	AVK_2,		// hex 0xE023; dec 57379
	AVK_3,		// hex 0xE024; dec 57380
	AVK_4,		// hex 0xE025; dec 57381
	AVK_5,		// hex 0xE026; dec 57382
	AVK_6,		// hex 0xE027; dec 57383
	AVK_7,		// hex 0xE028; dec 57384
	AVK_8,		// hex 0xE029; dec 57385
	AVK_9,		// hex 0xE02A; dec 57386
	AVK_STAR,	// hex 0xE02B; dec 57387
	AVK_POUND,	// hex 0xE02C; dec 57388

	AVK_POWER,	// hex 0xE02D; dec 57389
	AVK_END,	// hex 0xE02E; dec 57390
	AVK_SEND,	// hex 0xE02F; dec 57391
	AVK_CLR,	// hex 0xE030; dec 57392

	AVK_UP,		// hex 0xE031; dec 57393
	AVK_DOWN,	// hex 0xE032; dec 57394
	AVK_LEFT,	// hex 0xE033; dec 57395
	AVK_RIGHT,	// hex 0xE034; dec 57396
	AVK_SELECT,	// hex 0xE035; dec 57397

	AVK_SOFT1,	// hex 0xE036; dec 57398
	AVK_SOFT2,	// hex 0xE037; dec 57399

	// New Types Only on port
	AVK_MENUOPEN,
	AVK_AUTOMAP,
	AVK_MOVELEFT,
	AVK_MOVERIGHT,
	AVK_PREVWEAPON,
	AVK_NEXTWEAPON,
	AVK_PASSTURN,

	// New Flags Menu Only on port
	AVK_MENU_UP	=			0x40,
	AVK_MENU_DOWN =			0x80,
	AVK_MENU_PAGE_UP =		0x100,
	AVK_MENU_PAGE_DOWN =	0x200,
	AVK_MENU_SELECT =		0x400,
	AVK_MENU_OPEN =			0x800
};

enum _GameController {
	CONTROLLER_BUTTON_INVALID = -1,
	CONTROLLER_BUTTON_A,			// Gamepad A
	CONTROLLER_BUTTON_B,			// Gamepad B
	CONTROLLER_BUTTON_X,			// Gamepad X
	CONTROLLER_BUTTON_Y,			// Gamepad Y
	CONTROLLER_BUTTON_BACK,			// Back
	CONTROLLER_BUTTON_START,		// Start
	CONTROLLER_BUTTON_LEFT_STICK,	// Left Stick
	CONTROLLER_BUTTON_RIGHT_STICK,	// Right Stick
	CONTROLLER_BUTTON_LEFT_BUMPER,	// Left Bumper
	CONTROLLER_BUTTON_RIGHT_BUMPER,	// Right Bumper
	CONTROLLER_BUTTON_DPAD_UP,		// D-Pad Up
	CONTROLLER_BUTTON_DPAD_DOWN,	// D-Pad Down
	CONTROLLER_BUTTON_DPAD_LEFT,	// D-Pad Left
	CONTROLLER_BUTTON_DPAD_RIGHT,	// D-Pad Right
	CONTROLLER_BUTTON_LAXIS_UP,		// L-Axis Up
	CONTROLLER_BUTTON_LAXIS_DOWN,	// L-Axis Down
	CONTROLLER_BUTTON_LAXIS_LEFT,	// L-Axis Left
	CONTROLLER_BUTTON_LAXIS_RIGHT,	// L-Axis Right
	CONTROLLER_BUTTON_RAXIS_UP,		// R-Axis Up
	CONTROLLER_BUTTON_RAXIS_DOWN,	// R-Axis Down
	CONTROLLER_BUTTON_RAXIS_LEFT,	// R-Axis Left
	CONTROLLER_BUTTON_RAXIS_RIGHT,	// R-Axis Right
	CONTROLLER_BUTTON_LEFT_TRIGGER,	// Left Trigger
	CONTROLLER_BUTTON_RIGHT_TRIGGER,// Right Trigger
	CONTROLLER_BUTTON_MAX
};

enum _MOUSE {
	MOUSE_BUTTON_INVALID = -1,
	MOUSE_BUTTON_LEFT,			// Mouse Left
	MOUSE_BUTTON_MIDDLE,		// Mouse Middle
	MOUSE_BUTTON_RIGHT,			// Mouse Right
	MOUSE_BUTTON_X1,			// Mouse X1
	MOUSE_BUTTON_X2,			// Mouse X2
	MOUSE_BUTTON_WHELL_UP,		// Mouse Wheel Up
	MOUSE_BUTTON_WHELL_DOWN,	// Mouse Wheel Down
	MOUSE_BUTTON_MOTION_UP,		// Mouse Motion Up
	MOUSE_BUTTON_MOTION_DOWN,	// Mouse Motion Down
	MOUSE_BUTTON_MOTION_LEFT,	// Mouse Motion Left
	MOUSE_BUTTON_MOTION_RIGHT,	// Mouse MotionRight
	MOUSE_BUTTON_MAX
};

#define LINE_FLAG_RENDER_SPRITE_TWO_SIDED 1 // (internal) rendering both sides of the sprite
#define LINE_FLAG_RENDER_SPRITE 2 // (internal) rendering sprite as line
#define LINE_FLAG_DOORLERP 4 // Door
#define LINE_FLAG_EAST_SOUTH 8 // Shift East or South
#define LINE_FLAG_WEST_NORTH 16 // Shift West or North
#define LINE_FLAG_HIDDEN 32 // Hidden
#define LINE_FLAG_DOOROPEN 64 // (internal) door open/close
#define LINE_FLAG_AUTOMAP_VISIBLE 128  // (internal) automap visible
#define LINE_FLAG_VERTICAL 256 // Shift vertical
#define LINE_FLAG_HORIZONTAL 512 // Shift horizontal
#define LINE_FLAG_DOORLOCKED 1024 // Locked door
#define LINE_FLAG_SOUTH 2048 // South - facing wall
#define LINE_FLAG_NORTH 4096 // North - facing wall
#define LINE_FLAG_WEST 8192 // West - facing wall
#define LINE_FLAG_EAST 16384 // East - facing wall
#define LINE_FLAG_FLIP_HORIZONTAL 32768 // Flip texture horizontally

#define SPRITE_FLAG_HIDDEN 65536 // Hidden
#define SPRITE_FLAG_WALL 131072	// Wall
#define SPRITE_FLAG_TILE 262144	// Wall texture
#define SPRITE_FLAG_NORTH 524288 // North - facing
#define SPRITE_FLAG_SOUTH 1048576 // South - facing
#define SPRITE_FLAG_EAST 2097152 // East - facing
#define SPRITE_FLAG_WEST 4194304 // West - facing
#define SPRITE_FLAG_DECAL 8388608 // Wall decal
#define SPRITE_FLAG_NOENTITY 16777216 // No entity
#define SPRITE_FLAG_FLIP_WALL 33554432 // Flip wall

#define SPRITE_FLAG_FOUR_ITEMS 67108864			// (internal) draw four items
#define SPRITE_FLAG_TWO_SIDED 134217728			// (internal) wall double sided
#define SPRITE_FLAG_AUTOMAP_VISIBLE 268435456	// (internal) automap visible
#define SPRITE_FLAG_0x20000000 536870912		// (internal) 
#define SPRITE_FLAG_0x40000000 1073741824		// (internal) 
#define SPRITE_FLAG_AUTO_ANIMATE 2147483648		// (internal) automatic missile animation

#define SPRITE_FLAGS_ORIENTED (SPRITE_FLAG_NORTH | SPRITE_FLAG_SOUTH | SPRITE_FLAG_EAST | SPRITE_FLAG_WEST)
#define SPRITE_FLAGS_HORIZONTAL = (SPRITE_FLAG_NORTH| SPRITE_FLAG_SOUTH)
#define SPRITE_FLAGS_VERTICAL (SPRITE_FLAG_EAST | SPRITE_FLAG_WEST)

// Doom Canvas
#define BIT_AM_WALL 1
#define BIT_AM_SECRET 2
#define BIT_AM_ENTRANCE 4
#define BIT_AM_EVENTS 8
#define BIT_AM_VISITED 16

#define BYTE_CODE_ID 0
#define BYTE_CODE_ARG1 1
#define BYTE_CODE_ARG2 2
#define BYTE_CODE_MAX 3

#define CHANGEMAP_FADE_BIT = 128;
#define CHANGEMAP_SHOWSTAT_SHIFT 31
#define CHANGEMAP_SHOWSTATS_BIT (1 << CHANGEMAP_SHOWSTAT_SHIFT)

// Game
#define EVENT_FLAG_BLOCKINPUT 1
#define MCODE_EXEC_ENTER_NORTH 1
#define MCODE_EXEC_ENTER_EAST 2
#define MCODE_EXEC_ENTER_SOUTH 4
#define MCODE_EXEC_ENTER_WEST 8
#define MCODE_EXEC_EXIT_SOUTH 16
#define MCODE_EXEC_EXIT_WEST 32
#define MCODE_EXEC_EXIT_NORTH 64
#define MCODE_EXEC_EXIT_EAST 128
#define MCODE_EXEC_TRIGGER 256
#define MCODE_FLAG_REMOVE 512

#define	FRACBITS    16
#define	FRACUNIT    (1<<FRACBITS)
#define	FRACMASK    (FRACUNIT-1)    // Named from PsyDoom (StationDoom)

typedef int fixed_t;

#define	RANDTABLESIZE	128

typedef struct Random_s
{
	byte	randTable[RANDTABLESIZE];
	int		nextRand;
} Random_t;

struct DoomCanvas_s;
struct Render_s;
struct Player_s;
struct Game_s;
struct EntityDefManager_s;
struct Combat_s;
struct Hud_s;
struct Menu_s;
struct MenuSystem_s;
struct ParticleSystem_s;
struct Sound_s;

typedef struct DoomRPG_s
{
	int				upTimeMs;
	struct Random_s	random;
	struct DoomCanvas_s*	doomCanvas;
	struct Render_s*		render;
	struct Player_s*		player;
	struct Game_s*			game;
	struct EntityDefManager_s*		entityDef;
	struct Combat_s*		combat;
	struct Hud_s*			hud;
	struct Menu_s*			menu;
	struct MenuSystem_s*	menuSystem;
	struct ParticleSystem_s* particleSystem;
	struct Sound_s*			sound;

	boolean			graphSetCliping;
	int				memoryBeg;
	int				errorID;
	char			errorStr[64];
	boolean			closeApplet;
	int				imageMemory;
} DoomRPG_t;

typedef struct Image_s
{
	SDL_Texture* imgBitmap;
	int	width;
	int	height;
	boolean isTransparentMask;
} Image_t;

//extern DoomRPG_t *doomRpg;

fixed_t	DoomRPG_FixedMul(fixed_t a, fixed_t b);
fixed_t	DoomRPG_FixedDiv(fixed_t a, fixed_t b);
fixed_t	DoomRPG_FixedDiv2(fixed_t a, fixed_t b);

void DoomRPG_Error(const char* fmt, ...);
unsigned int DoomRPG_GetTimeMS(void);
unsigned int DoomRPG_GetUpTimeMS(void);
int DoomRPG_freeMemory(void);

int DoomRPG_getEventKey(int mouse_Button, const Uint8* state);
void DoomRPG_setDefaultBinds(DoomRPG_t* doomrpg);
void DoomRPG_setBind(DoomRPG_t* doomrpg, int mouse_Button, const Uint8* state);


int DoomRPG_Init(void);
void DoomRPG_FreeAppData(DoomRPG_t* doomrpg);

void DoomRPG_createImage(DoomRPG_t* doomrpg, const char* resourceName, boolean isTransparentMask, Image_t* img);
void DoomRPG_createImageBerserkColor(DoomRPG_t* doomrpg, const char* resourceName, boolean isTransparentMask, Image_t* img);
void DoomRPG_freeImage(DoomRPG_t* doomrpg, Image_t* image);

byte* DoomRPG_fileOpenRead(DoomRPG_t* doomrpg, const char* resourceName);
void DoomRPG_setErrorID(DoomRPG_t* doomrpg, int ID);
int DoomRPG_getErrorID(DoomRPG_t* doomrpg);
byte DoomRPG_byteAtNext(byte* data, int* posData);
byte DoomRPG_byteAt(byte* data, int posData);
short DoomRPG_shortAtNext(byte* data, int* posData);
short DoomRPG_shortAt(byte* data, int posData);
int DoomRPG_intAtNext(byte* data, int* posData);
int DoomRPG_intAt(byte* data, int posData);
short DoomRPG_shiftCoordAt(byte* data, int* posData);

void DoomRPG_loopGame(DoomRPG_t* doomrpg);

void DoomRPG_setClipFalse(DoomRPG_t* doomrpg);
void DoomRPG_setClipTrue(DoomRPG_t* doomrpg, int x, int y, int w, int h);
void DoomRPG_setColor(DoomRPG_t* doomrpg, int color);
void DoomRPG_flushGraphics(DoomRPG_t* doomrpg);
void DoomRPG_clearGraphics(DoomRPG_t* doomrpg);
void DoomRPG_drawRect(DoomRPG_t* doomrpg, int x, int y, int w, int h);
void DoomRPG_fillRect(DoomRPG_t* doomrpg, int x, int y, int w, int h);
void DoomRPG_drawCircle(DoomRPG_t* doomrpg, int x, int y, int r);
void DoomRPG_fillCircle(DoomRPG_t* doomrpg, int x, int y, int r);
void DoomRPG_drawLine(DoomRPG_t* doomrpg, int x1, int y1, int x2, int y2);
void DoomRPG_setFontColor(DoomRPG_t* doomrpg, int color); // New

void DoomRPG_setRand(Random_t* rand);
byte DoomRPG_randNextByte(Random_t* rand);
int DoomRPG_randNextInt(Random_t* rand);

void DoomRPG_notifyDestroyed(DoomRPG_t* doomrpg);

void File_writeBoolean(SDL_RWops* rw, int i);
void File_writeByte(SDL_RWops* rw, int i);
void File_writeShort(SDL_RWops* rw, int i);
void File_writeInt(SDL_RWops* rw, int i);
void File_writeLong(SDL_RWops* rw, int i);

boolean File_readBoolean(SDL_RWops* rw);
int File_readByte(SDL_RWops* rw);
int File_readShort(SDL_RWops* rw);
int File_readInt(SDL_RWops* rw);
int File_readLong(SDL_RWops* rw);

#endif
