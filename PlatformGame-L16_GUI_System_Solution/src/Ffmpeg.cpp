#include "Ffmpeg.h"
#include "Textures.h"
#include "Log.h"
#include "Render.h"
#include "Window.h"
#include "Audio.h"
#include "Scene.h"
#include "Map.h"
#include "Point.h"
#include "Engine.h"

Ffmpeg::Ffmpeg(bool enabled) : Module()
{
    name = "cutsceneplayer";
    audioIndex = -1;
}

Ffmpeg::~Ffmpeg()
{
}

bool Ffmpeg::Awake()
{
    LOG("Loading CutscenePlayer");
    bool ret = true;

    return ret;
}

bool Ffmpeg::Start()
{
    // File path of the video to be played
    const char* file = "Assets/Video/test.mp4";

    // Allocate memory for the format context
    formatContext = avformat_alloc_context();

    // Open the input video file with FFMPEG
    if (avformat_open_input(&formatContext, file, NULL, NULL) < 0) {
        printf("Failed to open input file");
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        return true;
    }

    // Find input stream information
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        printf("Failed to find input stream information");
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        return true;
    }

    // Dump format information
    av_dump_format(formatContext, 0, file, 0);

    // Open codec context for video and audio streams
    OpenCodecContext(&streamIndex);                     // Funcion para abrir audio y video

    // Create SDL texture for rendering video frames
    renderTexture = SDL_CreateTexture(Engine::GetInstance().render.get()->renderer, SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING, videoCodecContext->width, videoCodecContext->height);
    if (!renderTexture) {
        printf("Failed to create texture - %s\n", SDL_GetError());
        return true;
    }

    // Set up SDL rectangle for video rendering
    renderRect = { 0, 0, videoCodecContext->width, videoCodecContext->height };

    // Set the module state to running
    running = true;
    return true;
}
bool Ffmpeg::OpenCodecContext(int* index)
{
    // Find video stream in the file
    int videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        LOG("Failed to find video stream in input file\n");
        return true;
    }
    LOG("Video index found: %d", videoIndex);

    // Find audio stream in the file
    int audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioIndex < 0) {
        LOG("Failed to find audio stream in input file\n");
        // No audio stream found, continue without audio
        audioIndex = -1;  // Mark as not available with a clear value
    }
    else {
        LOG("Audio index found: %d", audioIndex);
    }

    // Open video codec context
    bool videoOpenFailed = OpenVideoCodecContext(videoIndex);
    if (videoOpenFailed) {
        LOG("Failed to open video codec context");
        return true;
    }

    // Open audio codec context if audio stream found
    bool audioOpenFailed = false;
    if (audioIndex >= 0) {
        audioOpenFailed = OpenAudioCodecContext(audioIndex);
        if (audioOpenFailed) {
            LOG("Failed to open audio codec context");
            // Just log the failure, we can continue without audio
            audioIndex = -1;  // Mark as not available
        }
    }

    *index = videoIndex;
    this->audioIndex = audioIndex;  // Store the audio index, even if -1

    LOG("Successfully opened codec contexts. Video index: %d, Audio index: %d", videoIndex, audioIndex);
    return false; // Everything is set up
}
bool Ffmpeg::OpenVideoCodecContext(int videoIndex)
{
    // Find the decoder for the video codec
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);
    if (!codec) {
        LOG("Failed to find video codec!\n");
        return true;
    }

    // Allocate a codec context for the decoder
    videoCodecContext = avcodec_alloc_context3(codec);
    if (!videoCodecContext) {
        LOG("Failed to allocate the video codec context\n");
        return true;
    }

    // Copy video codec parameters to the decoder context
    if (avcodec_parameters_to_context(videoCodecContext, formatContext->streams[videoIndex]->codecpar) < 0) {
        LOG("Failed to copy video codec parameters to decoder context!\n");
        return true;
    }

    // Open the video codec
    if (avcodec_open2(videoCodecContext, codec, NULL) < 0) {
        LOG("Failed to open video codec\n");
        return true;
    }

// Devuelve true en todos los casos donde falle el abrir y cargar bien el video, si todo sale bien devuelve false. por lo que no se haria un return temprano en la func. de arriba

    return false;
}

Uint32 audioTimerCallback(Uint32 interval, void* param)
{
    Ffmpeg* cutscenePlayer = static_cast<Ffmpeg*>(param);
    if (cutscenePlayer) {
        cutscenePlayer->ProcessAudio();
    }
    return interval;
}

bool Ffmpeg::OpenAudioCodecContext(int audioIndex)
{
    // Find the decoder for the audio codec
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[audioIndex]->codecpar->codec_id);
    if (!codec) {
        printf("Failed to find audio codec!\n");
        return true;
    }

    // Allocate a codec context for the decoder
    audioCodecContext = avcodec_alloc_context3(codec);
    if (!audioCodecContext) {
        printf("Failed to allocate the audio codec context\n");
        return true;
    }

    // Copy audio codec parameters to the decoder context
    if (avcodec_parameters_to_context(audioCodecContext, formatContext->streams[audioIndex]->codecpar) < 0) {
        printf("Failed to copy audio codec parameters to decoder context!\n");
        return true;
    }

    // Open the audio codec
    if (avcodec_open2(audioCodecContext, codec, NULL) < 0) {
        printf("Failed to open audio codec\n");
        return true;
    }

    // Set up SDL audio device
    SDL_AudioSpec wantedSpec, obtainedSpec;
    wantedSpec.freq = audioCodecContext->sample_rate;
    switch (audioCodecContext->sample_fmt) {
    case AV_SAMPLE_FMT_U8:
    case AV_SAMPLE_FMT_U8P:
        wantedSpec.format = AUDIO_U8;
        break;
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S16P:
        wantedSpec.format = AUDIO_S16SYS;
        break;
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S32P:
        wantedSpec.format = AUDIO_S32SYS;
        break;
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_FLTP:
        wantedSpec.format = AUDIO_F32SYS;
        break;
    }
    wantedSpec.channels = 2;
    wantedSpec.silence = 0;
    wantedSpec.samples = audioCodecContext->frame_size;
    wantedSpec.callback = NULL;
    wantedSpec.userdata = NULL;

    // Open the audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wantedSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
        printf("Failed to open audio device %s\n", SDL_GetError());
        return true;
    }

    SDL_AddTimer((1000 * wantedSpec.samples) / wantedSpec.freq, audioTimerCallback, this);

    // Start audio playback
    SDL_PauseAudioDevice(audioDevice, 0);

    system("cls");
    return false;
}

void Ffmpeg::ProcessAudio()
{
    // Allocate memory for an audio frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return;
    }

    Uint8* stream;
    AVPacket packet;

    // Process each packet in the audio buffer
    if (!audioBuffer.empty()) {
        // Get the first packet from the audio buffer
        packet = audioBuffer.front();
        audioBuffer.pop();

        // Send the packet to the audio decoder
        int ret = avcodec_send_packet(audioCodecContext, &packet);
        if (ret < 0) {
            av_frame_free(&frame);
            return;
        }

        // Receive decoded audio frame
        ret = avcodec_receive_frame(audioCodecContext, frame);
        if (ret < 0) {
            av_frame_free(&frame);
            return;
        }

        // Get the pointer to the audio data
        stream = reinterpret_cast<Uint8*>(frame->data[0]);

        // Queue the audio data for playback
        SDL_QueueAudio(audioDevice, stream, frame->linesize[0]);

        // Unreference the frame
        av_frame_unref(frame);
    }

    // Free memory allocated for the audio frame
    av_frame_free(&frame);
}

bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    LOG("ConvertPixels called with videoIndex=%d, audioIndex=%d", videoIndex, audioIndex);

    // Calculate time per frame based on the average frame rate of the video stream
    int time = 1000 * formatContext->streams[videoIndex]->avg_frame_rate.den
        / formatContext->streams[videoIndex]->avg_frame_rate.num;

    AVPacket packet;

    // Allocate memory for source and destination frames
    AVFrame* srcFrame = av_frame_alloc();
    if (!srcFrame) {
        LOG("Failed to allocate source frame");
        return false;
    }
    AVFrame* dstFrame = av_frame_alloc();
    if (!dstFrame) {
        LOG("Failed to allocate destination frame");
        av_frame_free(&srcFrame);
        return false;
    }

    // Allocate memory for the image data of the destination frame
    AllocImage(dstFrame);

    // Create a scaling context to convert the pixel format of the video frames
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    int packetCount = 0;
    int videoPackets = 0;
    int audioPackets = 0;

    // Loop through each packet in the video stream
    while (av_read_frame(formatContext, &packet) >= 0 && running)
    {
        packetCount++;
        LOG("Processing packet %d with stream_index=%d", packetCount, packet.stream_index);

        if (packet.stream_index == videoIndex)
        {
            videoPackets++;
            LOG("Found video packet %d", videoPackets);

            // Send packet to video decoder
            avcodec_send_packet(videoCodecContext, &packet);
            // Receive decoded frame
            int ret = avcodec_receive_frame(videoCodecContext, srcFrame);
            if (ret) {
                LOG("Failed to receive frame, error=%d", ret);
                continue;
            }

            // Scale the source frame to the destination frame
            sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                srcFrame->linesize, 0, videoCodecContext->height,
                dstFrame->data, dstFrame->linesize);

            // Update the SDL texture with the scaled YUV data
            SDL_UpdateYUVTexture(renderTexture, &renderRect,
                dstFrame->data[0], dstFrame->linesize[0],
                dstFrame->data[1], dstFrame->linesize[1],
                dstFrame->data[2], dstFrame->linesize[2]
            );

            // Render the video
            RenderCutscene();
            // SelectCharacter();

             // Wait for the specified time before processing the next frame
            SDL_Delay(time);

            // Unreference the packet to release its resources
            av_packet_unref(&packet);
        }
        else if (audioIndex >= 0 && packet.stream_index == audioIndex)  // Fix this condition
        {
            audioPackets++;
            LOG("Found audio packet %d. Pushing to audio buffer.", audioPackets);
            // Push to the audio buffer
            audioBuffer.push(packet);
        }
        else {
            LOG("Unknown packet stream_index=%d, not matching video(%d) or audio(%d)",
                packet.stream_index, videoIndex, audioIndex);
            // Unreference the packet to release its resources
            av_packet_unref(&packet);
        }
    }

    LOG("Finished processing packets. Total: %d, Video: %d, Audio: %d",
        packetCount, videoPackets, audioPackets);

    // Free memory allocated for the destination frame
    av_freep(&dstFrame->data[0]);

    // Free memory allocated for source and destination frames
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);

    return false;
}
bool Ffmpeg::AllocImage(AVFrame* image)
{
    // Set the pixel format, width, and height of the image frame
    image->format = AV_PIX_FMT_YUV420P;
    image->width = videoCodecContext->width;
    image->height = videoCodecContext->height;

    // Allocate memory for the image frame
    av_image_alloc(image->data, image->linesize,
        image->width, image->height, (AVPixelFormat)image->format, 32);

    return false;
}

void Ffmpeg::RenderCutscene()
{
    SDL_RenderClear(Engine::GetInstance().render.get()->renderer);

    SDL_RenderCopy(Engine::GetInstance().render.get()->renderer, renderTexture, NULL, NULL);

    if (isHover1) Engine::GetInstance().render->DrawTexture(texture1, position1.x, position1.y, NULL);
    if (isHover2) Engine::GetInstance().render->DrawTexture(texture2, position2.x, position2.y, NULL);

    SDL_RenderPresent(Engine::GetInstance().render.get()->renderer);
}

bool Ffmpeg::Update(float dt)
{

    return true;
}

bool Ffmpeg::CleanUp()
{
    LOG("Freeing cutscene player");

    // Close the vide and audio codec contexts
    avcodec_free_context(&videoCodecContext);
    avcodec_free_context(&audioCodecContext);

    // Close and free the format contexts
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);

    // Close audio device
    SDL_CloseAudioDevice(audioDevice);

    // Destroy textures
    SDL_DestroyTexture(renderTexture);
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);

    return true;
}