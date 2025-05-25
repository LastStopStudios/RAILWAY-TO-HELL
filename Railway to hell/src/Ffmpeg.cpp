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
#include <cmath>

Ffmpeg::Ffmpeg(bool enabled) : Module()
{
    name = "cutsceneplayer";
    streamIndex = -1;
    currentVideoPath = "";
    currentAudioPath = "";
    isAudioPlaying = false;
    isSkipping = false;
    skipCompleted = false;
    skipStartTime = 0;
    skipBarTexture = nullptr;
    running = false;
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
    LOG("Loading CutscenePlayer");

    formatContext = nullptr;
    videoCodecContext = nullptr;
    renderTexture = nullptr;

    running = true;
    return true;
}

// Genera la ruta del archivo WAV basándose en la ruta del video
std::string Ffmpeg::GetWavPathFromVideo(const char* videoPath)
{
    std::string videoPathStr(videoPath);
    std::string wavPath;

    size_t lastSlash = videoPathStr.find_last_of('/');
    size_t lastDot = videoPathStr.find_last_of('.');

    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
        std::string filename = videoPathStr.substr(lastSlash + 1, lastDot - lastSlash - 1);
        wavPath = "Assets/Videos/Audios/" + filename + ".wav";
    }

    LOG("Generated WAV path: %s from video: %s", wavPath.c_str(), videoPath);
    return wavPath;
}

bool Ffmpeg::LoadAndPlayWavAudio(const char* wavPath)
{
    StopWavAudio();

    bool playResult = Engine::GetInstance().audio->PlayMusic(wavPath, 1.0f);

    if (!playResult) {
        LOG("Failed to load and play WAV music: %s", wavPath);
        return false;
    }

    isAudioPlaying = true;
    currentAudioPath = wavPath;
    LOG("Successfully loaded and playing WAV music: %s", wavPath);
    return true;
}

void Ffmpeg::StopWavAudio()
{
    if (isAudioPlaying) {
        // Solo parar el audio actual, no reproducir "Nothing.ogg"
        Mix_HaltMusic();

        LOG("Stopping WAV music playback");
        isAudioPlaying = false;
        currentAudioPath = "";
    }
}

bool Ffmpeg::LoadVideo(const char* videoPath)
{
    if (!videoPath || strlen(videoPath) == 0) {
        LOG("Invalid video path provided");
        return true;
    }

    // Si el video ya está cargado, no lo recargamos
    if (currentVideoPath == videoPath && formatContext != nullptr) {
        return false;
    }

    CloseCurrentVideo();
    currentVideoPath = videoPath;

    LOG("Loading video: %s", videoPath);

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        LOG("Failed to allocate format context");
        return true;
    }

    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) < 0) {
        LOG("Failed to open input file: %s", videoPath);
        avformat_free_context(formatContext);
        formatContext = nullptr;
        return true;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOG("Failed to find input stream information");
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
        return true;
    }

    av_dump_format(formatContext, 0, videoPath, 0);

    // Abrir contexto de codec solo para video (sin audio del video)
    if (OpenCodecContext(&streamIndex)) {
        LOG("Failed to open codec contexts");
        CloseCurrentVideo();
        return true;
    }

    if (!videoCodecContext) {
        LOG("Video codec context is null after opening");
        CloseCurrentVideo();
        return true;
    }

    // Crear textura SDL para renderizar los frames del video
    renderTexture = SDL_CreateTexture(
        Engine::GetInstance().render.get()->renderer,
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        videoCodecContext->width,
        videoCodecContext->height
    );

    if (!renderTexture) {
        LOG("Failed to create texture - %s\n", SDL_GetError());
        CloseCurrentVideo();
        return true;
    }

    renderRect = { 0, 0, videoCodecContext->width, videoCodecContext->height };

    // Cargar y reproducir el audio WAV correspondiente
    std::string wavPath = GetWavPathFromVideo(videoPath);
    if (!wavPath.empty()) {
        LoadAndPlayWavAudio(wavPath.c_str());
    }

    return false;
}

void Ffmpeg::CloseCurrentVideo()
{
    StopWavAudio();

    if (videoCodecContext) {
        avcodec_flush_buffers(videoCodecContext);
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
    }

    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }

    if (renderTexture) {
        SDL_DestroyTexture(renderTexture);
        renderTexture = nullptr;
    }

    streamIndex = -1;
    currentVideoPath = "";
    currentAudioPath = "";
    isAudioPlaying = false;
    isSkipping = false;
    skipCompleted = false;
    skipStartTime = 0;
    running = true;
}

bool Ffmpeg::ConvertPixels(const char* videoPath)
{
    if (LoadVideo(videoPath)) {
        return true;
    }

    return ConvertPixels(streamIndex, -1);
}

// Abrir contexto de codec solo para el stream de video
bool Ffmpeg::OpenCodecContext(int* index)
{
    if (!formatContext) {
        LOG("Format context is null");
        return true;
    }

    int videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        LOG("Failed to find video stream in input file\n");
        return true;
    }
    LOG("Video index found: %d", videoIndex);

    LOG("Skipping audio stream detection - using separate WAV files");

    bool videoOpenFailed = OpenVideoCodecContext(videoIndex);
    if (videoOpenFailed) {
        LOG("Failed to open video codec context");
        return true;
    }

    *index = videoIndex;

    LOG("Successfully opened video codec context. Video index: %d", videoIndex);
    return false;
}

bool Ffmpeg::OpenVideoCodecContext(int videoIndex)
{
    if (!formatContext || videoIndex < 0 || videoIndex >= formatContext->nb_streams) {
        LOG("Invalid format context or video index");
        return true;
    }

    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);
    if (!codec) {
        LOG("Failed to find video codec!\n");
        return true;
    }

    videoCodecContext = avcodec_alloc_context3(codec);
    if (!videoCodecContext) {
        LOG("Failed to allocate the video codec context\n");
        return true;
    }

    if (avcodec_parameters_to_context(videoCodecContext, formatContext->streams[videoIndex]->codecpar) < 0) {
        LOG("Failed to copy video codec parameters to decoder context!\n");
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
        return true;
    }

    if (avcodec_open2(videoCodecContext, codec, NULL) < 0) {
        LOG("Failed to open video codec\n");
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
        return true;
    }

    return false;
}

bool Ffmpeg::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            return false;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                LOG("ESC pressed - stopping video playback");
                running = false;
                return false;

            case SDLK_SPACE:
                // Iniciar proceso de skip al presionar espacio
                if (!isSkipping && !skipCompleted) {
                    isSkipping = true;
                    skipStartTime = SDL_GetTicks();
                    LOG("Skip initiated - hold SPACE to continue");
                }
                break;

            default:
                break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                // Cancelar skip si se suelta la tecla antes de completar
                if (isSkipping && !skipCompleted) {
                    isSkipping = false;
                    LOG("Skip cancelled - SPACE released");
                }
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    UpdateSkipSystem();
    return true;
}

void Ffmpeg::UpdateSkipSystem()
{
    if (!isSkipping || skipCompleted) return;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - skipStartTime;

    // Si se completó el tiempo de skip
    if (elapsedTime >= SKIP_DURATION) {
        LOG("Skip completed - stopping audio and marking for end");
        skipCompleted = true;
        isSkipping = false;

        StopWavAudio();
        running = false;
        return;
    }

    // Verificar si la tecla SPACE sigue presionada
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (!keyState[SDL_SCANCODE_SPACE]) {
        isSkipping = false;
        LOG("Skip cancelled - SPACE not held");
    }
}

// Renderizar círculo que se llena progresivamente de derecha a izquierda (todo el círculo)
void Ffmpeg::RenderSkipBar(float progress)
{
    if (!isSkipping) return;

    SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;

    int centerX = Engine::GetInstance().window->width - 80;
    int centerY = 700;
    int radius = 20;

    // Asegurar que el progreso esté entre 0 y 1
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    float fillAngle = progress * 2.0f * M_PI;

    SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);  // Rojo oscuro sólido

    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                float angle = atan2f(y, x); // Rango: -? a ?
                if (angle < 0.0f) angle += 2.0f * M_PI; // Normalizar a 0 a 2?

                if (angle <= fillAngle) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
    }
}

// Función principal para decodificar y convertir frames de video
bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    if (!formatContext || !videoCodecContext) {
        LOG("No video loaded or invalid codec context. Load a video first.");
        return true;
    }

    if (videoIndex != streamIndex) {
        LOG("Warning: Called ConvertPixels with different video index than loaded video");
    }

    LOG("ConvertPixels called with videoIndex=%d (audio disabled)", videoIndex);

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        LOG("Failed to allocate packet");
        return true;
    }

    AVFrame* srcFrame = av_frame_alloc();
    if (!srcFrame) {
        LOG("Failed to allocate source frame");
        av_packet_free(&packet);
        return true;
    }

    AVFrame* dstFrame = av_frame_alloc();
    if (!dstFrame) {
        LOG("Failed to allocate destination frame");
        av_frame_free(&srcFrame);
        av_packet_free(&packet);
        return true;
    }

    if (AllocImage(dstFrame)) {
        LOG("Failed to allocate image for destination frame");
        av_frame_free(&srcFrame);
        av_frame_free(&dstFrame);
        av_packet_free(&packet);
        return true;
    }

    // Crear contexto de escalado para conversión de formato
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    if (!sws_ctx) {
        LOG("Failed to create scaling context");
        av_frame_free(&srcFrame);
        av_frame_free(&dstFrame);
        av_packet_free(&packet);
        return true;
    }

    AVRational videoTimeBase = formatContext->streams[videoIndex]->time_base;
    Uint32 videoStartTime = SDL_GetTicks();
    int64_t firstPts = AV_NOPTS_VALUE;

    // Bucle principal de lectura de paquetes
    while (av_read_frame(formatContext, packet) >= 0 && running && !skipCompleted)
    {
        if (skipCompleted) {
            LOG("Skip completed - breaking main loop");
            av_packet_unref(packet);
            break;
        }

        if (packet->data == nullptr || packet->size <= 0) {
            av_packet_unref(packet);
            continue;
        }

        if (!HandleEvents()) {
            av_packet_unref(packet);
            break;
        }

        if (!running || skipCompleted) {
            av_packet_unref(packet);
            break;
        }

        // Procesar solo paquetes de video
        if (packet->stream_index == videoIndex)
        {
            if (!videoCodecContext) {
                LOG("Video codec context became null during playback");
                av_packet_unref(packet);
                break;
            }

            int sendResult = avcodec_send_packet(videoCodecContext, packet);
            if (sendResult < 0) {
                LOG("Error sending video packet to decoder: %d", sendResult);
                av_packet_unref(packet);
                continue;
            }

            int receiveResult = avcodec_receive_frame(videoCodecContext, srcFrame);
            if (receiveResult < 0) {
                if (receiveResult != AVERROR(EAGAIN)) {
                    LOG("Error receiving video frame from decoder: %d", receiveResult);
                }
                av_packet_unref(packet);
                continue;
            }

            // Obtener timestamp de presentación para este frame
            int64_t pts = srcFrame->pts;
            if (pts == AV_NOPTS_VALUE) {
                pts = 0;
            }

            if (firstPts == AV_NOPTS_VALUE) {
                firstPts = pts;
                videoStartTime = SDL_GetTicks();
            }

            // Calcular el tiempo de visualización en milisegundos
            double timeInSeconds = av_q2d(videoTimeBase) * (pts - firstPts);
            int displayTimeMs = (int)(timeInSeconds * 1000.0);

            int elapsedMs = SDL_GetTicks() - videoStartTime;
            int delayMs = displayTimeMs - elapsedMs;

            // Escalar y convertir el frame al formato necesario
            int scaleResult = sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                srcFrame->linesize, 0, videoCodecContext->height,
                dstFrame->data, dstFrame->linesize);

            if (scaleResult < 0) {
                LOG("Error scaling frame");
                av_packet_unref(packet);
                continue;
            }

            // Actualizar la textura con los datos del nuevo frame
            int updateResult = SDL_UpdateYUVTexture(renderTexture, &renderRect,
                dstFrame->data[0], dstFrame->linesize[0],  // Y plane
                dstFrame->data[1], dstFrame->linesize[1],  // U plane
                dstFrame->data[2], dstFrame->linesize[2]   // V plane
            );

            if (updateResult < 0) {
                LOG("Failed to update YUV texture: %s", SDL_GetError());
                av_packet_unref(packet);
                continue;
            }

            RenderCutscene();

            // Esperar si es necesario para mantener el timing correcto
            if (delayMs > 0 && delayMs < 1000) {
                SDL_Delay(delayMs);
            }
        }

        av_packet_unref(packet);
    }

    // Limpiar recursos
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    av_packet_free(&packet);
    sws_freeContext(sws_ctx);

    LOG("Video playback ended - cleaning up");
    CloseCurrentVideo();

    return false;
}

bool Ffmpeg::AllocImage(AVFrame* image)
{
    if (!image || !videoCodecContext) {
        LOG("Invalid image frame or codec context");
        return true;
    }

    image->format = AV_PIX_FMT_YUV420P;
    image->width = videoCodecContext->width;
    image->height = videoCodecContext->height;

    int result = av_image_alloc(image->data, image->linesize,
        image->width, image->height, (AVPixelFormat)image->format, 32);

    if (result < 0) {
        LOG("Failed to allocate image data");
        return true;
    }

    return false;
}

void Ffmpeg::RenderCutscene()
{
    SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;

    if (!renderer || !renderTexture) {
        LOG("Invalid renderer or render texture");
        return;
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, renderTexture, NULL, NULL);

    // Dibujar elementos interactivos si están activos
    if (isHover1) Engine::GetInstance().render->DrawTexture(texture1, position1.x, position1.y, NULL);
    if (isHover2) Engine::GetInstance().render->DrawTexture(texture2, position2.x, position2.y, NULL);

    // Renderizar barra de skip si está activa
    if (isSkipping) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - skipStartTime;
        float progress = (float)elapsedTime / (float)SKIP_DURATION;
        progress = progress > 1.0f ? 1.0f : progress;

        RenderSkipBar(progress);
    }

    SDL_RenderPresent(renderer);
}

bool Ffmpeg::Update(float dt)
{
    HandleEvents();
    return true;
}

bool Ffmpeg::CleanUp()
{
    LOG("Freeing cutscene player");

    CloseCurrentVideo();

    if (skipBarTexture) {
        SDL_DestroyTexture(skipBarTexture);
        skipBarTexture = nullptr;
    }

    return true;
}