#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Pixel Access",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED);

    // Create streaming texture (acts as pixel buffer)
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    int quit = 0;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            if (event.type == SDL_KEYDOWN) quit = 1;
        }

        // Lock texture for pixel writing
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);

        // Draw a red pixel at (100,100)
        Uint32* pixelArray = (Uint32*)pixels;
        pixelArray[100 * (pitch/4) + 100] = 0xFF0000FF;  // RGBA format

        SDL_UnlockTexture(texture);

        // Render texture to screen
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Read pixel back from renderer
        SDL_Rect rect = {100, 100, 1, 1};
        Uint32 pixel;
        SDL_RenderReadPixels(renderer, &rect,
            SDL_PIXELFORMAT_RGBA8888,
            &pixel, sizeof(Uint32));

        // Extract color components
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888),
            &r, &g, &b, &a);

        printf("Pixel at (100,100): R=%d, G=%d, B=%d, A=%d\n", r, g, b, a);
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
