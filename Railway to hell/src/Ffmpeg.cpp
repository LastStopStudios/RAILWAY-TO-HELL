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

// Constructor initializes the module with basic parameters
Ffmpeg::Ffmpeg(bool enabled) : Module()
{
    name = "cutsceneplayer";    // Set the module name
    streamIndex = -1;           // Initialize video stream index to invalid
    currentVideoPath = "";      // Clear the current video path
    currentAudioPath = "";      // Clear the current audio path
    isAudioPlaying = false;     // No audio loaded initially

    // Initialize skip system variables
    isSkipping = false;
    skipCompleted = false;      // Initialize skip completed flag
    skipStartTime = 0;
    skipBarTexture = nullptr;
    running = false;            // Initialize running state
}

// Destructor - cleanup is handled in CleanUp()
Ffmpeg::~Ffmpeg()
{
}

// Initial awakening of the module - called before Start()
bool Ffmpeg::Awake()
{
    LOG("Loading CutscenePlayer");   // Log the initialization
    bool ret = true;                 // Set return value to success

    return ret;                      // Return success
}

// Start the module - called after Awake()
bool Ffmpeg::Start()
{
    LOG("Loading CutscenePlayer");   // Log the start of the module

    // Initialize pointers to null but don't load any video yet
    formatContext = nullptr;         // Format context holds file information
    videoCodecContext = nullptr;     // Video codec context for decoding video
    renderTexture = nullptr;         // Texture used for rendering video frames

    // Set the module state to running
    running = true;                  // Mark the module as active
    return true;                     // Return success
}

// Generate WAV file path from video file path
std::string Ffmpeg::GetWavPathFromVideo(const char* videoPath)
{
    std::string videoPathStr(videoPath);
    std::string wavPath;

    // Find the last slash and dot to extract filename
    size_t lastSlash = videoPathStr.find_last_of('/');
    size_t lastDot = videoPathStr.find_last_of('.');

    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
        // Extract filename without extension
        std::string filename = videoPathStr.substr(lastSlash + 1, lastDot - lastSlash - 1);

        // Build WAV path: Assets/Videos/Audios/filename.wav
        wavPath = "Assets/Videos/Audios/" + filename + ".wav";
    }

    LOG("Generated WAV path: %s from video: %s", wavPath.c_str(), videoPath);
    return wavPath;
}

// Load and play WAV audio file using PlayMusic
bool Ffmpeg::LoadAndPlayWavAudio(const char* wavPath)
{
    // Stop any currently playing audio
    StopWavAudio();

    // Load and play the music using the Audio module's PlayMusic method
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
        // CAMBIO: Solo parar el audio actual, no reproducir "Nothing.ogg"
        // Esto permite que otros sistemas reproduzcan música después
        Mix_HaltMusic(); // Si usas SDL_mixer
        // O simplemente no reproducir nada en lugar de "Nothing.ogg"

        LOG("Stopping WAV music playback");
        isAudioPlaying = false;
        currentAudioPath = "";
    }
}

// Load a video file from a specified path
bool Ffmpeg::LoadVideo(const char* videoPath)
{
    // Validate input
    if (!videoPath || strlen(videoPath) == 0) {
        LOG("Invalid video path provided");
        return true; // Error
    }

    // If the video is already loaded, don't reload it
    if (currentVideoPath == videoPath && formatContext != nullptr) {
        return false;  // Already loaded, return success (note: false = success)
    }

    // Close any previously opened video
    CloseCurrentVideo();

    // Save the current video path
    currentVideoPath = videoPath;

    LOG("Loading video: %s", videoPath);  // Log the video being loaded

    // Allocate memory for the format context
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        LOG("Failed to allocate format context");
        return true; // Error
    }

    // Open the input video file with FFMPEG
    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) < 0) {
        LOG("Failed to open input file: %s", videoPath);  // Log error
        avformat_free_context(formatContext);             // Free the context
        formatContext = nullptr;                          // Reset pointer
        return true;  // Error (note: true = error)
    }

    // Find input stream information
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOG("Failed to find input stream information");   // Log error
        avformat_close_input(&formatContext);             // Close the input
        avformat_free_context(formatContext);             // Free the context
        formatContext = nullptr;                          // Reset pointer
        return true;  // Error
    }

    // Dump format information for debugging
    av_dump_format(formatContext, 0, videoPath, 0);

    // Open codec context for video only (no audio from video)
    if (OpenCodecContext(&streamIndex)) {
        LOG("Failed to open codec contexts");    // Log error
        CloseCurrentVideo();                     // Clean up resources
        return true;  // Error
    }

    // Validate codec context was properly created
    if (!videoCodecContext) {
        LOG("Video codec context is null after opening");
        CloseCurrentVideo();
        return true; // Error
    }

    // Create SDL texture for rendering video frames
    renderTexture = SDL_CreateTexture(
        Engine::GetInstance().render.get()->renderer,  // Get the renderer
        SDL_PIXELFORMAT_YV12,                          // Set YUV format
        SDL_TEXTUREACCESS_STREAMING,                   // Streaming access for frequent updates
        videoCodecContext->width,                      // Width from codec context
        videoCodecContext->height                      // Height from codec context
    );

    if (!renderTexture) {
        LOG("Failed to create texture - %s\n", SDL_GetError());  // Log SDL error
        CloseCurrentVideo();                                     // Clean up
        return true;  // Error
    }

    // Set up SDL rectangle for video rendering
    renderRect = { 0, 0, videoCodecContext->width, videoCodecContext->height };

    // Load and play corresponding WAV audio
    std::string wavPath = GetWavPathFromVideo(videoPath);
    if (!wavPath.empty()) {
        LoadAndPlayWavAudio(wavPath.c_str());
    }

    return false;  // Success (note: false = success)
}

void Ffmpeg::CloseCurrentVideo()
{
    // Stop any playing WAV audio
    StopWavAudio();

    // Flush and free the video codec context
    if (videoCodecContext) {
        avcodec_flush_buffers(videoCodecContext);  // Flush remaining frames
        avcodec_free_context(&videoCodecContext);  // Free the context
        videoCodecContext = nullptr;               // Reset the pointer
    }

    // Close and free the format context
    if (formatContext) {
        avformat_close_input(&formatContext);      // Close the input
        avformat_free_context(formatContext);      // Free the context
        formatContext = nullptr;                   // Reset the pointer
    }

    // Destroy the render texture
    if (renderTexture) {
        SDL_DestroyTexture(renderTexture);    // Destroy the SDL texture
        renderTexture = nullptr;              // Reset the pointer
    }

    // Reset stream index
    streamIndex = -1;  // Reset video stream index

    // Clear the paths
    currentVideoPath = "";  // Reset the video path
    currentAudioPath = "";  // Reset the audio path

    // Reset audio state
    isAudioPlaying = false;  // Reset audio playing flag

    // Reset skip system
    isSkipping = false;
    skipCompleted = false;    // Reset the skip completed flag
    skipStartTime = 0;

    // Reset running to allow new videos
    running = true;  // Allow new videos to be played
}

// Overloaded ConvertPixels to use with paths
bool Ffmpeg::ConvertPixels(const char* videoPath)
{
    // Load the video if it's not the current one
    if (LoadVideo(videoPath)) {
        return true;  // Error loading video
    }

    // Use the original ConvertPixels with only video index (no audio index needed)
    return ConvertPixels(streamIndex, -1);
}

// Open codec context for video stream only
bool Ffmpeg::OpenCodecContext(int* index)
{
    // Validate format context
    if (!formatContext) {
        LOG("Format context is null");
        return true; // Error
    }

    // Find the best video stream in the file
    int videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        LOG("Failed to find video stream in input file\n");  // Log error
        return true;  // Error
    }
    LOG("Video index found: %d", videoIndex);  // Log success

    // We skip audio stream detection since we're using separate WAV files
    LOG("Skipping audio stream detection - using separate WAV files");

    // Open video codec context
    bool videoOpenFailed = OpenVideoCodecContext(videoIndex);
    if (videoOpenFailed) {
        LOG("Failed to open video codec context");  // Log error
        return true;  // Error
    }

    // Store video index
    *index = videoIndex;              // Store video index in the parameter

    LOG("Successfully opened video codec context. Video index: %d", videoIndex);
    return false; // Everything is set up successfully
}

// Open and configure video codec context
bool Ffmpeg::OpenVideoCodecContext(int videoIndex)
{
    // Validate inputs
    if (!formatContext || videoIndex < 0 || videoIndex >= formatContext->nb_streams) {
        LOG("Invalid format context or video index");
        return true; // Error
    }

    // Find the decoder for the video codec
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);
    if (!codec) {
        LOG("Failed to find video codec!\n");  // Log error
        return true;  // Error
    }

    // Allocate a codec context for the decoder
    videoCodecContext = avcodec_alloc_context3(codec);
    if (!videoCodecContext) {
        LOG("Failed to allocate the video codec context\n");  // Log error
        return true;  // Error
    }

    // Copy video codec parameters to the decoder context
    if (avcodec_parameters_to_context(videoCodecContext, formatContext->streams[videoIndex]->codecpar) < 0) {
        LOG("Failed to copy video codec parameters to decoder context!\n");  // Log error
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
        return true;  // Error
    }

    // Open the video codec
    if (avcodec_open2(videoCodecContext, codec, NULL) < 0) {
        LOG("Failed to open video codec\n");  // Log error
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
        return true;  // Error
    }

    // Return success if all steps completed
    return false;  // Success (note: false = success)
}

// Handle SDL events
bool Ffmpeg::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            // User closes the window
            running = false;
            return false;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                // ESC pressed - stop playback
                LOG("ESC pressed - stopping video playback");
                running = false;
                return false;

            case SDLK_SPACE:
                // Start skip process when space is pressed
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
                // Cancel skip if key is released before completion
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

    // Update skip system
    UpdateSkipSystem();

    return true; // Continue playback
}

// Update skip system
void Ffmpeg::UpdateSkipSystem()
{
    if (!isSkipping || skipCompleted) return;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - skipStartTime;

    // If skip time is completed
    if (elapsedTime >= SKIP_DURATION) {
        LOG("Skip completed - stopping audio and marking for end");
        skipCompleted = true;
        isSkipping = false;

        // IMPORTANT: Stop the audio when skip is completed
        StopWavAudio();

        running = false;  // Signal to stop the main loop
        return;
    }

    // Check if SPACE key is still being held
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (!keyState[SDL_SCANCODE_SPACE]) {
        isSkipping = false;
        LOG("Skip cancelled - SPACE not held");
    }
}

// Render skip bar with circular progress
void Ffmpeg::RenderSkipBar(float progress)
{
    if (!isSkipping) return;

    SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;

    // Bar position (top right corner)
    int centerX = Engine::GetInstance().window->width - 80;
    int centerY = 80;
    int radius = 30;

    // Draw background circle (semi-transparent gray)
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 180);
    for (int angle = 0; angle < 360; angle += 2) {
        for (int r = radius - 3; r <= radius; r++) {
            int x = centerX + (int)(r * cos(angle * M_PI / 180.0));
            int y = centerY + (int)(r * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    // Draw circular progress (bright yellow)
    SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
    int maxAngle = (int)(360 * progress);

    for (int angle = -90; angle < -90 + maxAngle; angle += 2) {
        // Draw lines from center to edge
        for (int r = 8; r < radius - 2; r++) {
            int x = centerX + (int)(r * cos(angle * M_PI / 180.0));
            int y = centerY + (int)(r * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    // Draw circle border (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int angle = 0; angle < 360; angle += 2) {
        int x = centerX + (int)(radius * cos(angle * M_PI / 180.0));
        int y = centerY + (int)(radius * sin(angle * M_PI / 180.0));
        SDL_RenderDrawPoint(renderer, x, y);

        // Inner circle
        int x2 = centerX + (int)((radius - 6) * cos(angle * M_PI / 180.0));
        int y2 = centerY + (int)((radius - 6) * sin(angle * M_PI / 180.0));
        SDL_RenderDrawPoint(renderer, x2, y2);
    }
}

// Main function to decode and convert video frames for playback (video only)
bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    // Check if we have a valid format context
    if (!formatContext || !videoCodecContext) {
        LOG("No video loaded or invalid codec context. Load a video first.");
        return true;  // Error
    }

    // Check if the video index matches the loaded video
    if (videoIndex != streamIndex) {
        LOG("Warning: Called ConvertPixels with different video index than loaded video");
    }

    LOG("ConvertPixels called with videoIndex=%d (audio disabled)", videoIndex);

    // Packet for demuxed data
    AVPacket* packet = av_packet_alloc();  // Use av_packet_alloc instead of deprecated av_init_packet
    if (!packet) {
        LOG("Failed to allocate packet");
        return true; // Error
    }

    // Allocate video frames
    AVFrame* srcFrame = av_frame_alloc();  // Source frame (as decoded)
    if (!srcFrame) {
        LOG("Failed to allocate source frame");
        av_packet_free(&packet);
        return true;  // Error
    }
    AVFrame* dstFrame = av_frame_alloc();  // Destination frame (converted format)
    if (!dstFrame) {
        LOG("Failed to allocate destination frame");
        av_frame_free(&srcFrame);  // Free source frame
        av_packet_free(&packet);
        return true;  // Error
    }

    // Allocate memory for the image data of the destination frame
    if (AllocImage(dstFrame)) {
        LOG("Failed to allocate image for destination frame");
        av_frame_free(&srcFrame);
        av_frame_free(&dstFrame);
        av_packet_free(&packet);
        return true; // Error
    }

    // Create a scaling context for format conversion
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,  // Source
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,         // Destination
        SWS_BILINEAR, NULL, NULL, NULL  // Scaling algorithm and parameters
    );

    if (!sws_ctx) {
        LOG("Failed to create scaling context");
        av_frame_free(&srcFrame);
        av_frame_free(&dstFrame);
        av_packet_free(&packet);
        return true; // Error
    }

    // Get time base for video stream
    AVRational videoTimeBase = formatContext->streams[videoIndex]->time_base;

    // Record the start time for playback synchronization
    Uint32 videoStartTime = SDL_GetTicks();
    int64_t firstPts = AV_NOPTS_VALUE;

    // Main packet reading loop
    while (av_read_frame(formatContext, packet) >= 0 && running && !skipCompleted)
    {
        // Check if skip has been completed - exit immediately
        if (skipCompleted) {
            LOG("Skip completed - breaking main loop");
            av_packet_unref(packet);
            break;
        }

        // Validate packet
        if (packet->data == nullptr || packet->size <= 0) {
            av_packet_unref(packet);
            continue;
        }

        // Process SDL events
        if (!HandleEvents()) {
            av_packet_unref(packet);  // Free the packet
            break;  // Exit the loop if HandleEvents returns false
        }

        // Double check that we're still supposed to be running
        if (!running || skipCompleted) {
            av_packet_unref(packet);
            break;
        }

        // Handle video packets only
        if (packet->stream_index == videoIndex)
        {
            // Double-check codec context is still valid
            if (!videoCodecContext) {
                LOG("Video codec context became null during playback");
                av_packet_unref(packet);
                break;
            }

            // Send packet to decoder
            int sendResult = avcodec_send_packet(videoCodecContext, packet);
            if (sendResult < 0) {
                LOG("Error sending video packet to decoder: %d", sendResult);
                av_packet_unref(packet);
                continue;  // Skip to next packet
            }

            // Receive decoded frame
            int receiveResult = avcodec_receive_frame(videoCodecContext, srcFrame);
            if (receiveResult < 0) {
                if (receiveResult != AVERROR(EAGAIN)) {
                    LOG("Error receiving video frame from decoder: %d", receiveResult);
                }
                av_packet_unref(packet);
                continue;  // Skip to next packet
            }

            // Get presentation timestamp for this frame
            int64_t pts = srcFrame->pts;
            if (pts == AV_NOPTS_VALUE) {
                pts = 0;  // Use 0 if no valid PTS
            }

            // Initialize the first PTS value
            if (firstPts == AV_NOPTS_VALUE) {
                firstPts = pts;  // Store first frame PTS
                videoStartTime = SDL_GetTicks();  // Record start time
            }

            // Calculate the display time in milliseconds
            double timeInSeconds = av_q2d(videoTimeBase) * (pts - firstPts);
            int displayTimeMs = (int)(timeInSeconds * 1000.0);

            // Get current playback time
            int elapsedMs = SDL_GetTicks() - videoStartTime;

            // Calculate how long to wait for this frame
            int delayMs = displayTimeMs - elapsedMs;

            // Scale and convert the frame to the needed format
            int scaleResult = sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                srcFrame->linesize, 0, videoCodecContext->height,
                dstFrame->data, dstFrame->linesize);

            if (scaleResult < 0) {
                LOG("Error scaling frame");
                av_packet_unref(packet);
                continue;
            }

            // Update the texture with the new frame data
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

            // Render the current frame
            RenderCutscene();

            // Wait if necessary to maintain correct timing
            if (delayMs > 0 && delayMs < 1000) {  // Cap delay at 1 second
                SDL_Delay(delayMs);  // Wait for the next frame
            }
        }
        // Skip audio packets - we're using separate WAV files
        else {
            // Just ignore audio packets from the video file
        }

        // Free the packet
        av_packet_unref(packet);
    }

    // Clean up resources
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    av_packet_free(&packet);
    sws_freeContext(sws_ctx);

    // Clean up video resources after playback ends
    LOG("Video playback ended - cleaning up");
    CloseCurrentVideo();

    return false;  // Success
}

// Allocate memory for an image frame
bool Ffmpeg::AllocImage(AVFrame* image)
{
    if (!image || !videoCodecContext) {
        LOG("Invalid image frame or codec context");
        return true; // Error
    }

    // Set the pixel format, width, and height of the image frame
    image->format = AV_PIX_FMT_YUV420P;           // YUV 4:2:0 format
    image->width = videoCodecContext->width;      // Width from codec context
    image->height = videoCodecContext->height;    // Height from codec context

    // Allocate memory for the image data
    int result = av_image_alloc(image->data, image->linesize,
        image->width, image->height, (AVPixelFormat)image->format, 32);

    if (result < 0) {
        LOG("Failed to allocate image data");
        return true; // Error
    }

    return false;  // Success
}

// Render the current video frame
void Ffmpeg::RenderCutscene()
{
    SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;

    if (!renderer || !renderTexture) {
        LOG("Invalid renderer or render texture");
        return;
    }

    // Clear the renderer
    SDL_RenderClear(renderer);

    // Copy the video texture to the renderer
    SDL_RenderCopy(renderer, renderTexture, NULL, NULL);

    // Draw interactive elements if hovered
    if (isHover1) Engine::GetInstance().render->DrawTexture(texture1, position1.x, position1.y, NULL);
    if (isHover2) Engine::GetInstance().render->DrawTexture(texture2, position2.x, position2.y, NULL);

    // Render skip bar if active
    if (isSkipping) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - skipStartTime;
        float progress = (float)elapsedTime / (float)SKIP_DURATION;
        progress = progress > 1.0f ? 1.0f : progress; // Clamp to 1.0

        RenderSkipBar(progress);
    }

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

// Update function called each frame
bool Ffmpeg::Update(float dt)
{
    HandleEvents();
    // Currently empty, could be used for updating UI elements
    return true;  // Continue running
}

// Clean up all resources
bool Ffmpeg::CleanUp()
{
    LOG("Freeing cutscene player");  // Log cleanup

    // Close the current video and clean up remaining resources
    CloseCurrentVideo();

    if (skipBarTexture) {
        SDL_DestroyTexture(skipBarTexture);
        skipBarTexture = nullptr;
    }

    return true;  // Cleanup successful
}