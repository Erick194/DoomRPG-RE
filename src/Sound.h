#ifndef SOUND_H__
#define SOUND_H__

#include <SDL_mixer.h>

struct DoomRPG_s;
struct Image_s;

#define MAX_SOUNDCHANNELS	10
#define MAX_AUDIOFILES		95

#define SND_FLG_LOOP		1	// Hace que el sonido se repita indefinidamente / Makes the sound repeat indefinitely
#define SND_FLG_STOPSOUNDS	2	// Detiene todos los sonidos / Stops all sounds
// New Flags
#define SND_FLG_NOFORCESTOP 4	// Evita que el canal sea detenido forzosamente / Prevents the channel from being forcibly stopped
#define SND_FLG_ISMUSIC		8	// Define el canal como una musica / Define the channel as a music

typedef struct AudioFile_s
{
	void* ptr;
} AudioFile_t;

#include <fluidsynth.h>
typedef struct SoundChannel_s
{
	//int heap;
	Mix_Chunk* mediaAudioSound;
	//Mix_Music* mediaAudioMusic;
	fluid_player_t* mediaAudioMusic;
	int size;
	//short resourceID;
	//byte field_0xe;
	byte flags;
} SoundChannel_t;

typedef struct Sound_s
{
	boolean soundEnabled;
	int priority;
	int channel;
	int nextplay;
	struct SoundChannel_s soundChannel[MAX_SOUNDCHANNELS+1];
	int volume;
	struct DoomRPG_s* doomRpg;
	AudioFile_t* audioFiles; // New
} Sound_t;

Sound_t* Sound_init(Sound_t* sound, DoomRPG_t* doomRpg);
void Sound_free(Sound_t* sound, boolean freePtr);
void Sound_stopSounds(Sound_t* sound);
void Sound_freeSound(Sound_t* sound, int chan);
int Sound_getState(Sound_t* sound, int resourceID);
int Sound_getFreeChanel(Sound_t* sound);
void Sound_loadSound(Sound_t* sound, int chan, short resourceID);
void Sound_readySound(Sound_t* sound, int chan);
void Sound_playSound(Sound_t* sound, int resourceID, byte flags, int priority);
void Sound_freeSounds(Sound_t* sound);
int Sound_getFromResourceID(resourceID);
void Sound_updateVolume(Sound_t* sound);
int Sound_minusVolume(Sound_t* sound, int volume);
int Sound_addVolume(Sound_t* sound, int volume);

#endif
