#ifndef RENDER_H__
#define RENDER_H__

#define MAPNAMESTRLEN 16
#define MAX_CUSTOM_SPRITES 16
#define MAX_DROP_SPRITES 8

#define COLUMN_SCALE_INIT MAXINT
#define COLUMN_SCALE_OCCLUDED (MAXINT - 1)


#define SPRITE_FLAG_HIDDEN 65536

struct DoomRPG_s;
struct Entity_s;
struct Render_s;

/* think_t is a function pointer to a routine to handle an actor */
typedef void (*span_t) (struct Render_s* render, int param_2, int param_3, int param_4, int param_5, int param_6);

typedef struct Vertex_s
{
	int x;
	int y;
	int z;
} Vertex_t;

typedef struct Node_s
{
	short x1;
	short y1;
	short x2;
	short y2;
	int args1;
	int args2;
	struct Sprite_s* sprites;
	int floorHeight;
	int ceilingHeight;
	int floorTexelID;
	int ceilingTexelID;
	struct Node_s* prev;
	struct Node_s* next;
} Node_t;

typedef struct Line_s
{
	Vertex_t vert1;
	Vertex_t vert2;
	short texture;
	int flags;
} Line_t;

typedef struct Sprite_s
{
	int x;
	int y;
	int info;
	byte renderMode;
	int sortZ;
	struct Node_s* node;
	struct Sprite_s* nodeNext;
	struct Sprite_s* viewNext;
	struct Entity_s* ent;
} Sprite_t;

typedef struct Render_s
{
	span_t spanFunction;
	int unk0;
	int screenWidth;
	int screenHeight;
	int halfScreenHeight;
	int halfScreenWidth;
	int fracHalfScreenWidth;
	int fracHalfScreenHeight;
	int viewZOld;
	struct Node_s* nodes;
	int nodesLength;
	struct Line_s* lines;
	int linesLength;
	struct Sprite_s* mapSprites;
	int numMapSprites;
	int numSprites;
	int* tileEvents;
	int numTileEvents;
	int* mapByteCode;
	char** mapStringsIDs;
	int mapStringCount;
	byte mapFlags[1024];
	char mapName[MAPNAMESTRLEN];
	short unk1;
	short mapNameID;
	int loadMapID;
	int introColor;
	int mapSpawnIndex;
	int mapSpawnDir;
	int mapCameraSpawnIndex;
	struct Sprite_s* customSprites[MAX_CUSTOM_SPRITES];
	struct Sprite_s* dropSprites[MAX_DROP_SPRITES];
	int firstDropSprite;
	int unk2;
	int mapMemory;
	int stringMem;
	int eventMem;
	int memory;
	int bitShapeMemory;
	int lineMem;
	int spriteMem;
	int nodeMem;
	int texelMemory;
	int paletteMemory;
	int mappingMemory;
	short* ceilingColor;
	short* floorColor;
	byte floorTex;
	byte ceilingTex;
	byte unk3;
	boolean damageBlend;
	int viewCos_;
	int viewSin_;
	int viewTransX;
	int viewSin;
	int viewCos;
	int viewTransY;
	int sinTable[256];
	boolean skipStretch;
	boolean unk4;
	boolean skipCull;
	boolean skipBSP;
	boolean skipLines;
	boolean unk5;
	boolean skipSprites;
	boolean skipViewNudge;
	int lineCount;
	int lineRasterCount;
	int nodeCount;
	int nodeRasterCount;
	int spriteCount;
	int spriteRasterCount;
	int viewX;
	int viewY;
	int viewZ;
	int viewAngle;
	int* columnScale;
	struct Sprite_s* viewSprites;
	struct Node_s viewNodes;
	int* mapTextureTexels;
	int mapTextureTexelsCount;
	int* mapSpriteTexels;
	int mapSpriteTexelsCount;
	char* ioBuffer;
	int ioBufferPos;
	int screenX;
	int screenY;
	byte* mediaTexels;
	short* mediaPalettes;
	int mediaPalettesLength;
	short* shapeData;
	int* mediaTexelOffsets;
	int* mediaBitShapeOffsets;
	short* mediaTexturesIds;
	short* mediaSpriteIds;
	unsigned short* spanPalettes;
	unsigned short newSpanPalette[16];
	struct Line_s tmpLine;
	byte spanMode;
	int numLines;
	int animFrameTime;
	int currentFrameTime;
	int horizonFrameTime;
	int frameTime;
	struct DoomRPG_s* doomRpg;
	SDL_Rect clipRect;
	SDL_Texture* piDIB;
	int pitch;
	byte* framebuffer;
	//IDIB* piDIB;
	int screenLeft;
	int screenTop;
	int screenRight;
	int screenBottom;
	short* pixels;
	short mediaPlanes[24][64 * 64];
	int planeTexturesCnt;
	int planeTextureIds[24];
	short *planeTextures[1024*2];

	// New [GEC]
	// Needed to avoid buffer overflows like in sector 5
	int textureCnt, spriteCnt;
} Render_t;

Render_t* Render_init(Render_t * render, DoomRPG_t* doomRpg);
void Render_setup(Render_t* render, SDL_Rect* windowRect);
void Render_freeRuntime(Render_t* render);
void Render_free(Render_t* render, boolean freePtr);
int Render_startup(Render_t* render);
void Render_loadPalettes(Render_t* render);
unsigned int Render_make565RGB(Render_t* render, int blue, int green, int red);
unsigned short Render_RGB888_To_RGB565(Render_t* render, int rgb);
void Render_setGrayPalettes(Render_t* render);

boolean Render_loadMappings(Render_t* render);
boolean Render_beginLoadMap(Render_t* render, int mapNameID);
boolean Render_beginLoadMapData(Render_t* render);
boolean Render_loadBitShapes(Render_t* render);
boolean Render_loadTexels(Render_t* render);
int Render_getSTexelOffsets(Render_t* render, int i);
int Render_getSTexelBufferSize(Render_t* render, int i);

void Render_relinkSprite(Render_t* render, Sprite_t* sprite);
void Render_addMapTextures(Render_t* render, int textureId);
void Render_addMapTexture(Render_t* render, int textureIndex);
void Render_addMapSprites(Render_t* render, int spriteId);
void Render_addMapSprite(Render_t* render, int spriteIndex);

void Render_render(Render_t* render, int viewx, int viewy, int viewz, unsigned int viewangle);
void Render_initColumnScale(Render_t* render);
void Render_renderFloorAndCeilingSolidBG(Render_t* render);
void Render_renderFloorAndCeilingBG(Render_t* render);
void Render_drawplane(Render_t* render, int x, int y, short** planeTextures, int cnt);
void Render_spanPlane(Render_t* render, int x, int y, short** planeTextures, int param_5, int param_6, int param_7, int param_8, int cnt);
void Render_renderBSP(Render_t* render);
void Render_renderBSPNoclip(Render_t* render);
void Render_walkNode(Render_t* render, int i);
boolean Render_cullBoundingBox(Render_t* render, Node_t* node);
void Render_transform2DVerts(Render_t* render, Vertex_t* vert);
boolean Render_clipLine(Render_t* render, Line_t* line);
void Render_clipVertex(Render_t* render, Vertex_t* vert, Line_t* line, int i, int i2);
void Render_projectVertex(Render_t* render, Vertex_t* vert);
void Render_drawLines(Render_t* render, Line_t* line);
void Render_drawLine(Render_t* render, Line_t* line);
void Render_occludeClippedLine(Render_t* render, Line_t* line);
void Render_drawNodeLines(Render_t* render, Node_t* node);
void Render_drawWallSpans(Render_t* render, Line_t* line);
void Render_renderSpriteObject(Render_t* render, Sprite_t* sprite);
void Render_renderSprite(Render_t* render, int x, int y, int mediaId, int info, byte renderMode);
void Render_drawSpriteSpan(Render_t* render, Line_t* line);
void Render_getSpanMode(Render_t* render, int palOffset, byte spanMode);
void Render_SpanMode0(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode1(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode2(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode3(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode4(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode5(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode6(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode7(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode8(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);
void Render_SpanMode9(Render_t* render, int param_2, int param_3, int param_4, int param_5, int param_6);

void Render_draw2DSprite(Render_t* render, int weaponFrame, int flashFrame, int x, int y, byte renderMode, boolean damageBlend);

void Render_fadeScreen(Render_t* render, int fade);
void Render_setBerserkColor(Render_t* render);

int Render_findEventIndex(Render_t* render, int i);

void Render_renderFloorAndCeilingBG_Test(Render_t* render);
void Render_drawPlane_Test(Render_t* render, int x, int y, int planeTexture, int cnt);
void Render_spanPlane_Test(Render_t* render, int x, int y, int planeTexture, int param_5, int param_6, int param_7, int param_8, int cnt);

#endif
