#ifndef ENTITY_H__
#define ENTITY_H__

struct DoomRPG_s;
struct EntityDef_s;
struct EntityMonster_s;

typedef struct Entity_s
{
	struct EntityDef_s* def;
	struct Entity_s* nextOnTile;
	struct Entity_s* prevOnTile;
	short linkIndex;
	short pad;
	int info;
	int visitOrderCount;
	int visitDist[4];
	int visitOrder[4];
	struct EntityMonster_s* monster;
	struct DoomRPG_s* doomRpg;
} Entity_t;

void Entity_attack(Entity_t* entity);
boolean Entity_aiGoal_MOVE(Entity_t* entity, int srcX, int srcY, int destX, int destY);
Entity_t* Entity_powerCouplingDied(void* gameStruct);
void Entity_died(Entity_t* entity);
int Entity_calcWorldDistance(Entity_t* entity, int x, int y);
void Entity_spawnDropItem(Entity_t* entity);
int Entity_calcPath(Entity_t* entity, int srcX, int srcY, int destX, int destY, int i5, int i6);
int Entity_aiMoveToGoal(Entity_t* entity);
void Entity_initspawn(Entity_t* entity);
boolean Entity_checkLineOfSight(Entity_t* entity, int srcX, int srcY, int destX, int destY, int i5);
boolean Entity_renderOnlyStateMonsters(Entity_t* entity);
void Entity_aiThink(Entity_t* entity);
void Entity_setPosition(Entity_t* entity, int x, int y);
void Entity_pain(Entity_t* entity, int i, int i2);
void Entity_reset(Entity_t* entity);
void Entity_touched(Entity_t* entity);
void Entity_trimCorpsePile(Entity_t* entity, int x, int y);


#endif
