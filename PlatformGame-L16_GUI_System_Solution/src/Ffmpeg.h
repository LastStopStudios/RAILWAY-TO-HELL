#pragma once
#define __CUTSCENEPLAYER_H__ 
#define __CUTSCENEPLAYER_H__ 

#include "Point.h"
#include "Module.h"
#include <queue>
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_thread.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
//#include <libpostproc/postprocess.h>
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

	bool Awake(pugi::xml_node& conf);

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	bool OpenCodecContext(int* index);
	bool OpenVideoCodecContext(int index);
	bool OpenAudioCodecContext(int index);
	bool ConvertPixels(int videoIndex, int audioIndex);
	bool AllocImage(AVFrame* dstFrame);
	void RenderCutscene();
	void SelectCharacter();//esta dentro del codigo comentado
	void ProcessAudio();

public:
	int streamIndex = -1;
	AVFormatContext* formatContext;
	AVCodecContext* videoCodecContext;
	AVCodecContext* audioCodecContext;

	SDL_AudioDeviceID audioDevice;
	int audioStreamIndex;

	SDL_Texture* renderTexture;
	SDL_Texture* texture1;
	SDL_Texture* texture2;
	SDL_Rect renderRect;
	bool running;

	SDL_Rect rect1 = { 300,100,250,500 };
	SDL_Rect rect2 = { 600,100,250,500 };
	iPoint position1 = { 300,500 };
	iPoint position2 = { 600,500 };
	bool isHover1 = false;
	bool isHover2 = false;

	std::queue<AVPacket> audioBuffer;

};