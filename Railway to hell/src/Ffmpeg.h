#pragma once
#define __CUTSCENEPLAYER_H__ 

#include "Point.h"
#include "Module.h"
#include <queue>
#include <string>
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_thread.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libpostproc/postprocess.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

struct SDL_Texture;

class Ffmpeg : public Module
{
public:

	Ffmpeg(bool enabled = true);

	virtual ~Ffmpeg();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	bool OpenCodecContext(int* index);
	bool OpenVideoCodecContext(int index);
	bool ConvertPixels(int videoIndex, int audioIndex);
	bool ConvertPixels(const char* videoPath);
	bool LoadVideo(const char* videoPath);
	bool AllocImage(AVFrame* dstFrame);
	void RenderCutscene();
	bool HandleEvents();

	// WAV audio handling methods
	bool LoadAndPlayWavAudio(const char* wavPath);
	void StopWavAudio();
	std::string GetWavPathFromVideo(const char* videoPath);

	// Skip system methods
	void RenderSkipBar(float progress);
	void UpdateSkipSystem();

public:
	int streamIndex = -1;
	AVFormatContext* formatContext;
	AVCodecContext* videoCodecContext;

	// Audio management through Engine's Audio module
	bool isAudioPlaying = false;

	SDL_Texture* renderTexture;
	SDL_Texture* texture1;
	SDL_Texture* texture2;
	SDL_Rect renderRect;
	bool running = false;  // Added initialization

	SDL_Rect rect1 = { 300,100,250,500 };
	SDL_Rect rect2 = { 600,100,250,500 };
	iPoint position1 = { 300,500 };
	iPoint position2 = { 600,500 };
	bool isHover1 = false;
	bool isHover2 = false;

	// Skip system variables
	bool isSkipping = false;
	bool skipCompleted = false;  // Nueva variable para controlar el skip
	Uint32 skipStartTime = 0;
	const Uint32 SKIP_DURATION = 2000; // 2 seconds to complete skip
	SDL_Texture* skipBarTexture = nullptr;
	SDL_Rect skipBarRect = { 0, 0, 100, 100 };

private:
	std::string currentVideoPath;
	std::string currentAudioPath;
	void CloseCurrentVideo();
};