#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480

// Helper function to create RGB565 color
Uint16 RGB565(Uint8 r, Uint8 g, Uint8 b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Helper function to extract RGB from RGB565
void RGB565_to_RGB(Uint16 color, Uint8* r, Uint8* g, Uint8* b) {
    *r = (color & 0xF800) >> 8;
    *g = (color & 0x07E0) >> 3;
    *b = (color & 0x001F) << 3;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Pixel Access (RGB565)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED);

    // Create streaming texture with RGB565 format
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565,
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
        Uint16* pixelArray = (Uint16*)pixels;
        pixelArray[100 * (pitch/2) + 100] = RGB565(255, 255, 255);  // Red in RGB565

        SDL_UnlockTexture(texture);

        // Render texture to screen
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Read pixel back from renderer
        SDL_Rect rect = {101, 100, 1, 1};
        Uint16 pixel;
        SDL_RenderReadPixels(renderer, &rect,
            SDL_PIXELFORMAT_RGB565,
            &pixel, sizeof(Uint16));

        // Extract color components
        Uint8 r, g, b;
        RGB565_to_RGB(pixel, &r, &g, &b);

        printf("Pixel at (100,100): R=%d, G=%d, B=%d\n", r, g, b);
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
