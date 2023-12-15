#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!

int main( int argc, char* argv[] )
{
    const int WIDTH = 640;
    const int HEIGHT = 480;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Hello SDL", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    bool run = true;
    while (run) {
      SDL_Event ev;
      while(SDL_PollEvent(&ev)) {
        switch (ev.type) {
          case SDL_EVENT_QUIT:
            run = false;
            break;
        }
      }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}