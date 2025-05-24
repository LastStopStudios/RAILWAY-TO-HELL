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

// Constructor initializes the module with basic parameters
Ffmpeg::Ffmpeg(bool enabled) : Module()
{
    name = "cutsceneplayer";    // Set the module name
    audioIndex = -1;            // Initialize audio stream index to invalid
    streamIndex = -1;           // Initialize video stream index to invalid
    swr = nullptr;              // Initialize audio resampler pointer to null
    currentVideoPath = "";      // Clear the current video path
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
    audioCodecContext = nullptr;     // Audio codec context for decoding audio
    renderTexture = nullptr;         // Texture used for rendering video frames

    // Set the module state to running
    running = true;                  // Mark the module as active
    return true;                     // Return success
}

// Load a video file from a specified path
bool Ffmpeg::LoadVideo(const char* videoPath)
{
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

    // Open the input video file with FFMPEG
    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) < 0) {   // Abre el archivo o el stream multimedia
        LOG("Failed to open input file: %s", videoPath);  // Log error
        avformat_close_input(&formatContext);             // Close the input if opened
        avformat_free_context(formatContext);             // Free the context
        formatContext = nullptr;                          // Reset pointer
        return true;  // Error (note: true = error)
    }

    // Find input stream information
    if (avformat_find_stream_info(formatContext, NULL) < 0) { // Analiza el archivo abierto para analizar que contiene (video, audio...)
        LOG("Failed to find input stream information");   // Log error
        avformat_close_input(&formatContext);             // Close the input
        avformat_free_context(formatContext);             // Free the context
        formatContext = nullptr;                          // Reset pointer
        return true;  // Error
    }

    // Dump format information for debugging
    av_dump_format(formatContext, 0, videoPath, 0); // Imprime en la consola info detallada del archivo

    // Open codec context for video and audio streams
    if (OpenCodecContext(&streamIndex)) {
        LOG("Failed to open codec contexts");    // Log error
        CloseCurrentVideo();                     // Clean up resources
        return true;  // Error
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

    return false;  // Success (note: false = success)
}

// Clean up resources for the current video
void Ffmpeg::CloseCurrentVideo()
{
    // Close audio device if open
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);  // Close the SDL audio device
        audioDevice = 0;                    // Reset the device ID
    }

    // Free the audio resampler if it exists
    if (swr) {
        swr_free(&swr);    // Free the SwrContext
        swr = nullptr;     // Reset the pointer
    }

    // Free the video codec context
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);  // Free the context
        videoCodecContext = nullptr;               // Reset the pointer
    }

    // Free the audio codec context
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);  // Free the context
        audioCodecContext = nullptr;               // Reset the pointer
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

    // Clear the audio buffer queue
    while (!audioBuffer.empty()) {
        AVPacket pkt = audioBuffer.front();   // Get the front packet
        av_packet_unref(&pkt);                // Unreference the packet
        audioBuffer.pop();                    // Remove from queue
    }

    // Reset stream indices
    streamIndex = -1;  // Reset video stream index
    audioIndex = -1;   // Reset audio stream index

    // Clear the path
    currentVideoPath = "";  // Reset the video path
}

// Overloaded ConvertPixels to use with paths
bool Ffmpeg::ConvertPixels(const char* videoPath)
{
    // Load the video if it's not the current one
    if (LoadVideo(videoPath)) {
        return true;  // Error loading video
    }

    // Use the original ConvertPixels with the updated indices
    return ConvertPixels(streamIndex, audioIndex);
}

// Open codec contexts for video and audio streams
bool Ffmpeg::OpenCodecContext(int* index)
{
    // Find the best video stream in the file
    int videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        LOG("Failed to find video stream in input file\n");  // Log error
        return true;  // Error
    }
    LOG("Video index found: %d", videoIndex);  // Log success

    // Find the best audio stream in the file
    int audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioIndex < 0) {
        LOG("Failed to find audio stream in input file\n");  // Log warning
        // No audio stream found, continue without audio
        audioIndex = -1;  // Mark as not available with a clear value
    }
    else {
        LOG("Audio index found: %d", audioIndex);  // Log success
    }

    // Open video codec context
    bool videoOpenFailed = OpenVideoCodecContext(videoIndex);
    if (videoOpenFailed) {
        LOG("Failed to open video codec context");  // Log error
        return true;  // Error
    }

    // Open audio codec context if audio stream found
    bool audioOpenFailed = false;
    if (audioIndex >= 0) {
        audioOpenFailed = OpenAudioCodecContext(audioIndex);
        if (audioOpenFailed) {
            LOG("Failed to open audio codec context");  // Log warning
            // Just log the failure, we can continue without audio
            audioIndex = -1;  // Mark as not available
        }
    }

    // Store indices
    *index = videoIndex;              // Store video index in the parameter
    this->audioIndex = audioIndex;    // Store audio index in the member variable

    LOG("Successfully opened codec contexts. Video index: %d, Audio index: %d", videoIndex, audioIndex);
    return false; // Everything is set up successfully
}

// Open and configure video codec context
bool Ffmpeg::OpenVideoCodecContext(int videoIndex)
{
    // Find the decoder for the video codec
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);
    if (!codec) {
        LOG("Failed to find video codec!\n");  // Log error
        return true;  // Error
    }

    // Allocate a codec context for the decoder
    videoCodecContext = avcodec_alloc_context3(codec); // Crea un objeto AVCodecContext, que contiene toda la configuración necesaria para decodificar el video.
    if (!videoCodecContext) {
        LOG("Failed to allocate the video codec context\n");  // Log error
        return true;  // Error
    }

    // Copy video codec parameters to the decoder context
    if (avcodec_parameters_to_context(videoCodecContext, formatContext->streams[videoIndex]->codecpar) < 0) { //Copia los parámetros del stream
        LOG("Failed to copy video codec parameters to decoder context!\n");  // Log error
        return true;  // Error
    }

    // Open the video codec
    if (avcodec_open2(videoCodecContext, codec, NULL) < 0) { // Inicializa internamente el códec para que empiece a decodificar frames de video, aqui podria estar el problema del desfase
        LOG("Failed to open video codec\n");  // Log error
        return true;  // Error
    }

    // Return success if all steps completed
    return false;  // Success (note: false = success)
}

// Audio timer callback function - processes audio at regular intervals
Uint32 audioTimerCallback(Uint32 interval, void* param)
{
    // Cast the parameter to Ffmpeg object
    Ffmpeg* cutscenePlayer = static_cast<Ffmpeg*>(param);
    if (cutscenePlayer) {
        cutscenePlayer->ProcessAudio();  // Process audio in the cutscene player
    }
    return interval;  // Return same interval for continuous callback
}

// Open and configure audio codec context
bool Ffmpeg::OpenAudioCodecContext(int audioIndex)
{
    // Find the decoder for the audio codec
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[audioIndex]->codecpar->codec_id);
    if (!codec) {
        LOG("Failed to find audio codec!");  // Log error
        return true;  // Error
    }

    // Allocate and configure the codec context
    audioCodecContext = avcodec_alloc_context3(codec);
    if (!audioCodecContext) {
        LOG("Failed to allocate the audio codec context");  // Log error
        return true;  // Error
    }

    // Copy codec parameters and open the codec
    if (avcodec_parameters_to_context(audioCodecContext, formatContext->streams[audioIndex]->codecpar) < 0 ||
        avcodec_open2(audioCodecContext, codec, NULL) < 0) {
        LOG("Failed to set up audio codec context");  // Log error
        return true;  // Error
    }

    // Create and configure the audio resampler
    SwrContext* swr = swr_alloc();
    if (!swr) {
        LOG("Failed to allocate resampler context");  // Log error
        return true;  // Error
    }

    // Set up channel layouts
    AVChannelLayout in_ch_layout, out_ch_layout;
    av_channel_layout_default(&in_ch_layout, audioCodecContext->ch_layout.nb_channels);  // Input layout
    av_channel_layout_default(&out_ch_layout, 2);  // Stereo output layout

    // Configure resampler options
    av_opt_set_int(swr, "in_sample_rate", audioCodecContext->sample_rate, 0);  // Input sample rate
    av_opt_set_int(swr, "out_sample_rate", 44100, 0);  // Output sample rate (44.1kHz)
    av_opt_set_sample_fmt(swr, "in_sample_fmt", audioCodecContext->sample_fmt, 0);  // Input format
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);  // Output format (16-bit PCM)
    av_opt_set_chlayout(swr, "in_chlayout", &audioCodecContext->ch_layout, 0);  // Input channel layout
    av_opt_set_chlayout(swr, "out_chlayout", &out_ch_layout, 0);  // Output channel layout

    // Initialize the resampler
    if (swr_init(swr) < 0) {
        LOG("Failed to initialize resampler");  // Log error
        swr_free(&swr);  // Free the resampler
        return true;  // Error
    }

    // Save the resampler in the member variable
    this->swr = swr;

    // Configure SDL audio specifications
    SDL_AudioSpec wantedSpec, obtainedSpec;
    wantedSpec.freq = 44100;                // Sample rate
    wantedSpec.format = AUDIO_S16SYS;       // 16-bit audio
    wantedSpec.channels = 2;                // Stereo
    wantedSpec.silence = 0;                 // Silence value
    wantedSpec.samples = 4096;              // Buffer size
    wantedSpec.callback = NULL;             // Using queue mode
    wantedSpec.userdata = NULL;             // No user data

    // Open the audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wantedSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
        LOG("Failed to open audio device: %s", SDL_GetError());  // Log SDL error
        return true;  // Error
    }

    // Start audio playback
    SDL_PauseAudioDevice(audioDevice, 0);  // Unpause the device (0 = play)

    return false;  // Success
}

// Handle SDL events
bool Ffmpeg::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            // Usuario cierra la ventana
            running = false;
            return false;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                // ESC presionado - detener reproducción
                LOG("ESC pressed - stopping video playback");
                running = false;
                CloseCurrentVideo();
                return false;

                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    return true; // Continuar reproducción
}

// Process audio from the audio buffer
void Ffmpeg::ProcessAudio()
{
    // Skip if no audio context or device
    if (audioIndex < 0 || !audioCodecContext || audioDevice == 0) {
        return;  // No audio to process
    }

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();  // Allocate an audio frame, Asigna memoria para un nuevo AVFrame, que se usará para almacenar el audio decodificado
    if (!frame) {
        return;  // Memory allocation failed
    }

    // Variables for converted data
    uint8_t* outBuffer = NULL;
    int outBufferSize = 0;

    // Process each packet from the audio buffer
    if (!audioBuffer.empty()) {
        packet = audioBuffer.front();  // Get the next packet
        audioBuffer.pop();             // Remove it from the queue

        // Send the packet to the decoder
        int ret = avcodec_send_packet(audioCodecContext, &packet);
        if (ret < 0) {
            LOG("Error sending packet to audio decoder: %d", ret);  // Log error
            av_packet_unref(&packet);  // Free the packet
            av_frame_free(&frame);     // Free the frame
            return;
        }

        // Receive the decoded frame
        ret = avcodec_receive_frame(audioCodecContext, frame);
        if (ret < 0) {
            LOG("Error receiving frame from audio decoder: %d", ret);  // Log error
            av_packet_unref(&packet);  // Free the packet
            av_frame_free(&frame);     // Free the frame
            return;
        }

        // Calculate the output buffer size based on the resampling ratio
        int outSamples = av_rescale_rnd(
            swr_get_delay(swr, audioCodecContext->sample_rate) + frame->nb_samples,
            44100,  // Output sample rate
            audioCodecContext->sample_rate,
            AV_ROUND_UP
        );

        int outChannels = 2;  // Stereo
        outBufferSize = outSamples * outChannels * 2;  // 2 bytes per sample for S16

        // Allocate memory for the output buffer
        outBuffer = (uint8_t*)av_malloc(outBufferSize);
        if (!outBuffer) {
            LOG("Failed to allocate output buffer");  // Log error
            av_packet_unref(&packet);  // Free the packet
            av_frame_free(&frame);     // Free the frame
            return;
        }

        // Convert the audio using the resampler
        ret = swr_convert(
            swr,
            &outBuffer, outSamples,                  // Output
            (const uint8_t**)frame->data, frame->nb_samples  // Input
        );

        if (ret < 0) {
            LOG("Error converting audio: %d", ret);  // Log error
            av_freep(&outBuffer);      // Free the output buffer
            av_packet_unref(&packet);  // Free the packet
            av_frame_free(&frame);     // Free the frame
            return;
        }

        // Calculate actual size of the converted data
        int convertedSize = ret * outChannels * 2;

        // Queue the audio data for playback
        SDL_QueueAudio(audioDevice, outBuffer, convertedSize);

        // Free resources
        av_freep(&outBuffer);      // Free the output buffer
        av_packet_unref(&packet);  // Free the packet
    }

    // Free the frame
    av_frame_free(&frame);
}

// Main function to decode and convert video frames for playback
bool Ffmpeg::ConvertPixels(int videoIndex, int audioIndex)
{
    // Check if we have a valid format context
    if (!formatContext || !videoCodecContext) {
        LOG("No video loaded or invalid codec context. Load a video first.");
        return true;  // Error
    }

    // Check if the indices match the loaded video
    if (videoIndex != streamIndex || audioIndex != this->audioIndex) {
        LOG("Warning: Called ConvertPixels with different indices than loaded video");
        // Could handle this differently, here we use the provided indices
    }

    LOG("ConvertPixels called with videoIndex=%d, audioIndex=%d", videoIndex, audioIndex);

    // Master clock for synchronization
    double videoClock = 0.0;

    // Packet for demuxed data
    AVPacket packet;

    // Allocate video frames
    AVFrame* srcFrame = av_frame_alloc();  // Source frame (as decoded)
    if (!srcFrame) {
        LOG("Failed to allocate source frame");
        return false;  // Error
    }
    AVFrame* dstFrame = av_frame_alloc();  // Destination frame (converted format)
    if (!dstFrame) {
        LOG("Failed to allocate destination frame");
        av_frame_free(&srcFrame);  // Free source frame
        return false;  // Error
    }

    // Allocate memory for the image data of the destination frame
    AllocImage(dstFrame);

    // Create a scaling context for format conversion
    struct SwsContext* sws_ctx = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,  // Source
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,         // Destination
        SWS_BILINEAR, NULL, NULL, NULL  // Scaling algorithm and parameters
    );

    // Get time bases for video and audio streams
    AVRational videoTimeBase = formatContext->streams[videoIndex]->time_base;
    AVRational audioTimeBase;
    if (audioIndex >= 0) {
        audioTimeBase = formatContext->streams[audioIndex]->time_base;
    }

    // Clear any old audio data from the queue
    while (!audioBuffer.empty()) {
        AVPacket pkt = audioBuffer.front();
        av_packet_unref(&pkt);
        audioBuffer.pop();
    }

    // Flush audio device if audio is available
    if (audioIndex >= 0) {
        SDL_ClearQueuedAudio(audioDevice);
    }

    // Record the start time for playback synchronization
    Uint32 videoStartTime = SDL_GetTicks();
    int64_t firstPts = AV_NOPTS_VALUE;

    // Main packet reading loop
    while (av_read_frame(formatContext, &packet) >= 0 && running)
    {
        // Process SDL events
        if (!HandleEvents()) {
            av_packet_unref(&packet);  // Free the packet
            break;  // Exit the loop if HandleEvents returns false
        }

        // Handle video packets
        if (packet.stream_index == videoIndex)
        {
            // Send packet to decoder
            int sendResult = avcodec_send_packet(videoCodecContext, &packet);
            if (sendResult < 0) {
                LOG("Error sending video packet to decoder: %d", sendResult);
                av_packet_unref(&packet);
                continue;  // Skip to next packet
            }

            // Receive decoded frame
            int receiveResult = avcodec_receive_frame(videoCodecContext, srcFrame);
            if (receiveResult < 0) {
                LOG("Error receiving video frame from decoder: %d", receiveResult);
                av_packet_unref(&packet);
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

            // Update video clock
            videoClock = timeInSeconds;

            // Scale and convert the frame to the needed format
            sws_scale(sws_ctx, (uint8_t const* const*)srcFrame->data,
                srcFrame->linesize, 0, videoCodecContext->height,
                dstFrame->data, dstFrame->linesize);

            // Update the texture with the new frame data
            SDL_UpdateYUVTexture(renderTexture, &renderRect,
                dstFrame->data[0], dstFrame->linesize[0],  // Y plane
                dstFrame->data[1], dstFrame->linesize[1],  // U plane
                dstFrame->data[2], dstFrame->linesize[2]   // V plane
            );

            // Render the current frame
            RenderCutscene();

            // Wait if necessary to maintain correct timing
            if (delayMs > 0 && delayMs < 1000) {  // Cap delay at 1 second
                SDL_Delay(delayMs);  // Wait for the next frame
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
                packet.pts += offsetInTimeBase;  // Apply offset to presentation timestamp
                if (packet.dts != AV_NOPTS_VALUE) {
                    packet.dts += offsetInTimeBase;  // Apply offset to decoding timestamp
                }
            }

            // Make a copy of the packet for the audio buffer
            AVPacket* audioPkt = av_packet_alloc();
            av_packet_ref(audioPkt, &packet);

            // Process audio immediately to reduce latency
            avcodec_send_packet(audioCodecContext, audioPkt);

            AVFrame* audioFrame = av_frame_alloc();
            if (avcodec_receive_frame(audioCodecContext, audioFrame) == 0) {
                // Process the decoded audio frame
                ProcessAudioFrame(audioFrame);
            }

            // Clean up
            av_frame_free(&audioFrame);
            av_packet_free(&audioPkt);
        }

        // Free the packet
        av_packet_unref(&packet);
    }

    // Clean up resources
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    sws_freeContext(sws_ctx);

    return false;  // Success
}

// Process a decoded audio frame
void Ffmpeg::ProcessAudioFrame(AVFrame* frame)
{
    // Check for valid inputs
    if (!frame || !swr || audioDevice == 0) {
        return;  // Cannot process
    }

    // Calculate the number of output samples
    int outSamples = av_rescale_rnd(
        swr_get_delay(swr, audioCodecContext->sample_rate) + frame->nb_samples,
        44100,  // Output sample rate
        audioCodecContext->sample_rate,
        AV_ROUND_UP
    );

    // Calculate buffer size for stereo output
    int outChannels = 2;  // Stereo
    int outBufferSize = outSamples * outChannels * 2;  // 2 bytes per sample for S16

    // Allocate memory for output buffer
    uint8_t* outBuffer = (uint8_t*)av_malloc(outBufferSize);
    if (!outBuffer) {
        LOG("Failed to allocate audio output buffer");
        return;  // Memory allocation failed
    }

    // Convert audio using the resampler
    int convertedSamples = swr_convert(
        swr,
        &outBuffer, outSamples,                        // Output buffer and size
        (const uint8_t**)frame->data, frame->nb_samples  // Input data and samples
    );

    // Check for conversion errors
    if (convertedSamples < 0) {
        LOG("Error converting audio: %d", convertedSamples);
        av_freep(&outBuffer);  // Free buffer
        return;  // Conversion failed
    }

    // Calculate actual size of converted data
    int convertedSize = convertedSamples * outChannels * 2;

    // Queue audio data for playback
    SDL_QueueAudio(audioDevice, outBuffer, convertedSize);

    // Free buffer
    av_freep(&outBuffer);
}

// Allocate memory for an image frame
bool Ffmpeg::AllocImage(AVFrame* image)
{
    // Set the pixel format, width, and height of the image frame
    image->format = AV_PIX_FMT_YUV420P;           // YUV 4:2:0 format
    image->width = videoCodecContext->width;      // Width from codec context
    image->height = videoCodecContext->height;    // Height from codec context

    // Allocate memory for the image data
    av_image_alloc(image->data, image->linesize,
        image->width, image->height, (AVPixelFormat)image->format, 32);

    return false;  // Success
}

// Render the current video frame
void Ffmpeg::RenderCutscene()
{
    // Clear the renderer
    SDL_RenderClear(Engine::GetInstance().render.get()->renderer);

    // Copy the video texture to the renderer
    SDL_RenderCopy(Engine::GetInstance().render.get()->renderer, renderTexture, NULL, NULL);

    // Draw interactive elements if hovered
    if (isHover1) Engine::GetInstance().render->DrawTexture(texture1, position1.x, position1.y, NULL);
    if (isHover2) Engine::GetInstance().render->DrawTexture(texture2, position2.x, position2.y, NULL);

    // Present the rendered frame
    SDL_RenderPresent(Engine::GetInstance().render.get()->renderer);
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

    // Close the video and audio codec contexts
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

    // Close the current video and clean up remaining resources
    CloseCurrentVideo();

    return true;  // Cleanup successful
}