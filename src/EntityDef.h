#ifndef ENTITYDEF_H__
#define ENTITYDEF_H__


struct DoomRPG_s;
struct Image_s;


typedef struct EntityDef_s
{
	short tileIndex;
	byte eType;
	byte eSubType;
	char name[16];
	int parm;
} EntityDef_t;

typedef struct EntityDefManager_s
{
	int memory;
	int parm;
	struct EntityDef_s* list;
	short numDefs;
	struct DoomRPG_s* doomRpg;
} EntityDefManager_t;


EntityDefManager_t* EntityDef_init(EntityDefManager_t* entityDef, DoomRPG_t* doomRpg);
void EntityDef_free(EntityDefManager_t* entityDef, boolean freePtr);
EntityDef_t* EntityDef_find(EntityDefManager_t* entDef, byte b, byte b2);
EntityDef_t* EntityDef_lookup(EntityDefManager_t* entDef, int i);
int EntityDef_startup(EntityDefManager_t* entityDef);

#endif
