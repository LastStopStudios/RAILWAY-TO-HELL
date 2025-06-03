#pragma once

#include "Module.h"
#include "SDL2/SDL_mixer.h"
#include <list>

#define DEFAULT_MUSIC_FADE_TIME 2.0f

#define AMBIENT_CHANNEL 7 // ambient fx channel
#define BACKGROUND_MUSIC_CHANNEL 0

struct _Mix_Music;

class Audio : public Module
{
public:

	Audio();

	// Destructor
	virtual ~Audio();

	// Called before render is available
	bool Awake();

	// Called before quitting
	bool CleanUp();

	// Play a music file
	bool PlayMusic(const char* path, float fadeTime = DEFAULT_MUSIC_FADE_TIME);

	//Ajust sound volum Rango de 0 a MIX_MAX_VOLUME (128)
	void SetFxVolume(int id, int volume);

	// Load a WAV in memory
	int LoadFx(const char* path);

	bool StopBackgroundMusic();

	// Play a previously loaded WAV
	bool PlayFx(int fx, int repeat = 0);

	bool PlayAmbientFx(int fx);

	// Stop all currently playing sound effects
	void StopAllFx();

	void SetMusicVolume(int volume) {
		if (!active || music == NULL) return;

		if (volume < 0) {
			volume = 0;
		}
		else if (volume > MIX_MAX_VOLUME) {
			volume = MIX_MAX_VOLUME;
		}
		musicVolume = volume;
		Mix_VolumeMusic(volume);
	}

	int GetMusicVolume() const { return musicVolume; }
	int GetFxVolume() const { return fxVolume; }

	void SetGlobalFxVolume(int volume);

private:

	int musicVolume = MIX_MAX_VOLUME; 
	int fxVolume = MIX_MAX_VOLUME;
	int globalFxVolume = MIX_MAX_VOLUME;

	_Mix_Music* music;
	std::list<Mix_Chunk*> fx;
};
