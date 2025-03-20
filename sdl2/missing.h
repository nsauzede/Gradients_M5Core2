#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define NLOOPS 9999999
#define MILLIS 1

#define scrX 320
#define scrY 240

#define PI 3.14159265
#define MC_DATUM 0
#define TC_DATUM 0
#define TR_DATUM 0
#define TL_DATUM 0

#define TFT_BLACK 0
#define TFT_WHITE 1
#define TFT_RED 2
#define TFT_MAROON 3
#define TFT_YELLOW 4
#define TFT_BLUE 5
#define TFT_GREEN 6
#define TFT_TRANSPARENT 7

#define DBGC(n) n##_{public:const char*class__=#n;};class n:public n##_
//#define dbgprintf(...) do{printf("%s %p %s ", class__, this, __func__);printf(__VA_ARGS__);}while(0)
#define dbgprintf(...) do{printf("%s %s ", class__, __func__);printf(__VA_ARGS__);}while(0)
const char *class__ = "<no class>";

extern long gameLen;
extern void cleanUp(int xp, int yp);
extern void generateGradients();
extern void generateTerrain();
extern void loop();
extern void setup();

void randomSeed(int s) {
    dbgprintf("s=%d\n", s);
//        _exit(1);
    srand(s);
}
int esp_random() {
    int ret = rand();
    dbgprintf("returning %d\n", ret);
//        _exit(1);
    return ret;
}
int random(int a, int b = 0) {
//    dbgprintf("%d %d\n", a, b);
    int ret = 0;
    if (b != 0) {
//        _exit(1);
        int len = b - a;
        ret = (float)rand() / RAND_MAX * len + a;
    } else {
        ret = (float)rand() / RAND_MAX * a;
    }
//    dbgprintf("%d %d => %d\n", a, b, ret);
    return ret;
}
float max(float a, float b) {
    float ret = a > b ? a : b;
//    dbgprintf("%f %f => %f\n", a, b, ret);
//        _exit(1);
    return ret;
}

class DBGC(Preferences) {
public:
    void end() {
        dbgprintf("\n");
        _exit(1);
    }
    void begin(const char *, bool) {
        dbgprintf("\n");
    }
    void putLong(const char *, long) {
        dbgprintf("\n");
    }
    long getLong(const char *, long) {
        dbgprintf("\n");
        return 0;
    }
};

class DBGC(Serial_c) {
public:
    void print(const char *s) {
        dbgprintf("%s", s);
    }
    void print(float f) {
        dbgprintf("%f", f);
    }
    void println(float f) {
        dbgprintf("%f\n", f);
    }
};
Serial_c Serial;
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} sdl_t;
sdl_t sdl_, *sdl = &sdl_;
int sdlInit(sdl_t *sdl, int w, int h) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("SDL2 Minimal Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          w, h, 0);
    if (!window) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // Create streaming texture with RGB565 format
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        w, h);
/*
    // Set draw color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_RenderDrawPoint(renderer, w / 2 + 1, h / 2 - 1);
    SDL_RenderDrawPoint(renderer, w / 2 - 1, h / 2 + 1);
    SDL_RenderDrawPoint(renderer, w / 2 + 1, h / 2 + 1);

*/
    sdl->window = window;
    sdl->renderer = renderer;
    sdl->texture = texture;
    return 0;
}
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
    // Shift and mask the color components to fit into 5, 6, and 5 bits respectively
    uint16_t r5 = (r >> 3) & 0x1F; // Red: 5 bits
    uint16_t g6 = (g >> 2) & 0x3F; // Green: 6 bits
    uint16_t b5 = (b >> 3) & 0x1F; // Blue: 5 bits

    // Combine the components into a 16-bit RGB565 value
    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
//        dbgprintf("%x %x %x => %x\n", r, g, b, rgb565);
//        if (rgb565)_exit(1);
        return rgb565;
    }
void sdlSetPixel565(sdl_t *sdl, int x, int y, uint16_t col) {
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    pixelArray[y * (pitch/2) + x] = col;
    SDL_UnlockTexture(sdl->texture);
}
void sdlFillScreen565(sdl_t *sdl, uint16_t col) {
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    for (int y = 0; y < scrY; y++) {
        for (int x = 0; x < scrX; x++) {
            pixelArray[y * (pitch/2) + x] = col;
        }
    }
    SDL_UnlockTexture(sdl->texture);
}
void sdlPushSprite565(sdl_t *sdl, uint16_t *buffer) {
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    for (int y = 0; y < scrY; y++) {
        for (int x = 0; x < scrX; x++) {
            pixelArray[y * (pitch/2) + x] = buffer[y * (pitch/2) + x];
        }
    }
    SDL_UnlockTexture(sdl->texture);
}
uint16_t sdlReadPixel565(sdl_t *sdl, int x, int y) {
    uint16_t pixel = 0;
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    pixel = pixelArray[y * (pitch/2) + x];
    SDL_UnlockTexture(sdl->texture);
    return pixel;
}
void sdlUpdate(sdl_t *sdl) {
    // Render texture to screen
    SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, NULL);
    // Present the renderer
    SDL_RenderPresent(sdl->renderer);
}
void sdlQuit(sdl_t *sdl) {
   // Clean up
   SDL_DestroyTexture(sdl->texture);
   SDL_DestroyRenderer(sdl->renderer);
   SDL_DestroyWindow(sdl->window);
   SDL_Quit();
}
void sdlWait(sdl_t *sdl) {
   // Wait for a keypress
   SDL_Event event;
   int running = 1;
   while (running) {
       while (SDL_PollEvent(&event)) {
           if (event.type == SDL_KEYDOWN) {
               running = 0;
           }
       }
       SDL_Delay(16);
   }
}
void the_end() {
    dbgprintf("The end\n");
    sdlWait(sdl);
    sdlQuit(sdl);
}
void sdlDrawFastHLine565(int x1_, int y, int x2_, uint16_t col) {
    int x1, x2;
    if (x1_ > x2_) { x1 = x2_; x2 = x1_;
    } else { x1 = x1_; x2 = x2_; }
    if (x1 < 0) x1 = 0;
    if (x1 > (scrX - 1)) x1 = scrX - 1;
    if (x2 < 0) x2 = 0;
    if (x2 > (scrX - 1)) x2 = scrX - 1;
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    for (int x = x1; x < x2; x++) {
        pixelArray[y * (pitch/2) + x] = col;
    }
    SDL_UnlockTexture(sdl->texture);
}
void sdlDrawFastVLine565(int x, int y1_, int y2_, uint16_t col) {
    int y1, y2;
    if (y1_ > y2_) { y1 = y2_; y2 = y1_;
    } else { y1 = y1_; y2 = y2_; }
    if (y1 < 0) y1 = 0;
    if (y1 > (scrY - 1)) y1 = scrY - 1;
    if (y2 < 0) y2 = 0;
    if (y2 > (scrY - 1)) y2 = scrY - 1;
    void* pixels;
    int pitch;
    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);
    Uint16* pixelArray = (Uint16*)pixels;
    for (int y = y1; y < y2; y++) {
        pixelArray[y * (pitch/2) + x] = col;
    }
    SDL_UnlockTexture(sdl->texture);
}
void sdlDrawFastRect565(int x1, int y1, int x2, int y2, uint16_t col) {
    sdlDrawFastHLine565(x1, y1, x2, col);
    sdlDrawFastVLine565(x1, y1, y2, col);
    sdlDrawFastHLine565(x1, y2, x2, col);
    sdlDrawFastVLine565(x2, y1, y2, col);
}
void sdldemo() {
//    sdlInit(sdl, scrX, scrY);
    sdlFillScreen565(sdl, color565(25, 22, 28));
    sdlDrawFastRect565(20, 20, scrX - 20, scrY - 20, color565(0, 255, 255));
    sdlDrawFastHLine565(10, 10, scrX - 10, color565(0, 255, 0));
    sdlDrawFastVLine565(10, 10, scrY - 10, color565(255, 0, 0));
    sdlDrawFastHLine565(10, scrY - 10, scrX - 10, color565(0, 0, 255));
    sdlDrawFastVLine565(scrX - 10, 10, scrY - 10, color565(255, 255, 255));
    sdlSetPixel565(sdl, scrX / 2 - 1, scrY / 2 - 1, color565(255, 0, 0));
    sdlSetPixel565(sdl, scrX / 2 + 1, scrY / 2 - 1, color565(0, 255, 0));
    sdlSetPixel565(sdl, scrX / 2 - 1, scrY / 2 + 1, color565(0, 0, 255));
    sdlSetPixel565(sdl, scrX / 2 + 1, scrY / 2 + 1, color565(255, 255, 255));
    sdlUpdate(sdl);
    uint16_t pixel = 0;
    int x, y;
    x = scrX / 2 - 1; y = scrY / 2 - 1;
    pixel = sdlReadPixel565(sdl, x, y);
    printf("read pixel at %d %d => %04x\n", x, y, pixel);
    x = scrX / 2 + 1; y = scrY / 2 - 1;
    pixel = sdlReadPixel565(sdl, x, y);
    printf("read pixel at %d %d => %04x\n", x, y, pixel);
    x = scrX / 2 - 1; y = scrY / 2 + 1;
    pixel = sdlReadPixel565(sdl, x, y);
    printf("read pixel at %d %d => %04x\n", x, y, pixel);
    x = scrX / 2 + 1; y = scrY / 2 + 1;
    pixel = sdlReadPixel565(sdl, x, y);
    printf("read pixel at %d %d => %04x\n", x, y, pixel);
    x = scrX / 2; y = scrY / 2;
    pixel = sdlReadPixel565(sdl, x, y);
    printf("read pixel at %d %d => %04x\n", x, y, pixel);
    sdlWait(sdl);
    sdlQuit(sdl);
}
//static long millis_ = 0;
void delay(int d) {
//    dbgprintf("d=%d\n", d);
//        _exit(1);
//    millis_ += d;
    SDL_Delay(d);
}
long millis() {
#if 0
    dbgprintf("millis=%ld (gameLen=%ld)\n", millis_, gameLen);
    if (millis_ > gameLen) {
        dbgprintf("Game over.\n");
        _exit(1);
    }
    millis_ += MILLIS;
    return millis_;
#else
    return SDL_GetTicks();
#endif
}
// Function to extract RGB components from RGB565
void rgb565_to_rgb(uint16_t rgb565, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = ((rgb565 >> 11) & 0x1F) << 3;
    *g = ((rgb565 >> 5) & 0x3F) << 2;
    *b = (rgb565 & 0x1F) << 3;
}
typedef struct {
    bool status;
    int k;
} kp_t;
void sdlKeypressed(sdl_t *sdl, int nkp, kp_t *kp) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            for (int i = 0; i < nkp; i++) {
                if (event.key.keysym.sym == kp[i].k) {
                    kp[i].status = true;
                }
            }
        } else if (event.type == SDL_KEYUP) {
            for (int i = 0; i < nkp; i++) {
                if (event.key.keysym.sym == kp[i].k) {
                    kp[i].status = false;
                }
            }
        }
    }
}
class DBGC(Btn_c) {
public:
    bool wasPressed() {
        static kp_t kp[1] = {{false, SDLK_a}};
        sdlKeypressed(sdl, sizeof(kp)/sizeof(kp[0]), kp);
        bool status = kp[0].status;
        if (status)
            dbgprintf("this=%p key=%d\n", this, kp[0].k);
        return status | true;
    }
};
bool refresh_ = false;
class DBGC(IMU_c) {
public:
    void Init() {
        dbgprintf("\n");
    }
    void getAccelData(float *a, float *b, float *c) {
        static float x = 0, y = 0, z = 0;
        static float last_x = 0, last_y = 0, last_z = 0;
        float dx = x - last_x;
        float dy = y - last_y;
        float dz = z - last_z;
        float eps = 0.9;
        if (last_x >= (x + eps)) last_x -= eps;
        if (last_x <= (x - eps)) last_x += eps;
        if (last_y >= (y + eps)) last_y -= eps;
        if (last_y <= (y - eps)) last_y += eps;
        if (last_z >= (z + eps)) last_z -= eps;
        if (last_z <= (z - eps)) last_z += eps;
        static long last_t = 0;
        long t = millis();
        float dt = (float)t - last_t;
        last_t = t;
        float vx = dx / dt, vy = dy / dt, vz = dz / dt;
        float ax = vx / dt, ay = vy / dt, az = vz / dt;
//        dbgprintf("%f %f %f V: %f %f %f A: %f %f %f\n", x, y, z, vx, vy, vz, ax, ay, az);
        static kp_t kp[] = {
            {false, SDLK_ESCAPE},
            {false, SDLK_RIGHT},{false, SDLK_LEFT},
            {false, SDLK_UP},{false, SDLK_DOWN},
            {false, SDLK_KP_1},{false, SDLK_KP_2},
        };
        sdlKeypressed(sdl, sizeof(kp)/sizeof(kp[0]), kp);
        float d = 10.0;
        int k = 0;
        static bool refresh = true;
        if (kp[k++].status) { _exit(1); }
        if (kp[k++].status) { x -= d; refresh = true; }
        if (kp[k++].status) { x += d; refresh = true; }
        if (kp[k++].status) { y -= d; refresh = true; }
        if (kp[k++].status) { y += d; refresh = true; }
        if (kp[k++].status) { z -= d; refresh = true; }
        if (kp[k++].status) { z += d; refresh = true; }
        refresh_ = refresh;
        *a = ax; *b = ay; *c = az;
//        if (refresh)dbgprintf("%f %f %f\n", *a, *b, *c);
        refresh = false;
    }
};
class DBGC(LCD_c) {
    int cx, cy;
    int cf, cb;
    int ts;
public:
    LCD_c():cx(0),cy(0),cf(0),cb(0),ts(0) {
        dbgprintf("w=%d h=%d\n", scrX, scrY);
        sdlInit(sdl, scrX, scrY);
    }
    void update() {
//        dbgprintf("\n");
        sdlUpdate(sdl);
    }
    void fillScreen(uint16_t col) {
//        dbgprintf("col=%x\n", col);
//        sdlFillScreen565(sdl, col);
    }
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return ::color565(r, g, b);
    }
    void drawPixel(int x, int y, uint16_t col) {
        sdlSetPixel565(sdl, x, y, col);
    }
    void drawString(const char *s, int, int, int) {
        dbgprintf("str=%s\n", s);
    }
    void drawEllipse(int a, int b, int c, int d, int e) {
//        dbgprintf("%d %d %d %d %d\n", a, b, c, d, e);
    }
    void drawNumber(int a, int b, int c) {
//        dbgprintf("%d %d %d\n", a, b, c);
    }
    void drawFloat(float a, int b, int c, int d) {
//        dbgprintf("%f %d %d %d\n", a, b, c, d);
    }
    void setTextColor(uint16_t f, uint16_t b) {
        //dbgprintf("%d %d\n", f, b);
        cf = f;
        cb = b;
    }
    void setTextSize(int a) {
        //dbgprintf("%d\n", a);
        ts = a;
    }
    void setCursor(int a, int b) {
        //dbgprintf("%d %d\n", a, b);
        cx = a;
        cy = b;
    }
    void println(const char *s) {
        printf("color:%d,%d size:%d cursor:%d,%d str=%s\n", cf, cb, ts, cx, cy, s);
    }
    void setTextDatum(int a) {
//        dbgprintf("%d\n", a);
    }
    int textWidth(const char *s) {
//        dbgprintf("str=%s\n", s);
        return 0;
    }
    void createSprite(int a, int b) {
        dbgprintf("%d %d\n", a, b);
//        sdlUpdate(sdl);
    }
    void pushSprite(int a, int b) {
        dbgprintf("%d %d\n", a, b);
//        sdlUpdate(sdl);
    }
    void fillSprite(int a) {
        dbgprintf("%d\n", a);
//        sdlUpdate(sdl);
    }
    void fillRect(int a, int b, int c, int d, int e) {
        dbgprintf("%d %d %d %d %d\n", a, b, c, d, e);
//        sdlUpdate(sdl);
        _exit(1);
    }
};
class DBGC(TFT_eSprite) {
    LCD_c *lcd;
    uint16_t *buffer;
public:
    TFT_eSprite(LCD_c *lcd_):lcd(lcd_),buffer(0) {
        dbgprintf("this=%p lcd=%p\n", this, lcd);
        buffer = (uint16_t *)calloc(scrX * scrY, sizeof(uint16_t));
    }
    ~TFT_eSprite() {
        free(buffer);
    }
    uint16_t readPixel(int x, int y) {
        return buffer[y * scrX + x];
    }
    void drawPixel(int x, int y, uint16_t c) {
//        dbgprintf("this=%p %d %d %d\n", this, x, y, c);
        buffer[y * scrX + x] = c;
    }
void drawFastHLine(int x1_, int y, int x2_, uint16_t col) {
    int x1, x2;
    if (x1_ > x2_) { x1 = x2_; x2 = x1_;
    } else { x1 = x1_; x2 = x2_; }
    if (x1 < 0) x1 = 0;
    if (x1 > (scrX - 1)) x1 = scrX - 1;
    if (x2 < 0) x2 = 0;
    if (x2 > (scrX - 1)) x2 = scrX - 1;
    for (int x = x1; x < x2; x++) {
        buffer[y * scrX + x] = col;
    }
}
void drawFastVLine(int x, int y1_, int y2_, uint16_t col) {
    int y1, y2;
    if (y1_ > y2_) { y1 = y2_; y2 = y1_;
    } else { y1 = y1_; y2 = y2_; }
    if (y1 < 0) y1 = 0;
    if (y1 > (scrY - 1)) y1 = scrY - 1;
    if (y2 < 0) y2 = 0;
    if (y2 > (scrY - 1)) y2 = scrY - 1;
    for (int y = y1; y < y2; y++) {
        buffer[y * scrX + x] = col;
    }
}
void drawFastRect(int x1, int y1, int x2, int y2, uint16_t col) {
    drawFastHLine(x1, y1, x2, col);
    drawFastVLine(x1, y1, y2, col);
    drawFastHLine(x1, y2, x2, col);
    drawFastVLine(x2, y1, y2, col);
}
    void drawEllipse(int a, int b, int c, int d, int e) {
//        dbgprintf("this=%p %d %d %d %d %d\n", this, a, b, c, d, e);
        drawFastRect(a, b, a + c, b + d, e);
//        _exit(1);
    }
    void fillTriangle(int a, int b, int c, int d, int e, int f, int g) {
//        dbgprintf("this=%p %d %d %d %d %d\n", this, a, b, c, d, e);
        g = color565(255, 255, 255);
        drawPixel(a, b, g);
        drawPixel(c, d, g);
        drawPixel(e, f, g);
//        _exit(1);
    }
    void drawNumber(int a, int b, int c) {
        lcd->drawNumber(a, b, c);
    }
    void drawFloat(float a, int b, int c, int d) {
        lcd->drawFloat(a, b, c, d);
    }
    void setTextDatum(int a) {
        lcd->setTextDatum(a);
    }
    void setTextColor(int f, int b = 0) {
        lcd->setTextColor(f, b);
    }
    void setTextSize(int s) {
        lcd->setTextSize(s);
    }
    void fillEllipse(float a, float b, int c, int d, uint16_t e) {
//        dbgprintf("this=%p %f %f %d %d %d\n", this, a, b, c, d, e);
//        drawFastRect(a, b, a + c, a + d, e);
//        _exit(1);
    }
    void fillRect(int a, int b, int c, int d, int e) {
//        dbgprintf("this=%p %d %d %d %d %d\n", this, a, b, c, d, e);
        //lcd->fillRect(a, b, c, d, e);
        drawFastRect(a, b, a + c, b + d, e);
//        _exit(1);
    }
    void fillSprite(int a) {
        dbgprintf("this=%p %d\n", this, a);
//        lcd->fillSprite(a);
//        _exit(1);
    }
    void pushSprite(int a, int b) {
//        dbgprintf("this=%p %d %d\n", this, a, b);
//        lcd->pushSprite(a, b);
        sdlPushSprite565(sdl, buffer);
//        sdlUpdate(sdl);
        if (a + b) {
        dbgprintf("this=%p %d %d\n", this, a, b);
        _exit(1);
        }
    }
    void createSprite(int a, int b) {
        dbgprintf("this=%p %d %d\n", this, a, b);
//        lcd->createSprite(a, b);
//        _exit(1);
    }
};

class DBGC(M5) {
public:
    LCD_c Lcd;
    IMU_c IMU;
    Btn_c BtnA;
    void begin() {
        dbgprintf("\n");
        //_exit(1);
    }
    void update() {
#if 1
        Lcd.update();
#else
        dbgprintf("\n");
        _exit(1);
#endif
    }
};
M5 M5;

int main() {
#ifndef DEMO
    setup();
    for (int i = 0; i < NLOOPS; i++) {
//        dbgprintf("loop %d\n", i);
        SDL_Delay(16);
        loop();
    }
    the_end();
#else
    sdldemo();
#endif
    return 0;
}
