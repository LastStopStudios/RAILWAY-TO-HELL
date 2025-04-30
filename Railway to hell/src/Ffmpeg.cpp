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
    swr = nullptr;
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
    const char* file = "Assets/Videos/test.mp4";

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
        LOG("Failed to find audio codec!");
        return true;
    }

    // Allocate and configure the codec context
    audioCodecContext = avcodec_alloc_context3(codec);
    if (!audioCodecContext) {
        LOG("Failed to allocate the audio codec context");
        return true;
    }

    // Copy codec parameters and open the codec
    if (avcodec_parameters_to_context(audioCodecContext, formatContext->streams[audioIndex]->codecpar) < 0 ||
        avcodec_open2(audioCodecContext, codec, NULL) < 0) {
        LOG("Failed to set up audio codec context");
        return true;
    }

    // Create and configure the audio resampler
    SwrContext* swr = swr_alloc();
    if (!swr) {
        LOG("Failed to allocate resampler context");
        return true;
    }

    // Set up channel layouts
    AVChannelLayout in_ch_layout, out_ch_layout;
    av_channel_layout_default(&in_ch_layout, audioCodecContext->ch_layout.nb_channels);
    av_channel_layout_default(&out_ch_layout, 2); // Stereo output

    // Configure resampler
    av_opt_set_int(swr, "in_sample_rate", audioCodecContext->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", audioCodecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_chlayout(swr, "in_chlayout", &audioCodecContext->ch_layout, 0);
    av_opt_set_chlayout(swr, "out_chlayout", &out_ch_layout, 0);

    if (swr_init(swr) < 0) {
        LOG("Failed to initialize resampler");
        swr_free(&swr);
        return true;
    }

    // Save the resampler
    this->swr = swr;

    // Configure SDL audio
    SDL_AudioSpec wantedSpec, obtainedSpec;
    wantedSpec.freq = 44100;
    wantedSpec.format = AUDIO_S16SYS;
    wantedSpec.channels = 2;
    wantedSpec.silence = 0;
    wantedSpec.samples = 4096;  // Larger buffer size for smoother playback
    wantedSpec.callback = NULL; // Using queue mode
    wantedSpec.userdata = NULL;

    // Open audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wantedSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
        LOG("Failed to open audio device: %s", SDL_GetError());
        return true;
    }

    // Start audio playback
    SDL_PauseAudioDevice(audioDevice, 0);

    return false;
}

bool Ffmpeg::HandleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            return false;
        }
    }
    return true;
}

void Ffmpeg::ProcessAudio()
{
    // Skip if no audio context or device
    if (audioIndex < 0 || !audioCodecContext || audioDevice == 0) {
        return;
    }

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return;
    }

    // To store converted data
    uint8_t* outBuffer = NULL;
    int outBufferSize = 0;

    // Process each packet from the audio buffer
    if (!audioBuffer.empty()) {
        packet = audioBuffer.front();
        audioBuffer.pop();

        // Send the packet to the decoder
        int ret = avcodec_send_packet(audioCodecContext, &packet);
        if (ret < 0) {
            LOG("Error sending packet to audio decoder: %d", ret);
            av_packet_unref(&packet);
            av_frame_free(&frame);
            return;
        }

        // Receive the decoded frame
        ret = avcodec_receive_frame(audioCodecContext, frame);
        if (ret < 0) {
            LOG("Error receiving frame from audio decoder: %d", ret);
            av_packet_unref(&packet);
            av_frame_free(&frame);
            return;
        }

        // Calculate the output buffer size
        int outSamples = av_rescale_rnd(
            swr_get_delay(swr, audioCodecContext->sample_rate) + frame->nb_samples,
            44100, // Output sample rate
            audioCodecContext->sample_rate,
            AV_ROUND_UP
        );

        int outChannels = 2; // Stereo
        outBufferSize = outSamples * outChannels * 2; // 2 bytes per sample for S16

        // Allocate memory for the output buffer
        outBuffer = (uint8_t*)av_malloc(outBufferSize);
        if (!outBuffer) {
            LOG("Failed to allocate output buffer");
            av_packet_unref(&packet);
            av_frame_free(&frame);
            return;
        }

        // Convert the audio using the resampler
        ret = swr_convert(
            swr,
            &outBuffer, outSamples,
            (const uint8_t**)frame->data, frame->nb_samples
        );

        if (ret < 0) {
            LOG("Error converting audio: %d", ret);
            av_freep(&outBuffer);
            av_packet_unref(&packet);
            av_frame_free(&frame);
            return;
        }

        // Actual size of the converted data
        int convertedSize = ret * outChannels * 2;

        // Play the audio
        SDL_QueueAudio(audioDevice, outBuffer, convertedSize);

        // Free resources
        av_freep(&outBuffer);
        av_packet_unref(&packet);
    }

    // Free the frame
    av_frame_free(&frame);
}
bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    LOG("ConvertPixels called with videoIndex=%d, audioIndex=%d", videoIndex, audioIndex);

    // Master clock for synchronization
    double videoClock = 0.0;

    AVPacket packet;

    // Allocate frames
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

    // Create a scaling context
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    // Get time bases for video and audio streams
    AVRational videoTimeBase = formatContext->streams[videoIndex]->time_base;
    AVRational audioTimeBase;
    if (audioIndex >= 0) {
        audioTimeBase = formatContext->streams[audioIndex]->time_base;
    }

    // Clear any old audio data
    while (!audioBuffer.empty()) {
        AVPacket pkt = audioBuffer.front();
        av_packet_unref(&pkt);
        audioBuffer.pop();
    }

    // Flush audio device
    if (audioIndex >= 0) {
        SDL_ClearQueuedAudio(audioDevice);
    }

    Uint32 videoStartTime = SDL_GetTicks();
    int64_t firstPts = AV_NOPTS_VALUE;

    // Main packet reading loop
    while (av_read_frame(formatContext, &packet) >= 0 && running)
    {
        // Process events
        if (!HandleEvents()) {
            av_packet_unref(&packet);
            break;
        }

        // Handle video packets
        if (packet.stream_index == videoIndex)
        {
            // Send packet to decoder
            int sendResult = avcodec_send_packet(videoCodecContext, &packet);
            if (sendResult < 0) {
                LOG("Error sending video packet to decoder: %d", sendResult);
                av_packet_unref(&packet);
                continue;
            }

            // Receive decoded frame
            int receiveResult = avcodec_receive_frame(videoCodecContext, srcFrame);
            if (receiveResult < 0) {
                LOG("Error receiving video frame from decoder: %d", receiveResult);
                av_packet_unref(&packet);
                continue;
            }

            // Get presentation timestamp for this frame
            int64_t pts = srcFrame->pts;
            if (pts == AV_NOPTS_VALUE) {
                pts = 0;
            }

            // Initialize the first PTS value
            if (firstPts == AV_NOPTS_VALUE) {
                firstPts = pts;
                videoStartTime = SDL_GetTicks();
            }

            // Calculate the display time in milliseconds
            double timeInSeconds = av_q2d(videoTimeBase) * (pts - firstPts);
            int displayTimeMs = (int)(timeInSeconds * 1000.0);

            // Current playback time
            int elapsedMs = SDL_GetTicks() - videoStartTime;

            // Calculate how long to wait for this frame
            int delayMs = displayTimeMs - elapsedMs;

            // Update video clock
            videoClock = timeInSeconds;

            // Scale and convert the frame
            sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                srcFrame->linesize, 0, videoCodecContext->height,
                dstFrame->data, dstFrame->linesize);

            // Update texture
            SDL_UpdateYUVTexture(renderTexture, &renderRect,
                dstFrame->data[0], dstFrame->linesize[0],
                dstFrame->data[1], dstFrame->linesize[1],
                dstFrame->data[2], dstFrame->linesize[2]
            );

            // Render video
            RenderCutscene();

            // Wait if necessary to maintain correct timing
            if (delayMs > 0 && delayMs < 1000) {
                SDL_Delay(delayMs);
            }
        }
        // Handle audio packets
        else if (audioIndex >= 0 && packet.stream_index == audioIndex)
        {
            // Apply audio sync offset - adjust packet PTS
            if (packet.pts != AV_NOPTS_VALUE) {
                // Convert audioSyncOffset from milliseconds to the packet's time base
                AVRational msTimeBase = { 1, 1000 };
                int64_t offsetInTimeBase = av_rescale_q(audioSyncOffset,
                    msTimeBase,
                    audioTimeBase);
                packet.pts += offsetInTimeBase;
                if (packet.dts != AV_NOPTS_VALUE) {
                    packet.dts += offsetInTimeBase;
                }
            }

            // Make a copy of the packet for the audio buffer
            AVPacket* audioPkt = av_packet_alloc();
            av_packet_ref(audioPkt, &packet);

            // Process audio immediately to reduce latency
            avcodec_send_packet(audioCodecContext, audioPkt);

            AVFrame* audioFrame = av_frame_alloc();
            if (avcodec_receive_frame(audioCodecContext, audioFrame) == 0) {
                // Process audio frame
                ProcessAudioFrame(audioFrame);
            }

            av_frame_free(&audioFrame);
            av_packet_free(&audioPkt);
        }

        // Release the packet
        av_packet_unref(&packet);
    }

    // Clean up
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    sws_freeContext(sws_ctx);

    return false;
}
void Ffmpeg::ProcessAudioFrame(AVFrame* frame)
{
    if (!frame || !swr || audioDevice == 0) {
        return;
    }

    // Calculate output buffer size
    int outSamples = av_rescale_rnd(
        swr_get_delay(swr, audioCodecContext->sample_rate) + frame->nb_samples,
        44100, // Output sample rate
        audioCodecContext->sample_rate,
        AV_ROUND_UP
    );

    int outChannels = 2; // Stereo
    int outBufferSize = outSamples * outChannels * 2; // 2 bytes per sample for S16

    // Allocate memory for output buffer
    uint8_t* outBuffer = (uint8_t*)av_malloc(outBufferSize);
    if (!outBuffer) {
        LOG("Failed to allocate audio output buffer");
        return;
    }

    // Convert audio
    int convertedSamples = swr_convert(
        swr,
        &outBuffer, outSamples,
        (const uint8_t**)frame->data, frame->nb_samples
    );

    if (convertedSamples < 0) {
        LOG("Error converting audio: %d", convertedSamples);
        av_freep(&outBuffer);
        return;
    }

    // Calculate actual size of converted data
    int convertedSize = convertedSamples * outChannels * 2;

    // Queue audio data
    SDL_QueueAudio(audioDevice, outBuffer, convertedSize);

    // Free buffer
    av_freep(&outBuffer);
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