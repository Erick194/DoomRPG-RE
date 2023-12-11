#ifndef PARTICLESYSTEM_H__
#define PARTICLESYSTEM_H__

struct DoomRPG_s;
struct Image_s;
struct Player_s;

typedef struct ParticleNode_s
{
	int particleStartX;
	int particleStartY;
	int particleVelX;
	int particleVelY;
	int particleSize;
	int particleGravity;
	struct ParticleNode_s* next;
	struct ParticleNode_s* prev;
} ParticleNode_t;

typedef struct ParticleSystem_s
{
	struct DoomRPG_s* doomRpg;
	int memory;
	struct Image_s imgGibs;
	struct Image_s imgGibsBerserk; // New
	int gibSizeWH;
	struct ParticleNode_s nodeListA;
	struct ParticleNode_s nodeListB;
	struct ParticleNode_s nodeListC[64];
	int dispX;
	int dispY;
	int dispWidth;
	int dispHeight;
	int f547c;
	int centreX;
	int centreY;
	int minVelX;
	int maxVelX;
	int minVelY;
	int maxVelY;
	int minStartX;
	int maxStartX;
	int minStartY;
	int maxStartY;
	int minSize;
	int maxSize;
	int color;
	int gravity;
	byte f543a;
	int startTime;
	int endTime;
	int scale;
	int scaleSize;
	int particleCount;
	int scaleX;
	int scaleY;
} ParticleSystem_t;

ParticleSystem_t* ParticleSystem_init(ParticleSystem_t* particleSystem, DoomRPG_t* doomRpg);
void ParticleSystem_free(ParticleSystem_t* particleSystem, boolean freePtr);
void ParticleSystem_linkParticle(ParticleSystem_t* particleSystem, ParticleNode_t* particleNode);
void ParticleSystem_unlinkParticle(ParticleSystem_t* particleSystem, ParticleNode_t* particleNode);
void ParticleSystem_calculateScales(ParticleSystem_t* particleSystem, int i);
void ParticleSystem_freeAllParticles(ParticleSystem_t* particleSystem);
void ParticleSystem_render(ParticleSystem_t* particleSystem, int z);
void ParticleSystem_spawnParticles(ParticleSystem_t* particleSystem, int i, int i2, int z);
void ParticleSystem_calculateParticles(ParticleSystem_t* particleSystem, int i, int z);
void ParticleSystem_startup(ParticleSystem_t* particleSystem);

#endif
