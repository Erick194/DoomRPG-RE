#ifndef GAME_H__
#define GAME_H__

#include "Render.h"
#include "Entity.h"
#include "EntityMonster.h"

typedef enum
{
	MAP_MENU = 0,
	MAP_INTRO,
	MAP_SECTOR01,
	MAP_SECTOR02,
	MAP_SECTOR03,
	MAP_SECTOR04,
	MAP_SECTOR05,
	MAP_SECTOR06,
	MAP_SECTOR07,
	MAP_JUNCTION,
	MAP_JUNCTION_DESTROYED,
	MAP_ITEMS,
	MAP_REACTOR,
	MAP_END_GAME,
	MAP_MAX,

	MAPFILE_INTRO = 0,
	MAPFILE_L01,
	MAPFILE_L02,
	MAPFILE_L03,
	MAPFILE_L04,
	MAPFILE_L05,
	MAPFILE_L06,
	MAPFILE_L07,
	MAPFILE_JUNCTION,
	MAPFILE_JUNCTION_DESTROYED,
	MAPFILE_ITEMS,
	MAPFILE_REACTOR,
	MAPFILE_END_GAME,
	MAPFILE_MAX,

	MAPNAME_ENTRANCE = 0,
	MAPNAME_JUNCTION,
	MAPNAME_S01,
	MAPNAME_S02,
	MAPNAME_S03,
	MAPNAME_S04,
	MAPNAME_S05,
	MAPNAME_S06,
	MAPNAME_S07,
	MAPNAME_JUNCTION_DESTROYED,
	MAPNAME_REACTOR,
	MAPNAME_MAX
} gameMaps_e;

#define EV_GOTO			1
#define EV_CHANGEMAP	2
#define EV_TRIGGER		3
#define EV_MESSAGE		4
#define EV_PAIN			5
#define EV_MOVELINE		6
#define EV_SHOW			7
#define EV_DIALOG		8
#define EV_GIVEMAP		9
#define EV_PASSWORD		10
#define EV_CHANGESTATE	11
#define EV_LOCK			12
#define EV_UNLOCK		13
#define EV_TOGGLELOCK	14
#define EV_OPENLINE		15
#define EV_CLOSELINE	16
#define EV_MOVELINE2	17
#define EV_HIDE			18
#define EV_NEXTSTATE	19
#define EV_PREVSTATE	20
#define EV_INCSTAT		21
#define EV_DECSTAT		22
#define EV_REQSTAT		23
#define EV_FORCEMESSAGE	24
#define EV_ANIM			25
#define EV_DIALOGNOBACK	26
#define EV_SAVEGAME		27
#define EV_ABORTMOVE	28
#define EV_SCREENSHAKE	29
#define EV_CHANGEFLOORCOLOR	30
#define EV_CHANGECEILCOLOR	31
#define EV_ENABLEWEAPONS	32
#define EV_OPENSTORE		33
#define EV_CHANGESPRITE		34
#define EV_SPAWNPARTICLES	35
#define EV_REFRESHVIEW		36
#define EV_WAIT				37
#define EV_ACTIVE_PORTAL	38
#define EV_CHECK_COMPLETED_LEVEL	39
#define EV_NOTE				40
#define EV_CHECK_KEY		41
#define EV_PLAYSOUND		42

struct DoomRPG_s;
struct Entity_s;
struct EntityMonster_s;
struct Sprite_s;
struct Image_s;

typedef struct GameSprite_s
{
	int index;
	int frame;
	int time;
	int flags;
	struct Sprite_s *sprite;
} GameSprite_t;

typedef struct GameSpriteMissile_s
{
	struct GameSprite_s* gsMissile;
	int posX;
	int posY;
	boolean unk0;
} GameSpriteMissile_t;

typedef struct Game_s
{
	struct Entity_s entities[400];
	int numEntities;
	struct Entity_s* entityDb[1024];
	struct Entity_s* traceEntities[8];
	int numTraceEntities;
	struct EntityMonster_s entityMonsters[100];
	int numMonsters;
	int spawnParam;
	boolean disableAI;
	struct Entity_s* inactiveMonsters;
	struct Entity_s* activeMonsters;
	struct Entity_s* combatMonsters;
	int waitTime;
	int firstSpecialEntityIndex;
	int lastSpecialEntityIndex;
	int firstDropIndex;
	int dropIndex;
	boolean f658b;
	boolean skipAdvanceTurn;
	char* passCode;
	int tileEvent;
	byte saveTileEvent;
	int tileEventIndex;
	int tileEventFlags;
	boolean monstersTurn;
	boolean ignoreMonsterAI;
	boolean interpolatingMonsters;
	boolean isSaved;
	boolean isLoaded;
	int eventFlags[2];
	int memory;
	int entityMemory;
	struct GameSprite_s gsprites[MAX_CUSTOM_SPRITES];
	int gSpriteDurationTime;
	int changeMapParam;
	struct Entity_s* spawnMonster;
	boolean activePortal;
	int spawnCount;
	int powerCouplingHealth[2];
	int powerCouplingIndex;
	int powerCouplingDeaths;
	byte activeSprites;
	byte f684l;
	int newDestX;
	int newDestY;
	int newAngle;
	char newMapName[32];
	char fileMapName[32];
	int activeLoadType;
	struct Entity_s* soundMonster;
	struct DoomRPG_s* doomRpg;

	char mapNames[MAPNAME_MAX][24];
	char mapFiles[MAPFILE_MAX][24];
} Game_t;


int Game_getResourceMapID(Game_t* game, char* mapName);
Game_t* Game_init(Game_t* game, DoomRPG_t* doomRpg);
void Game_activate(Game_t* game, Entity_t* entity);
void Game_advanceTurn(Game_t* game);
GameSprite_t* Game_gsprite_allocAnim(Game_t* game, int index, int x, int y);
void Game_gsprite_clear(Game_t* game);
void Game_gsprite_update(Game_t* game);
void Game_hurtEntityAt(Game_t* game, int i, int i2, int i3, int i4, int z, int z2);
void Game_deactivate(Game_t* game, Entity_t* entity);
void Game_radiusHurtEntities(Game_t* game, int i, int i2, int i3, int i4, int z, int z2);
void Game_endMonstersTurn(Game_t* game);
Entity_t* Game_getEntityByIndex(Game_t* game, int indx);
void Game_eventFlagsForMovement(Game_t* game, int i, int i2, int i3, int i4);
Entity_t* Game_findMapEntityXYFlag(Game_t* game, int x, int y, int flags);
Entity_t* Game_findMapEntityXY(Game_t* game, int x, int y);
void Game_changeMap(Game_t* game);
void Game_givemap(Game_t* game);
boolean Game_checkConfigVersion(Game_t* game);
int Game_findEntity(Game_t* game, Entity_t* entity);
void Game_linkEntity(Game_t* game, Entity_t* entity, int x, int y);
void Game_loadConfig(Game_t* game);
void Game_loadMapEntities(Game_t* game);
void Game_loadPlayerState(Game_t* game, char* fileName);
void Game_loadState(Game_t* game, int i);
void Game_loadWorldState(Game_t* game);
void Game_monsterAI(Game_t* game);
void Game_monsterLerp(Game_t* game);
boolean Game_performDoorEvent(Game_t* game, int codeId, int arg1, int flags);
void Game_deleteSaveFiles(Game_t* game);
void Game_remove(Game_t* game, Entity_t* entity);
boolean Game_executeEvent(Game_t* game, int event, int codeId, int arg1, int arg2, int flags);
boolean Game_executeTile(Game_t* game, int x, int y, int flags);
boolean Game_runEvent(Game_t* game, int event, int index, int flags);
void Game_saveConfig(Game_t* game, int num);
void Game_savePlayerState(Game_t* game, char* fileName, char* fileMapName, int x, int y, int angle);
void Game_saveState(Game_t* game, int mapId, int x, int y, int angleDir, boolean z);
void Game_saveWorldState(Game_t* game);
void Game_setLineLocked(Game_t* game, int index, boolean z, boolean z2);
boolean Game_snapMonsters(Game_t* game);
void Game_updateSpawnPortals(Game_t* game);
void Game_spawnPlayer(Game_t* game);
GameSprite_t* Game_gsprite_alloc(Game_t* game, int amin, int frame, int x, int y);
boolean Game_touchTile(Game_t* game, int x, int y, int touched);
void Game_trace(Game_t* game, int srcX, int srcY, int destX, int destY, Entity_t* entity, int flags);
void Game_unlinkEntity(Game_t* game, Entity_t* entity);
void Game_unloadMapData(Game_t* game);
boolean Game_updateMonsters(Game_t* game);

#endif
