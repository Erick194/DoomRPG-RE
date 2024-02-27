
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Sound.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "Z_Zip.h"

#define INIT_ALLSOUNDS	1

static soundTable[MAX_AUDIOFILES] = {
	5039, 5040, 5042, 5043, 5044, 5045, 5046, 5047, 5048, 5049, 5050,
	5051, 5052, 5053, 5054, 5055, 5057, 5058, 5059, 5060, 5061, 5062,
	5063, 5064, 5065, 5066, 5067, 5068, 5069, 5070, 5071, 5072, 5073,
	5074, 5076, 5077, 5078, 5079, 5080, 5081, 5082, 5083, 5084, 5085,
	5086, 5087, 5088, 5089, 5090, 5091, 5092, 5093, 5094, 5095, 5096,
	5097, 5098, 5099, 5100, 5101, 5102, 5103, 5104, 5105, 5106, 5107,
	5108, 5109, 5110, 5111, 5112, 5113, 5114, 5115, 5116, 5117, 5118,
	5119, 5120, 5121, 5122, 5123, 5124, 5125, 5126, 5127, 5128, 5129,
	5130, 5131, 5133, 5134, 5136, 5137, 5138
};

Sound_t* Sound_init(Sound_t* sound, DoomRPG_t* doomRpg)
{
	int i;
	SoundChannel_t* chan;

	printf("Sound_init\n");

	if (sound == NULL)
	{
		sound = SDL_malloc(sizeof(Sound_t));
		if (sound == NULL) {
			return NULL;
		}
	}
	SDL_memset(sound, 0, sizeof(Sound_t));

	sound->soundEnabled = 0;
	sound->priority = 3;
	sound->channel = 0;
	sound->volume = 100;
	i = 0;
	do {
		chan = &sound->soundChannel[i];
		//chan->heap = 0;
		chan->mediaAudioSound = NULL;
		chan->mediaAudioMusic = NULL;
		chan->size = 0;
		//chan->resourceID = 0;
		//chan->field_0xe = 0;
	} while (++i < (MAX_SOUNDCHANNELS+1));
	sound->doomRpg = doomRpg;


	Mix_AllocateChannels(MAX_SOUNDCHANNELS);
	//Mix_VolumeMusic((sound->volume * MIX_MAX_VOLUME) / 100);
	Mix_Volume(-1, (sound->volume * MIX_MAX_VOLUME) / 100);

	//printf("Num Channes is: %d\n", Mix_AllocateChannels(-1));
	//printf("Music volume is: %d\n", Mix_VolumeMusic(-1));
	//printf("Sound volume is: %d\n", Mix_Volume(-1, -1));

	// Allowed range of synth.gain is 0.0 to 10.0
	fluid_settings_setnum(fluidSynth.settings, "synth.gain", 1.0 * ((sound->volume * MIX_MAX_VOLUME) / 100) / 128.0);

#if INIT_ALLSOUNDS
	sound->audioFiles = SDL_malloc(sizeof(AudioFile_t) * MAX_AUDIOFILES);

	for (i = 0; i < MAX_AUDIOFILES; i++)
	{
		SDL_RWops* rw;
		char fileName[128];
		byte* fdata;
		int fSize;

		if ((i == 0) || (i == 1) || (i == 3)) { // Midi Files
			snprintf(fileName, sizeof(fileName), "%03d.mid", soundTable[i]);

			fdata = readZipFileEntry(fileName, &zipFile, &fSize);
			//rw = SDL_RWFromMem(fdata, fSize);
			//if (!rw) {
			//	DoomRPG_Error("Error with SDL_RWFromMem: %s\n", SDL_GetError());
			//}
			//sound->audioFiles[i].ptr = Mix_LoadMUS_RW(rw, 0);

			sound->audioFiles[i].ptr = (fluid_player_t*)new_fluid_player(fluidSynth.synth);
			fluid_player_add_mem((fluid_player_t*)sound->audioFiles[i].ptr, fdata, fSize);

			SDL_free(fdata);
		}
		else {
			snprintf(fileName, sizeof(fileName), "%03d.wav", soundTable[i]);

			fdata = readZipFileEntry(fileName, &zipFile, &fSize);
			rw = SDL_RWFromMem(fdata, fSize);
			if (!rw) {
				DoomRPG_Error("Error with SDL_RWFromMem: %s\n", SDL_GetError());
			}
			sound->audioFiles[i].ptr = Mix_LoadWAV_RW(rw, 0);
			SDL_free(fdata);
		}
	}
#endif

	return sound;
}

void Sound_free(Sound_t* sound, boolean freePtr)
{
	Sound_freeSounds(sound);

#if INIT_ALLSOUNDS
	for (int i = 0; i < MAX_AUDIOFILES; i++) {
		if ((i == 0) || (i == 1) || (i == 3)) { // Midi Files
			//Mix_FreeMusic((Mix_Music*)sound->audioFiles[i].ptr);
			delete_fluid_player((fluid_player_t*)sound->audioFiles[i].ptr);
		}
		else {
			Mix_FreeChunk((Mix_Chunk*)sound->audioFiles[i].ptr);
		}
	}
	SDL_free(sound->audioFiles);
#endif

	if (freePtr) {
		SDL_free(sound);
	}
}

void Sound_stopSounds(Sound_t* sound)
{
	int chan = 0;
	do {
		if (sound->soundChannel[chan].flags & SND_FLG_ISMUSIC) {
			/*if (Mix_PlayingMusic()) {
				Mix_HaltMusic();
				sound->soundChannel[chan].flags = 0;
			}*/

			if (fluid_player_get_status(sound->soundChannel[chan].mediaAudioMusic) == FLUID_PLAYER_PLAYING) {
				fluid_player_stop(sound->soundChannel[chan].mediaAudioMusic);
				fluid_player_seek(sound->soundChannel[chan].mediaAudioMusic, 0);
				sound->soundChannel[chan].flags = 0;
			}
		}
		else {
			if (Mix_Playing(chan) && !(sound->soundChannel[chan].flags & SND_FLG_NOFORCESTOP)) {
				Mix_HaltChannel(chan);
				sound->soundChannel[chan].flags = 0;
			}
		}
	} while (++chan < (MAX_SOUNDCHANNELS + 1));
	sound->priority = 0;
}

void Sound_freeSound(Sound_t* sound, int chan)
{
	SoundChannel_t* sChannel;
	sChannel = &sound->soundChannel[chan];

	if (sChannel->mediaAudioSound) {
		if (Mix_Playing(chan)) {
			Mix_HaltChannel(chan);
		}
	}

	if (sChannel->mediaAudioSound) {
#if !INIT_ALLSOUNDS
		Mix_FreeChunk(sChannel->mediaAudioSound);
#endif
	}

	/*if (sChannel->mediaAudioMusic) {
		if (Mix_PlayingMusic()) {
			Mix_HaltMusic();
		}
	}*/

	if (sChannel->mediaAudioMusic) {
		if (fluid_player_get_status(sChannel->mediaAudioMusic) == FLUID_PLAYER_PLAYING) {
			fluid_player_stop(sChannel->mediaAudioMusic);
			fluid_player_seek(sChannel->mediaAudioMusic, 0);
		}
	}

	if (sChannel->mediaAudioMusic) {
#if !INIT_ALLSOUNDS
		Mix_FreeMusic(sChannel->mediaAudioMusic);
#endif
	}

	sChannel->flags = 0;
	sChannel->mediaAudioSound = NULL;
	sChannel->mediaAudioMusic = NULL;
}

int Sound_getState(Sound_t* sound, int resourceID)
{
	int chan = 0;
	int play = 0;
	do {
		if (Mix_Playing(chan)) {
			++play;
		}
	} while (++chan < MAX_SOUNDCHANNELS);

	//printf("Sound_getState %d\n", sound->nextplay + play);
	return sound->nextplay + play;
}

int Sound_getFreeChanel(Sound_t* sound) {

	int chan = 0;
	int play = -1;
	do {
		if (Mix_Playing(chan)) {
			continue;
		}

		if (play == -1) {
			Sound_freeSound(sound, chan);
			play = chan;
		}
		else {
			Mix_HaltChannel(chan);
		}
	} while (++chan < MAX_SOUNDCHANNELS);

	return play;
}

void Sound_loadSound(Sound_t* sound, int chan, short resourceID)
{
#if !INIT_ALLSOUNDS
	SDL_RWops* rw;
#endif
	SoundChannel_t* sChannel;
	int flags;

#if INIT_ALLSOUNDS
	int id;
#else
	char fileName[128];
	byte* fdata;
	int fSize;
#endif

#if INIT_ALLSOUNDS
	id = Sound_getFromResourceID(resourceID);
	if (id == -1) {
		return;
	}
#endif

	sChannel = &sound->soundChannel[chan];
	flags = sChannel->flags; // save Flags;

	if (sChannel->flags & SND_FLG_ISMUSIC) {
		if (sChannel->mediaAudioMusic) {
			Sound_freeSound(sound, chan);
			sChannel->mediaAudioMusic = NULL;
		}

#if INIT_ALLSOUNDS
		//sChannel->mediaAudioMusic = (Mix_Music*)sound->audioFiles[id].ptr;
		sChannel->mediaAudioMusic = (fluid_player_t*)sound->audioFiles[id].ptr;
#else
		snprintf(fileName, sizeof(fileName), "%03d.mid", resourceID);
		fdata = readZipFileEntry(fileName, &zipFile, &fSize);

		rw = SDL_RWFromMem(fdata, fSize);
		if (!rw) {
			DoomRPG_Error("Error with SDL_RWFromMem: %s\n", SDL_GetError());
		}

		sChannel->mediaAudioMusic = Mix_LoadMUS_RW(rw, 0);
		SDL_free(fdata);
#endif
	}
	else {
		if (sChannel->mediaAudioSound) {
			Sound_freeSound(sound, chan);
			sChannel->mediaAudioSound = NULL;
		}

#if INIT_ALLSOUNDS
		sChannel->mediaAudioSound = (Mix_Chunk*) sound->audioFiles[id].ptr;
#else
		snprintf(fileName, sizeof(fileName), "%03d.wav", resourceID);
		fdata = readZipFileEntry(fileName, &zipFile, &fSize);

		rw = SDL_RWFromMem(fdata, fSize);
		if (!rw) {
			DoomRPG_Error("Error with SDL_RWFromMem: %s\n", SDL_GetError());
		}
		sChannel->mediaAudioSound = Mix_LoadWAV_RW(rw, 0);
		SDL_free(fdata);
#endif
	}

	sChannel->flags = flags; // Restore flags
}

void Sound_readySound(Sound_t* sound, int chan)
{
	SoundChannel_t* sChannel;
	sChannel = &sound->soundChannel[chan];

	if (sChannel->flags & SND_FLG_ISMUSIC) {
		//Mix_VolumeMusic((sound->volume * MIX_MAX_VOLUME) / 100);
		// Allowed range of synth.gain is 0.0 to 10.0
		fluid_settings_setnum(fluidSynth.settings, "synth.gain", 1.0 * ((sound->volume * MIX_MAX_VOLUME) / 100) / 128.0);
	}
	else {
		Mix_VolumeChunk(sound->soundChannel[chan].mediaAudioSound, (sound->volume * MIX_MAX_VOLUME) / 100);
	}
}

void Sound_playSound(Sound_t* sound, int resourceID, byte flags, int priority)
{
	boolean sndPriority = sound->doomRpg->doomCanvas->sndPriority;
	boolean isMusic = (flags & SND_FLG_ISMUSIC) ? true : false;

	if (sound->soundEnabled != 0) {

		resourceID = resourceID; //Sound_getFromResourceID(resourceID);
		if (resourceID >= 0) {

			/*if (isMusic) {
				printf("Sound_playSound %03d.mid\n", resourceID);
			}
			else {
				printf("Sound_playSound %03d.wav\n", resourceID);
			}*/

			//printf("priority %d\n", priority);
			//printf("sound->priority %d\n", sound->priority);

			if (((priority < sound->priority) && Sound_getState(sound, resourceID) && !isMusic) && sndPriority) {
				printf("Sound: Dynamic playback of %d prevented by priority (%d < %d)\n", resourceID, priority, sound->priority);
			}
			else {

				if (isMusic) {
					sound->channel = MAX_SOUNDCHANNELS; // Music
				}
				else {
					sound->channel = Sound_getFreeChanel(sound->doomRpg->sound);
				}

				if (sound->channel >= 0) {
					if (flags & SND_FLG_STOPSOUNDS) {
						Sound_stopSounds(sound);
					}

					sound->soundChannel[sound->channel].flags = flags & SND_FLG_LOOP;
					sound->soundChannel[sound->channel].flags |= flags & SND_FLG_ISMUSIC;
					sound->soundChannel[sound->channel].flags |= flags & SND_FLG_NOFORCESTOP;

					Sound_loadSound(sound, sound->channel, resourceID);
					Sound_readySound(sound, sound->channel);
					
					sound->priority = priority;
					if (flags & SND_FLG_ISMUSIC) {
						//Mix_PlayMusic(sound->soundChannel[sound->channel].mediaAudioMusic, (flags & SND_FLG_LOOP) ? -1 : 0);
						fluid_player_set_loop(sound->soundChannel[sound->channel].mediaAudioMusic, (flags & SND_FLG_LOOP) ? -1 : 0);
						fluid_player_play(sound->soundChannel[sound->channel].mediaAudioMusic);

					}
					else {
						Mix_PlayChannel(sound->channel, sound->soundChannel[sound->channel].mediaAudioSound, (flags & SND_FLG_LOOP) ? -1 : 0);
					}

					sound->nextplay++;
				}
			}
		}
	}
}

void Sound_freeSounds(Sound_t* sound)
{
	int chan = 0;
	do {
		Sound_freeSound(sound, chan);
	} while (++chan < (MAX_SOUNDCHANNELS + 1));
}

int Sound_getFromResourceID(resourceID)
{
	for (int i = 0; i < MAX_AUDIOFILES; i++) {
		if (soundTable[i] == resourceID) {
			return i;
		}
	}

	return -1;
}

void Sound_updateVolume(Sound_t* sound)
{
	int chan = 0;
	do {
		if (sound->soundChannel[chan].flags & SND_FLG_ISMUSIC) {
			/*if (Mix_PlayingMusic()) {
				Mix_VolumeMusic((sound->volume * MIX_MAX_VOLUME) / 100);
			}*/

			if (fluid_player_get_status(sound->soundChannel[chan].mediaAudioMusic) == FLUID_PLAYER_PLAYING) {
				// Allowed range of synth.gain is 0.0 to 10.0
				fluid_settings_setnum(fluidSynth.settings, "synth.gain", 1.0 * ((sound->volume * MIX_MAX_VOLUME) / 100) / 128.0);
			}
		}
		else {
			if (Mix_Playing(chan)) {
				Mix_VolumeChunk(sound->soundChannel[chan].mediaAudioSound, (sound->volume * MIX_MAX_VOLUME) / 100);
			}
		}
	} while (++chan < (MAX_SOUNDCHANNELS + 1));

	int menu = sound->doomRpg->menuSystem->menu;
	//if (menu == MENU_MAIN_OPTIONS || menu == MENU_INGAME_OPTIONS) { // Old
	if (menu == MENU_SOUND || menu == MENU_INGAME_SOUND) {
		Menu_textVolume(sound->doomRpg->menu, sound->volume);
	}
}

int Sound_minusVolume(Sound_t* sound, int volume)
{
	sound->volume -= volume;
	if (sound->volume < 0) {
		sound->volume = 0;
	}
	Sound_updateVolume(sound);
	return sound->volume;
}

int Sound_addVolume(Sound_t* sound, int volume)
{
	sound->volume += volume;
	if (sound->volume > 100) {
		sound->volume = 100;
	}
	Sound_updateVolume(sound);
	return sound->volume;
}