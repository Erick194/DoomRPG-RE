//Using SDL and standard IO
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <zlib.h>

#include "Z_Zone.h"
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Player.h"
#include "Hud.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "Z_Zip.h"

extern DoomRPG_t* doomRpg;
int main(int argc, char* args[])
{
	SDL_Event ev;
	int		UpTime = 0;
	int		mouseTime = 0;
	int		key, oldKey, mouse_Button;

	Z_Init();
	SDL_InitVideo();
	SDL_InitAudio();

	openZipFile("DoomRPG.zip", &zipFile);

	/*int size;
	byte* data;
	data = readZipFileEntry("c.bmp", &zipFile, &size);

	SDL_RWops* rw;
	rw = SDL_RWFromFile("c.bmp", "w");
	SDL_RWwrite(rw, data, sizeof(byte), size);
	SDL_RWclose(rw);

	SDL_free(data);

	closeZipFile(&zipFile);*/

	if (DoomRPG_Init() == 0) {
		DoomRPG_Error("Failed to initialize Doom Rpg\n");
	}

	//Hud_addMessage(doomRpg->hud, "Bienvenido a Doom RPG por GEC...");

	const Uint8* state = SDL_GetKeyboardState(NULL);

	key = 0;
	oldKey = -1;
	
	
	while (doomRpg->closeApplet != true)
	{

		int currentTimeMillis = DoomRPG_GetUpTimeMS();

		mouse_Button = MOUSE_BUTTON_INVALID;

		while (SDL_PollEvent(&ev))
		{
			// check event type
			switch (ev.type) {

				// Mouse Event
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				{
					Uint32 buttons = SDL_GetMouseState(NULL, NULL);

					if ((buttons & SDL_BUTTON_LMASK) != 0) {
						mouse_Button = MOUSE_BUTTON_LEFT;
					}
					else if ((buttons & SDL_BUTTON_MMASK) != 0) {
						mouse_Button = MOUSE_BUTTON_MIDDLE;
					}
					else if ((buttons & SDL_BUTTON_RMASK) != 0) {
						mouse_Button = MOUSE_BUTTON_RIGHT;
					}
					else if ((buttons & SDL_BUTTON_X1MASK) != 0) {
						mouse_Button = MOUSE_BUTTON_X1;
					}
					else if ((buttons & SDL_BUTTON_X2MASK) != 0) {
						mouse_Button = MOUSE_BUTTON_X2;
					}
					break;
				}

				case SDL_MOUSEWHEEL:
				{
					if (currentTimeMillis > mouseTime) {
						mouseTime = currentTimeMillis + 128;
						if (ev.wheel.y > 0) {
							mouse_Button = MOUSE_BUTTON_WHELL_UP;
						}
						else if (ev.wheel.y < 0) {
							mouse_Button = MOUSE_BUTTON_WHELL_DOWN;
						}
					}
					break;
				}

				case SDL_MOUSEMOTION:
				{
					if (!doomRpg->menuSystem->setBind) {
						if (currentTimeMillis > mouseTime) {
							mouseTime = currentTimeMillis + 128;
							int x = 0, y = 0;
							SDL_GetRelativeMouseState(&x, &y);

							int sensivity = (doomRpg->doomCanvas->mouseSensitivity * 1000) / 100;

							if (x <= -sensivity) {
								mouse_Button = MOUSE_BUTTON_MOTION_LEFT;
							}
							else if (x >= sensivity) {
								mouse_Button = MOUSE_BUTTON_MOTION_RIGHT;
							}

							if (doomRpg->doomCanvas->mouseYMove) {
								if (y <= -sensivity) {
									mouse_Button = MOUSE_BUTTON_MOTION_UP;
								}
								else if (y >= sensivity) {
									mouse_Button = MOUSE_BUTTON_MOTION_DOWN;
								}
							}
						}
					}
					break;
				}
				
				case SDL_WINDOWEVENT:
				{
					if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
						//printf("MESSAGE:Resizing window...\n");
						//SDL_Rect rect = { 0,0,640,480};
						//SDL_RenderSetViewport(sdlVideo.renderer, &rect);
						//resizeWindow();
					}

					if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
						SDL_Log("Window %d closed", ev.window.windowID);
						closeZipFile(&zipFile);
						DoomRPG_FreeAppData(doomRpg);
						SDL_CloseAudio();
						SDL_Close();
						exit(0);
						break;
					}

					if (ev.window.event != SDL_WINDOWEVENT_CLOSE)
					{
						int w, h;
						SDL_GetWindowSize(sdlVideo.window, &w, &h);
						SDL_WarpMouseInWindow(sdlVideo.window, w / 2, h / 2);
						SDL_GetRelativeMouseState(NULL, NULL);
					}
					break;
				}

				case SDL_QUIT:
				{
					// shut down
					exit(0);
					break;
				}
			}

			key = DoomRPG_getEventKey(mouse_Button, state);
			if (key != oldKey) {
				//printf("oldKey %d\n", oldKey);
				//printf("key %d\n", key);

				oldKey = key;
				if (!doomRpg->menuSystem->setBind) {
					DoomCanvas_keyPressed(doomRpg->doomCanvas, key);
				}
				else {
					goto setBind;
				}
			}
			else if (key == 0) {
			setBind:
				if (doomRpg->menuSystem->setBind) {
					DoomRPG_setBind(doomRpg, mouse_Button, state);
				}
			}
		}
		
		if (currentTimeMillis > UpTime) {
			UpTime = currentTimeMillis + 15;
			DoomRPG_loopGame(doomRpg);
		}
	}

	closeZipFile(&zipFile);
	DoomRPG_FreeAppData(doomRpg);
	SDL_CloseAudio();
	SDL_Close();

	return 0;
}