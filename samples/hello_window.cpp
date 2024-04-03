#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "spdlog/spdlog.h"

// Note: your main function __must__ take this form, otherwise on nonstandard platforms (iOS, etc), your app will not
// launch.
int main(int argc, char* argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("Failed to init, error: {}", SDL_GetError());
        exit(1);
    }

    // create a window
    SDL_Window* window = SDL_CreateWindow("Ash", 1920, 1080, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        spdlog::error("Failed to create window, error: {}", SDL_GetError());
        exit(1);
    }

    spdlog::info("Application started successfully!");

    bool close_requested = false;
    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                close_requested = true;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
