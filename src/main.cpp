#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <switch.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/petit2d.h"
#include "DemoContext.h"
#include "ScreenIntro.h"
#include "ScreenMain.h"

#define AUDIOFREQ 48000
#define AUDIOBUFF 2048

struct Surface
{
    int     width   = 1280;
    int     height  = 720;
    bool    docked  = false;

    Petit2D::Sprite::Sprite sprite;
    glm::mat4 matrix;
} screen;

struct SDL
{
    SDL_GLContext       glContext   = nullptr;
    SDL_Window*         window      = nullptr;
    SDL_AudioDeviceID   audioDevice = 0;
} sdl;

struct Demo
{
    enum Step : int
    {
        NONE    = 0,
        INTRO   = 1,
        MAIN    = 2
    };

    Step        step        = Step::NONE;
    DemoContext context;
    ScreenIntro screenIntro;
    ScreenMain  screenMain;
} demo;

void audioCallback(void* userdata, uint8_t* stream, int len)
{
    auto context = static_cast<DemoContext*>(userdata);
    memset(stream, 0, len);

    auto size = (size_t) len >> 2;
    auto reads = context->module->read_interleaved_stereo(AUDIOFREQ, size, reinterpret_cast<int16_t*>(stream));
    if (reads != size)
    {
        // loop
        context->module->read_interleaved_stereo(AUDIOFREQ, size - reads, reinterpret_cast<int16_t*>(&stream[reads]));
    }
}

int main(int argc, char *argv[])
{
    auto exitRequest = false;
    auto lastTime = 0ul;

    romfsInit();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);

    SDL_AudioSpec obtainedAudioSpec;
    SDL_AudioSpec wantedAudioSpec;
    wantedAudioSpec.callback = audioCallback;
    wantedAudioSpec.userdata = &demo.context;
    wantedAudioSpec.samples = AUDIOBUFF;
    wantedAudioSpec.channels = 2;
    wantedAudioSpec.format = AUDIO_S16SYS;
    wantedAudioSpec.freq = AUDIOFREQ;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    sdl.window = SDL_CreateWindow("Rebellion", 0, 0, screen.width, screen.height, SDL_WINDOW_OPENGL);
    sdl.audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wantedAudioSpec, &obtainedAudioSpec, 0);
    sdl.glContext = SDL_GL_CreateContext(sdl.window);

    SDL_SetWindowSize(sdl.window, screen.width, screen.height);
    SDL_GameControllerOpen(0);
    SDL_GL_MakeCurrent(sdl.window, sdl.glContext);
    SDL_GL_SetSwapInterval(1);

    gladLoadGL();
    Petit2D::Create();
    demo.context.load(8);

    screen.matrix = glm::ortho(0.0f, (float) screen.width, (float) screen.height, 0.0f, 0.0f, -1.0f);
    screen.sprite.width = Petit2D::Texture::GetWidth(demo.context.frameBufferTexture);
    screen.sprite.height = Petit2D::Texture::GetHeight(demo.context.frameBufferTexture);
    screen.sprite.s = 0.0f;
    screen.sprite.t = 1.0f;
    screen.sprite.p = 1.0f;
    screen.sprite.q = 0.0f;

    SDL_Event sdlEvent;
    SDL_PauseAudioDevice(sdl.audioDevice, false);
    while (!exitRequest)
    {
        // SDL Events
        while (SDL_PollEvent(&sdlEvent))
        {
            switch (sdlEvent.type)
            {
            case SDL_QUIT:
                exitRequest = true;
            break;

            case SDL_CONTROLLERBUTTONDOWN:
                if (sdlEvent.cbutton.which == 0)
                {
                    if (sdlEvent.cbutton.button == SDL_CONTROLLER_BUTTON_START)
                    {
                        // Exit if "+" pressed
                        SDL_Event quitEvent;
                        quitEvent.type = SDL_QUIT;
                        SDL_PushEvent(&quitEvent);
                    }
                }
            break;

            default:
            break;
            }
        }

        // Check docked/console mode
        switch (appletGetOperationMode())
        {
        case AppletOperationMode_Handheld:
            if (screen.docked == true)
            {
                screen.width = 1280;
                screen.height = 720;
                screen.docked = false;
                screen.matrix = glm::ortho(0.0f, (float) screen.width, (float) screen.height, 0.0f, 0.0f, -1.0f);
                SDL_SetWindowSize(sdl.window, screen.width, screen.height);
            }
        break;

        case AppletOperationMode_Console:
            if (screen.docked == false)
            {
                screen.width = 1920;
                screen.height = 1080;
                screen.docked = true;
                screen.matrix = glm::ortho(0.0f, (float) screen.width, (float) screen.height, 0.0f, 0.0f, -1.0f);
                SDL_SetWindowSize(sdl.window, screen.width, screen.height);
            }
        break;

        default:
        break;
        }

        // Render
        auto currentTime = SDL_GetPerformanceCounter();
        auto timerFrequency = SDL_GetPerformanceFrequency();
        auto elapsed = lastTime > 0 ? (float)((double)(currentTime - lastTime) / timerFrequency) : (float)(1.0f / 60.0f);
        lastTime = currentTime;

        auto scaleWidth = floor(screen.width / screen.sprite.width);
        auto scaleHeight = floor(screen.height / screen.sprite.height);
        auto scale = scaleWidth > scaleHeight ? scaleHeight : scaleWidth;

        screen.sprite.x = screen.width * 0.5f;
        screen.sprite.y = screen.height * 0.5f;
        screen.sprite.scale_x = scale;
        screen.sprite.scale_y = scale;

        switch (demo.step)
        {
        case Demo::Step::NONE:
            demo.step = Demo::Step::INTRO;
            demo.screenIntro.enter(demo.context);
            demo.screenIntro.render(demo.context, elapsed);
        break;

        case Demo::Step::INTRO:
            if (!demo.screenIntro.render(demo.context, elapsed))
            {
                demo.screenIntro.exit(demo.context);
                demo.screenMain.enter(demo.context);
                demo.step = Demo::Step::MAIN;
            }
        break;

        case Demo::Step::MAIN:
            if (!demo.screenMain.render(demo.context, elapsed))
            {
                demo.screenMain.exit(demo.context);
                demo.screenIntro.enter(demo.context);
                demo.step = Demo::Step::INTRO;
            }
        break;
        }

        Petit2D::SetFrameBuffer(nullptr);
        Petit2D::SetViewport(0, 0, screen.width, screen.height);
        Petit2D::SetBlending(Petit2D::BlendMode::ALPHA);
        Petit2D::SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        Petit2D::Clear();

        Petit2D::Sprite::Use();
        Petit2D::Sprite::SetMatrix(glm::value_ptr(screen.matrix));
        Petit2D::Sprite::SetTexture(Petit2D::Texture::TextureUnit::UNIT_0);

        screen.sprite.a = 1.0f;
        screen.sprite.t = 1.0f;

        Petit2D::SetTexture(demo.context.frameBufferTexture, Petit2D::Texture::TextureUnit::UNIT_0);
        Petit2D::Sprite::Begin();
        Petit2D::Sprite::Add(screen.sprite);
        Petit2D::Sprite::End();
        Petit2D::Sprite::Render();

        screen.sprite.a = 0.5f;
        screen.sprite.t = scale;
        
        Petit2D::SetTexture(demo.context.scanLinesTexture, Petit2D::Texture::TextureUnit::UNIT_0);
        Petit2D::Sprite::Begin();
        Petit2D::Sprite::Add(screen.sprite);
        Petit2D::Sprite::End();
        Petit2D::Sprite::Render();

        SDL_GL_SwapWindow(sdl.window);
    }

    SDL_PauseAudioDevice(sdl.audioDevice, true);
    
    demo.context.unload();
    Petit2D::Destroy();

    SDL_ClearQueuedAudio(sdl.audioDevice);
    SDL_CloseAudioDevice(sdl.audioDevice);
    SDL_GL_DeleteContext(sdl.glContext);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
    
    romfsExit();

    return 0;
}

