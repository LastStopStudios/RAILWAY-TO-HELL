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
    const char* file = "Assets/Video/test3.mp4";

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
    if (OpenCodecContext(&streamIndex)) {
        LOG("Failed to open codec contexts");
        return true;
    }

    // Inicializa las variables de sincronización DESPUÉS de tener streamIndex válido
    audio_clock = 0;
    video_clock = 0;
    frame_timer = 0;

    // Asegurarnos que streamIndex es válido antes de usarlo
    if (streamIndex >= 0 && streamIndex < (int)formatContext->nb_streams) {
        frame_delay = av_q2d(formatContext->streams[streamIndex]->time_base);
    }
    else {
        LOG("Invalid video stream index: %d", streamIndex);
        frame_delay = 0.04; // Valor por defecto (25 FPS)
    }

    // Create SDL texture for rendering video frames
    renderTexture = SDL_CreateTexture(Engine::GetInstance().render.get()->renderer,
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        videoCodecContext->width,
        videoCodecContext->height);
    if (!renderTexture) {
        printf("Failed to create texture - %s\n", SDL_GetError());
        return true;
    }

    // Set up SDL rectangle for video rendering
    renderRect = { 0, 0, videoCodecContext->width, videoCodecContext->height };

    // Set the module state to running
    running = true;
    return true;
}bool Ffmpeg::OpenCodecContext(int* index)
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

    // Al final de la función, asegúrate de asignar el índice
    *index = videoIndex;  // Esto debería ser el índice válido del stream de video
    this->audioIndex = audioIndex;

    LOG("Opened streams - Video: %d, Audio: %d", *index, this->audioIndex);

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
    wantedSpec.samples = 1024;
    wantedSpec.callback = NULL;
    wantedSpec.userdata = NULL;

    // Open audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wantedSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
        LOG("Failed to open audio device: %s", SDL_GetError());
        return true;
    }

    // Set up audio timer and start playback
    SDL_AddTimer((1000 * wantedSpec.samples) / wantedSpec.freq, audioTimerCallback, this);
    SDL_PauseAudioDevice(audioDevice, 0);

    return false;
}
void Ffmpeg::ProcessAudio()
{
    // Declarar antes para mantener el alcance
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return;
    }

    // Para almacenar datos convertidos
    uint8_t* outBuffer = NULL;
    int outBufferSize = 0;

    // Procesar cada paquete del buffer de audio
    if (!audioBuffer.empty()) {
        packet = audioBuffer.front();
        audioBuffer.pop();

        // Enviar el paquete al decodificador
        int ret = avcodec_send_packet(audioCodecContext, &packet);
        if (ret < 0) {
            LOG("Error sending packet to audio decoder: %d", ret);
            av_frame_free(&frame);
            return;
        }

        // Recibir el frame decodificado
        ret = avcodec_receive_frame(audioCodecContext, frame);
        if (ret < 0) {
            LOG("Error receiving frame from audio decoder: %d", ret);
            av_frame_free(&frame);
            return;
        }

        // Calcular el tamaño del buffer de salida
        int outSamples = av_rescale_rnd(
            swr_get_delay(swr, audioCodecContext->sample_rate) + frame->nb_samples,
            44100, // Frecuencia de salida
            audioCodecContext->sample_rate,
            AV_ROUND_UP
        );

        int outChannels = 2; // Estéreo
        outBufferSize = outSamples * outChannels * 2; // 2 bytes por muestra para S16

        // Asignar memoria para el buffer de salida
        outBuffer = (uint8_t*)av_malloc(outBufferSize);
        if (!outBuffer) {
            LOG("Failed to allocate output buffer");
            av_frame_free(&frame);
            return;
        }

        // Convertir el audio usando el resampler
        ret = swr_convert(
            swr,
            &outBuffer, outSamples,
            (const uint8_t**)frame->data, frame->nb_samples
        );

        audio_clock = frame->pts * av_q2d(formatContext->streams[audioIndex]->time_base);

        if (ret < 0) {
            LOG("Error converting audio: %d", ret);
            av_freep(&outBuffer);
            av_frame_free(&frame);
            return;
        }

        // El tamaño real de los datos convertidos
        int convertedSize = ret * outChannels * 2;

        // Reproducir el audio
        SDL_QueueAudio(audioDevice, outBuffer, convertedSize);

        // Liberar recursos
        av_freep(&outBuffer);
        av_packet_unref(&packet);
        av_frame_unref(frame);
    }

    // Liberar frame
    av_frame_free(&frame);
}
bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    LOG("ConvertPixels called with videoIndex=%d, audioIndex=%d", videoIndex, audioIndex);

    // Inicialización de variables para sincronización
    double frame_timer = static_cast<double>(SDL_GetTicks()) / 1000.0;
    double frame_delay = av_q2d(formatContext->streams[videoIndex]->time_base);
    double video_clock = 0;
    double audio_clock = 0;

    AVPacket packet;
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

    // Allocate image for dstFrame
    AllocImage(dstFrame);

    // Create scaling context
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    int packetCount = 0;
    int videoPackets = 0;
    int audioPackets = 0;

    while (av_read_frame(formatContext, &packet) >= 0 && running)
    {
        packetCount++;
        LOG("Processing packet %d with stream_index=%d", packetCount, packet.stream_index);

        if (packet.stream_index == videoIndex)
        {
            videoPackets++;
            LOG("Found video packet %d", videoPackets);

            // Send packet to decoder
            if (avcodec_send_packet(videoCodecContext, &packet) < 0) {
                LOG("Error sending video packet to decoder");
                av_packet_unref(&packet);
                continue;
            }

            // Receive decoded frame
            while (avcodec_receive_frame(videoCodecContext, srcFrame) == 0) {
                // Calculate video clock
                if (srcFrame->pts != AV_NOPTS_VALUE) {
                    video_clock = srcFrame->pts * av_q2d(formatContext->streams[videoIndex]->time_base);
                }
                else {
                    video_clock += frame_delay;
                }

                // Calculate delay for synchronization
                double current_time = static_cast<double>(SDL_GetTicks()) / 1000.0;
                double actual_delay = video_clock - frame_timer;

                // Sync with audio if available
                if (audioIndex >= 0) {
                    double diff = video_clock - audio_clock;
                    double sync_threshold = (diff > 0) ? 0.1 : -0.1;

                    // Adjust delay to stay in sync with audio
                    if (diff > sync_threshold) {
                        actual_delay = FFMIN(actual_delay, diff);
                    }
                    else if (diff < -0.5) {
                        // Video is too far behind, skip frame
                        continue;
                    }
                }

                // Wait if needed
                if (actual_delay > 0 && actual_delay < 1.0) {
                    SDL_Delay(static_cast<Uint32>(actual_delay * 1000.0));
                }

                // Convert and render the frame
                sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                    srcFrame->linesize, 0, videoCodecContext->height,
                    dstFrame->data, dstFrame->linesize);

                SDL_UpdateYUVTexture(renderTexture, &renderRect,
                    dstFrame->data[0], dstFrame->linesize[0],
                    dstFrame->data[1], dstFrame->linesize[1],
                    dstFrame->data[2], dstFrame->linesize[2]);

                RenderCutscene();

                // Update frame timer
                frame_timer = video_clock;
            }

            av_packet_unref(&packet);
        }
        else if (audioIndex >= 0 && packet.stream_index == audioIndex)
        {
            audioPackets++;
            LOG("Found audio packet %d. Pushing to audio buffer.", audioPackets);

            // Store audio packet timestamp before pushing to buffer
            if (packet.pts != AV_NOPTS_VALUE) {
                audio_clock = packet.pts * av_q2d(formatContext->streams[audioIndex]->time_base);
            }

            audioBuffer.push(packet);
        }
        else
        {
            LOG("Unknown packet stream_index=%d, not matching video(%d) or audio(%d)",
                packet.stream_index, videoIndex, audioIndex);
            av_packet_unref(&packet);
        }
    }

    LOG("Finished processing packets. Total: %d, Video: %d, Audio: %d",
        packetCount, videoPackets, audioPackets);

    // Cleanup
    av_freep(&dstFrame->data[0]);
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    sws_freeContext(sws_ctx);

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