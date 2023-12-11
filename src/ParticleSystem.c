
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "ParticleSystem.h"
#include "Render.h"
#include "DoomCanvas.h"
#include "Player.h"
#include "Hud.h"
#include "SDL_Video.h"

int recalcScales(int val1, int val2) { // New code
	int div = (sdlVideoModes[sdlVideo.resolutionIndex].width / 128);
	return val1 + (val1 - (((val1 << 16) / (val2 << 8)) / div));
}

ParticleSystem_t* ParticleSystem_init(ParticleSystem_t* particleSystem, DoomRPG_t* doomRpg)
{

	printf("ParticleSystem_init\n");

	if (particleSystem == NULL)
	{
		particleSystem = SDL_malloc(sizeof(ParticleSystem_t));
		if (particleSystem == NULL) {
			return NULL;
		}
	}
	SDL_memset(particleSystem, 0, sizeof(ParticleSystem_t));

	particleSystem->doomRpg = doomRpg;

	return particleSystem;
}

void ParticleSystem_free(ParticleSystem_t* particleSystem, boolean freePtr)
{
	DoomRPG_freeImage(particleSystem->doomRpg, &particleSystem->imgGibs);
	DoomRPG_freeImage(particleSystem->doomRpg, &particleSystem->imgGibsBerserk); // New
	if (freePtr) {
		SDL_free(particleSystem);
	}
}

void ParticleSystem_linkParticle(ParticleSystem_t* particleSystem, ParticleNode_t* particleNode)
{
	ParticleNode_t* nextNode;

	if (particleSystem->particleCount == 0) {
		particleSystem->startTime = particleSystem->doomRpg->doomCanvas->time;
	}
	++particleSystem->particleCount;

	particleNode->prev->next = particleNode->next;
	particleNode->next->prev = particleNode->prev;
	nextNode = (particleSystem->nodeListA).next;
	particleNode->prev = &particleSystem->nodeListA;
	particleNode->next = nextNode;
	nextNode->prev = particleNode;
	particleNode->prev->next = particleNode;
}

void ParticleSystem_unlinkParticle(ParticleSystem_t* particleSystem, ParticleNode_t* particleNode)
{
	ParticleNode_t* nextNode;

	--particleSystem->particleCount;

	particleNode->prev->next = particleNode->next;
	particleNode->next->prev = particleNode->prev;
	nextNode = (particleSystem->nodeListB).next;
	particleNode->prev = &particleSystem->nodeListB;
	particleNode->next = nextNode;
	nextNode->prev = particleNode;
	particleNode->prev->next = particleNode;
}

void ParticleSystem_calculateScales(ParticleSystem_t* particleSystem, int i)
{
	int cnt;
	int w, h;

	w = 235; // (128 * 2) - 21
	h = 174; // (88 * 2) - 2

	//w = (recalcScales(235, particleSystem->doomRpg->render->screenWidth));
	//h = (recalcScales(174, particleSystem->doomRpg->render->screenHeight));

	if (i <= 1) {
		particleSystem->scale = particleSystem->scaleSize = particleSystem->scaleX;
	}
	else {
		particleSystem->scale = h;
		particleSystem->scaleSize = w;
		for (cnt = 1; cnt < i; cnt++) {
			particleSystem->scale = (particleSystem->scale * h) >> 8;
			particleSystem->scaleSize = (particleSystem->scaleSize * w) >> 8;
		}

		particleSystem->scale = (particleSystem->scaleX * particleSystem->scale) >> 8;
		particleSystem->scaleSize = (particleSystem->scaleX * particleSystem->scale) >> 8;
	}
}

void ParticleSystem_freeAllParticles(ParticleSystem_t* particleSystem)
{
	ParticleNode_t* particleNode;
	ParticleNode_t* nextNode;

	particleNode = (particleSystem->nodeListA).next;
	while (&particleSystem->nodeListA != particleNode) {
		nextNode = particleNode->next;
		ParticleSystem_unlinkParticle(particleSystem, particleNode);
		particleNode = nextNode;
	}
	particleSystem->particleCount = 0;
}

void ParticleSystem_render(ParticleSystem_t* particleSystem, int z)
{
	DoomRPG_t* doomRpg;
	DoomCanvas_t* doomCanvas;
	int cnt, x, y, color, r, g, b;
	ParticleNode_t* particleNode, *next;

	if (particleSystem->particleCount) {
		doomRpg = particleSystem->doomRpg;
		doomCanvas = particleSystem->doomRpg->doomCanvas;

		int i2 = doomCanvas->time > particleSystem->startTime ? (((int)(doomCanvas->time - particleSystem->startTime)) << 16) / 2560 : 0;
		color = particleSystem->color;

		if (doomCanvas->doomRpg->player->berserkerTics) { // New

			color = Render_RGB888_To_RGB565(doomRpg->render, particleSystem->color);

			r = ((color >> 11) & 0x1f) + 8;
			if (r > 31) {
				r = 31;
			}

			color = ((color & 0xfffff7df) >> 1) & 0x7ff | (r << 11);

			r = (color >> 11) & 0x1f;
			g = (color >> 5) & 0x3f;
			b = (color & 0x1f);

			color = (((r << 3) | (r >> 2)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 3) | (b >> 2));
		}

		DoomRPG_setColor(doomRpg, color);

		cnt = particleSystem->particleCount;
		y = particleSystem->dispY;
		x = particleSystem->dispX;
		DoomRPG_setClipTrue(particleSystem->doomRpg, x, y, particleSystem->dispWidth - x, particleSystem->dispHeight - y);

		particleNode = particleSystem->nodeListA.next;

		while (particleNode != &particleSystem->nodeListA) {
			next = particleNode->next;

			int i4 = doomCanvas->SCR_CX + ((particleNode->particleStartX * particleSystem->scaleX) >> 16);
			int i5 = doomCanvas->SCR_CY + ((particleNode->particleStartY * particleSystem->scaleY) >> 16);

			particleNode->particleStartX  += (particleNode->particleVelX * i2) >> 8;
			particleNode->particleGravity += (particleSystem->gravity * ((i2 * i2) >> 8)) >> 8;
			particleNode->particleStartY  += particleNode->particleGravity + ((particleNode->particleVelY * i2) >> 8);

			if (z) {
				int i6 = doomCanvas->SCR_CX + ((particleNode->particleStartX * particleSystem->scaleX) >> 16);
				int i7 = doomCanvas->SCR_CY + ((particleNode->particleStartY * particleSystem->scaleY) >> 16);

				boolean z2 = false;

				int pSize = particleNode->particleSize >> 1;
				if (particleNode->particleSize < 0) {
					pSize = particleSystem->gibSizeWH >> 1;
				}

				if (i6 - pSize > particleSystem->dispWidth || 
					i6 + pSize < particleSystem->dispX || 
					i7 - pSize > particleSystem->dispHeight ||
					i7 + pSize < particleSystem->dispY) {
					z2 = true;
				}
				else if (particleSystem->f547c > 0) {
					int i8 = i6 - (doomCanvas->SCR_CX + particleSystem->centreX);
					int i9 = i7 - (doomCanvas->SCR_CY + particleSystem->centreY);
					int i10 = i4 - (doomCanvas->SCR_CX + particleSystem->centreX);
					int i11 = i5 - (doomCanvas->SCR_CY + particleSystem->centreY);
					if (((i8 >= 0 || i10 >= 0) && (i8 < 0 || i10 < 0)) || ((i9 >= 0 || i11 >= 0) && (i9 < 0 || i11 < 0))) {
						z2 = true;
					}
					else if (ABS(i8) <= particleSystem->f547c && ABS(i9) <= particleSystem->f547c) {
						z2 = true;
					}
				}

				if (z2) {
					ParticleSystem_unlinkParticle(particleSystem, particleNode);
				}
				else if (particleNode->particleSize > 0) {
					DoomRPG_fillCircle(particleSystem->doomRpg, i6, i7, particleNode->particleSize >> 1);
				}
				else {
					/*int i12 = 16 * ((-nVar.particleSize) - 1);
					int i13 = ((int)(doomCanvas->time / 175)) & 3;
					if (nVar.particleVelX > 0) {
						i13 = 3 - i13;
					}
					switch (i13) {
					case 1:
						i = 5;
						break;
					case 2:
						i = 3;
						break;
					case 3:
						i = 6;
						break;
					default:
						i = 0;
						break;
					}*/

					//DoomCanvas.drawImage(imgGibs, 0, i12, 16, 16, i, i6, i7, 3);

					int height = particleSystem->gibSizeWH;
					int i13 = ((int)(doomCanvas->time / 175)) & 3;

					if (particleNode->particleVelX > 0) {
						i13 = 3 - i13;
					}

					int xSrc = i13 * height;
					int ySrc = height * -(particleNode->particleSize + 1);


					int width = height;
					if (particleSystem->dispWidth < i6 + pSize) {
						width = (particleSystem->dispWidth - i6) + pSize;
					}
					else {
						if (i6 - pSize < particleSystem->dispX) {
							width = (i6 - particleSystem->dispX) + pSize;
							i6 = particleSystem->dispX + pSize;
						}
					}

					if (particleSystem->dispHeight < i7 + pSize) {
						height = (particleSystem->dispHeight - i7) + pSize;
					}
					else {
						if (i7 - pSize < particleSystem->dispY) {
							height = (i7 - particleSystem->dispY) + pSize;
							i7 = particleSystem->dispY + pSize;
						}
					}

					int iVar5 = width;
					if (width > 0) {
						iVar5 = height;
					}

					int flags = 0;
					if (sdlVideoModes[sdlVideo.resolutionIndex].width >= 640) {
						flags |= 64; // set flag SCALE
					}

					if (iVar5 > 0) {
						if (doomCanvas->doomRpg->player->berserkerTics) { // New
							DoomCanvas_drawImageSpecial(doomCanvas, &particleSystem->imgGibsBerserk, xSrc, ySrc, width, height, 0, i6 - pSize, i7 - pSize, flags);
						}
						else {
							DoomCanvas_drawImageSpecial(doomCanvas, &particleSystem->imgGibs, xSrc, ySrc, width, height, 0, i6 - pSize, i7 - pSize, flags);
						}
					}
				}
			}

			particleNode = next;
		}


		SDL_RenderSetClipRect(sdlVideo.renderer, NULL);

		particleSystem->startTime = doomRpg->doomCanvas->time;
		if (z) {
			particleSystem->endTime = doomRpg->doomCanvas->time;
		}
		if (doomRpg->doomCanvas->slowBlit && particleSystem->particleCount == 0 && cnt != 0) {
			DoomCanvas_invalidateRectAndUpdateView(doomRpg->doomCanvas);
		}
	}
}

void ParticleSystem_spawnParticles(ParticleSystem_t* particleSystem, int i, int i2, int z)
{
	ParticleSystem_calculateScales(particleSystem, i2);

	int i3 = ((((i << 8) * particleSystem->scaleSize) >> 8) + 128) >> 8;
	if (i3 != 0) {
		particleSystem->minVelX = (((particleSystem->minVelX << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->maxVelX = (((particleSystem->maxVelX << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->minVelY = (((particleSystem->minVelY << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->maxVelY = (((particleSystem->maxVelY << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->gravity = (((particleSystem->gravity << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->dispWidth = particleSystem->doomRpg->doomCanvas->displayRect.w;
		if (i2 == 2) {
			particleSystem->dispHeight = particleSystem->doomRpg->doomCanvas->displayRect.h - (((12288 * particleSystem->scaleY) + 32768) >> 16);
		}
		else if (i2 == 3) {
			particleSystem->dispHeight = particleSystem->doomRpg->doomCanvas->displayRect.h - (((13824 * particleSystem->scaleY) + 32768) >> 16);
		}
		else if (i2 == 4) {
			particleSystem->dispHeight = particleSystem->doomRpg->doomCanvas->displayRect.h - (((14336 * particleSystem->scaleY) + 32768) >> 16);
		}
		else {
			particleSystem->dispHeight = particleSystem->doomRpg->doomCanvas->displayRect.h - particleSystem->doomRpg->hud->statusBarHeight;
		}
		particleSystem->minStartX = (((particleSystem->minStartX << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->maxStartX = (((particleSystem->maxStartX << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->minStartY = (((particleSystem->minStartY << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->maxStartY = (((particleSystem->maxStartY << 8) * particleSystem->scale) + 32768) >> 16;
		particleSystem->minSize = (((particleSystem->minSize << 8) * particleSystem->scaleSize) + 32768) >> 16;
		particleSystem->maxSize = (((particleSystem->maxSize << 8) * particleSystem->scaleSize) + 32768) >> 16;
		if (particleSystem->maxSize == 0) {
			particleSystem->maxSize = 1;
		}
		ParticleSystem_calculateParticles(particleSystem, i3, z);
	}
}

void ParticleSystem_calculateParticles(ParticleSystem_t* particleSystem, int i, int z)
{
	ParticleNode_t* particleNode;
	int maxStartX, maxStartY, minStartX, minStartY;
	int maxVelX, maxVelY, minVelX, minVelY;
	int sx, sy, vx, vy, particleSize, cnt1, cnt2, randNext;

	if (z) {
		i += 5;
	}

	sx = 0;
	sy = 0;
	if (particleSystem->f543a != 0) {
		maxStartX = particleSystem->maxStartX;
		maxStartY = particleSystem->maxStartY;
		minStartX = particleSystem->minStartX;
		minStartY = particleSystem->minStartY;

		if (maxStartX != minStartX) {
			sx = (minStartX + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxStartX - minStartX + 1)) << 8;
		}
		else {
			sx = maxStartX;
		}

		if (maxStartY != minStartY) {
			sy = (minStartY + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxStartY - minStartY + 1)) << 8;
		}
		else {
			sy = maxStartY;
		}
	}

	randNext = (DoomRPG_randNextInt(&particleSystem->doomRpg->random) % 6);
	cnt2 = 0;
	for (cnt1 = 0; cnt1 < i; cnt1++)
	{
		if (particleSystem->nodeListB.next == &particleSystem->nodeListB) {
			return;
		}

		particleNode = particleSystem->nodeListB.next;

		if (particleSystem->f543a) {
			particleNode->particleStartX = sx;
			particleNode->particleStartY = sy;
		}
		else {
			maxStartX = particleSystem->maxStartX;
			maxStartY = particleSystem->maxStartY;
			minStartX = particleSystem->minStartX;
			minStartY = particleSystem->minStartY;

			particleNode->particleStartX = (minStartX + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxStartX - minStartX + 1)) << 8;
			particleNode->particleStartY = (minStartY + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxStartY - minStartY + 1)) << 8;
		}

		if (particleSystem->f547c > 0) {
			vx = (particleSystem->centreX << 8) - particleNode->particleStartX;
			vy = (particleSystem->centreY << 8) - particleNode->particleStartY;
			DoomRPG_randNextInt(&particleSystem->doomRpg->random);
			particleNode->particleVelX = (vx << 8) / 10240;
			particleNode->particleVelY = (vy << 8) / 10240;
		}
		else {
			maxVelX = particleSystem->maxVelX;
			maxVelY = particleSystem->maxVelY;
			minVelX = particleSystem->minVelX;
			minVelY = particleSystem->minVelY;

			if (maxVelX != minVelX) {
				particleNode->particleVelX = ((minVelX + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxVelX - minVelX + 1)) << 16) / 25600;
			}
			else {
				particleNode->particleVelX = maxVelX;
			}

			if (maxVelY != minVelY) {
				particleNode->particleVelY = ((minVelY + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (maxVelY - minVelY + 1)) << 16) / 25600;
			}
			else {
				particleNode->particleVelY = maxVelY;
			}
		}

		if (z && cnt2++ < 5) {
			particleSize = -(1 + (randNext + cnt2) % 6);
		}
		else if(particleSystem->maxSize != particleSystem->minSize) {
			particleSize = particleSystem->minSize + (DoomRPG_randNextInt(&particleSystem->doomRpg->random) & 0xFFFF) % (particleSystem->maxSize - particleSystem->minSize + 1);
		}
		else {
			particleSize = particleSystem->maxSize;
		}

		particleNode->particleSize = particleSize;
		if (particleNode->particleSize) {
			particleNode->particleGravity = 0;
			ParticleSystem_linkParticle(particleSystem, particleNode);
		}
	}
}



void ParticleSystem_startup(ParticleSystem_t* particleSystem)
{
	int i, gibSizeWH;
	ParticleNode_t* particleNode;
	DoomRPG_t* doomRpg;

	int w, h;

	particleSystem->nodeListA.prev = &particleSystem->nodeListA;
	particleSystem->nodeListA.next = &particleSystem->nodeListA;
	particleSystem->nodeListB.prev = &particleSystem->nodeListB;
	particleSystem->nodeListB.next = &particleSystem->nodeListB;

	for (i = 0; i < 64; i++) {
		particleNode = &particleSystem->nodeListC[i];
		particleNode->prev = particleNode;
		particleNode->next = particleNode;
		ParticleSystem_unlinkParticle(particleSystem, particleNode);
	}
	particleSystem->particleCount = 0;

	doomRpg = particleSystem->doomRpg;

	w = 128;
	h = 88;

	if (sdlVideoModes[sdlVideo.resolutionIndex].width >= 320) {
		w = recalcScales(128, doomRpg->render->screenWidth);
		h = recalcScales(88, doomRpg->render->screenHeight);
	}

	particleSystem->scaleX = ((doomRpg->render->screenWidth << FRACBITS) / (w << 8));
	particleSystem->scaleY = ((doomRpg->render->screenHeight << FRACBITS) / (h << 8));

	if (doomRpg->doomCanvas->displayRect.w < 160) {
		DoomRPG_createImage(doomRpg, "gibs_16.bmp", true, &particleSystem->imgGibs);
		DoomRPG_createImageBerserkColor(doomRpg, "gibs_16.bmp", true, &particleSystem->imgGibsBerserk); // New
		gibSizeWH = 16;
	}
	else {
		DoomRPG_createImage(doomRpg, "gibs_24.bmp", true, &particleSystem->imgGibs);
		DoomRPG_createImageBerserkColor(doomRpg, "gibs_24.bmp", true, &particleSystem->imgGibsBerserk); // New
		gibSizeWH = 24;
	}

	particleSystem->gibSizeWH = gibSizeWH;

	//printf("particleSystem->scaleX %d\n", particleSystem->scaleX);
	//printf("particleSystem->scaleY %d\n", particleSystem->scaleY);
	//printf("particleSystem->gibSizeWH %d\n", particleSystem->gibSizeWH);
}

