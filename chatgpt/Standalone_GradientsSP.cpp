#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <thread>
#include <chrono>

// Stubbed M5Stack-related functions and classes
class M5Stub {
public:
    void begin() { std::cout << "M5.begin() called (stubbed).\n"; }
    void update() { std::cout << "M5.update() called (stubbed).\n"; }
    class LcdClass {
    public:
        void fillScreen(int) {}
        void setTextColor(int, int) {}
        void setTextSize(int) {}
        void setCursor(int, int) {}
        void println(const char* text) { std::cout << text << "\n"; }
        int color565(int r, int g, int b) { return (r << 16) | (g << 8) | b; }
        void drawPixel(int, int, int) {}
        void fillRect(int, int, int, int, int) {}
        int textWidth(const char*) { return 100; }
        void drawString(const char*, int, int, int) {}
    } Lcd;
    class BtnAClass {
    public:
        bool wasPressed() { return false; }
    } BtnA;
    class IMUClass {
    public:
        void Init() {}
        void getAccelData(float* x, float* y, float* z) { if (x) *x = 0.0f; if (y) *y = 0.0f; if (z) *z = 0.0f; }
    } IMU;
};
M5Stub M5;

// === BEGIN ORIGINAL CODE ===

int Ncols = 100;
uint16_t cols[100];
uint8_t reds[100]={49,50,51,52,54,55,57,60,62,66,69,73,78,82,86,91,95,100,106,111,117,123,129,135,141,146,152,157,163,168,172,178,184,190,196,202,208,213,218,222,225,229,232,236,240,243,247,250,252,254,255,255,255,255,255,255,255,254,254,254,254,254,254,254,254,254,254,254,254,253,253,252,252,251,251,250,249,248,246,244,242,240,238,235,233,230,227,224,220,215,211,206,202,197,192,187,182,176,171,165};
uint8_t greens[100]={54,61,68,74,81,87,94,100,106,112,118,123,129,135,141,147,153,158,164,169,174,179,184,188,193,198,202,207,211,214,218,221,225,228,231,233,236,238,240,242,244,245,247,248,250,251,252,253,254,255,254,251,249,246,243,240,236,233,229,226,222,217,213,208,203,198,192,187,181,176,170,164,158,151,144,137,130,123,117,110,104,98,92,85,79,72,66,60,54,48,44,39,33,28,23,17,11,6,2,0};
uint8_t blues[100]={149,152,155,159,162,165,168,171,174,177,180,183,186,189,192,195,198,201,204,207,210,212,215,217,220,223,225,227,230,232,234,236,238,240,242,245,246,248,248,248,248,246,243,238,233,227,221,213,205,196,189,184,179,174,170,165,160,155,150,146,141,136,131,126,121,116,111,106,102,98,95,91,88,85,82,79,76,73,71,68,65,62,59,56,53,50,47,44,42,39,37,36,35,34,34,35,35,36,37,38};

void isolines()
{
    for (int i = 0; i < Ncols; i += round(Ncols/10))
    {
      reds[i] = 0;
      blues[i] = 0;
      greens[i] = 0;

    }
}

void convertTo565()
{
    for (int i = 0; i < Ncols; i++) cols[i] = M5.Lcd.color565(reds[i], greens[i], blues[i]);
}

// Display size
#define scrX 320
#define scrY 240
#define topBar 20

// Terrain grid
int NcpX = 33;
int NcpY = 25;
float cpA = 1000.0;
uint32_t  rngNo;
int smoothPass = 5;
float field[33][25];
float Fx[33][25];
float Fy[33][25];
float minF = 99999;
float maxF = -999999;
float dF;
int aaaaa;
float bbbbbb;
long ccccc;

// Orbs
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float tx = 0.0F;
float ty = 0.0F;
float xs = 160.0F;
float ys = 120.0F;
float xv = 0.0F;
float yv = 0.0F;
int xp = 0;
int yp = 0;
long lastR = 0;
long lastU = 0;

// ENV
float sm = 0.0F;
float drag = 0.99;
float conR = 1.0;
float topoR = 2.0;
float CoR = 0.9; //Coeficient of resistution
int ballR = 5;
int halfWind = 12; // Sprite remove half of window
int vectL = 6;
int precision = 8;

TFT_eSprite bck = TFT_eSprite(&M5.Lcd);
TFT_eSprite scene = TFT_eSprite(&M5.Lcd);

int sprX = 6;
int sprY = 6;
int score = 0;
int targX;
int targY;
long lastCapt;

long gameLen = 90000;

// Game modifiers
bool teleport = false; // Screen wraparound
bool grippy = false; // More friction

Preferences preferences;
long counter;
long HS;

// Intro variables
float xm = 160;
float ym = 120;
int ncolor = 1000;
float nx, ny;
float scr, scg, scb;
float ncr, ncg, ncb;
int k = 0;

void setup() {
  M5.begin();
  M5.IMU.Init();

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(2);
  lastR = millis();
  lastU = millis();

  // Intro
  scr = random(100) / 100.0;
  scg = random(100) / 100.0;
  scb = random(100) / 100.0;
  ncr = random(100) / 100.0;
  ncg = random(100) / 100.0;
  ncb = random(100) / 100.0;
  nx = 1 + ((float)random(900))/100.0;
  ny = 1 + ((float)random(900))/100.0;
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Gradients 0.92");
  M5.Lcd.setCursor(10, 30);  // Position the cursor in the top left corner
  M5.Lcd.println("vlado83 2025");
  M5.Lcd.setCursor(10, 220);  // Position the cursor in the top left corner
  M5.Lcd.println("Press A to start");
   while (!M5.BtnA.wasPressed()) {
    for (float t = 0; t < 100 * PI; t += 0.01) {
      M5.update(); // Update button states
      if (M5.BtnA.wasPressed()) {
        break;
      }
      k++;
      float ccr = scr * k / ncolor + ncr * (1 - k / ncolor);
      float ccg = scg * k / ncolor + ncg * (1 - k / ncolor);
      float ccb = scb * k / ncolor + ncb * (1 - k / ncolor);
      if (k == ncolor) {
        scr = ncr;
        scg = ncg;
        scb = ncb;
        ncr = random(100) / 100.0;
        ncg = random(100) / 100.0;
        ncb = random(100) / 100.0;
        k = 0;
      }

      int indX = round(xm + cos(nx * t) * sin(t) * 80);
      int indY = round(ym + sin(ny * t) * sin(t) * 80);

      uint32_t color = M5.Lcd.color565(ccr * 255, ccg * 255, ccb * 255);
      if (indX >= 0 && indX < 320 && indY >= 0 && indY < 240) {
        M5.Lcd.drawPixel(indX, indY, color);
      }

      delay(5);  // Adjust the delay for desired animation speed
    }
  }

  bck.createSprite(scrX, scrY);
  bck.fillSprite(TFT_BLACK);
  rngNo = esp_random();
  randomSeed(rngNo);
  generateTerrain();
  generateGradients();

  // Copy sprite to scene
  scene.createSprite(scrX, scrY);
  scene.fillSprite(TFT_BLACK);
  for (int x = 0; x < scrX; x++)
  {
    for (int y = 0; y < scrY; y++)
    {
      scene.drawPixel(x, y, bck.readPixel(x, y));
    }
  }

  // Inital orb
  targX = random(ballR, scrX - ballR);
  targY = random(topBar + ballR, scrY - ballR);
  lastCapt = millis();

  // Persistent memory check
  preferences.begin("VGame", false);
  counter = preferences.getLong("counter", -1);
  if (counter == -1)
  {
    preferences.putLong("counter", 1);
    counter = 1;
  }
  HS = preferences.getLong("HS", -1);
  if (HS == -1)
  {
    preferences.putLong("HS", 0);
    HS = 0;
  }

}

void loop()
{
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  if (grippy)
  {
    drag = 0.92;
    topoR = 0.7;
  }
  else
  {
    drag = 0.99;
    topoR = 2.0;
  }
  tx = tx * sm -  accX * (1.0 - sm);
  ty = ty * sm +  accY * (1.0 - sm);
  if (tx > 1.0) tx = 1.0;
  if (tx < -1.0) tx = -1.0;
  if (ty > 1.0) ty = 1.0;
  if (ty < -1.0) ty = -1.0;

  int dt = (lastR - millis()) * 10;
  lastR = millis();
  int xi = floor(xs / (scrX / (NcpX - 1)));
  int yi = floor(ys / (scrY / (NcpY - 1)));

  xv = xv + conR * tx * float(dt) / 1000.0 - topoR * Fx[xi][yi] * float(dt) / 1000.0;
  yv = yv + conR * ty * float(dt) / 1000.0 - topoR * Fy[xi][yi] * float(dt) / 1000.0;
  xv = xv * drag;
  yv = yv * drag;

  xp = int(xs);
  yp = int(ys);
  xs = xs + xv * float(dt) / 1000.0;
  ys = ys + yv * float(dt) / 1000.0;

  if (!teleport)
  {
    if (xs < ballR) {
      xv = -CoR * xv;
      xs = ballR;
    }
    if (xs > (scrX - ballR)) {
      xv = -CoR * xv;
      xs = scrX - ballR;
    }
    if (ys < topBar + ballR) {
      yv = -CoR * yv;
      ys = topBar + ballR;
    }
    if (ys > (scrY - ballR ))
    {
      yv = -CoR * yv;
      ys = scrY - ballR;
    }
  }
  else
  {
    if (xs < ballR) {
      xs = scrX - ballR;
    }
    if (xs > (scrX - ballR)) {
      xs = ballR;
    }
    if (ys < topBar + ballR) {
      ys = scrY - ballR;

    }
    if (ys > (scrY - ballR ))
    {
      ys = topBar + ballR;
    }
  }

  // Generate new scene
  cleanUp(xp, yp);
  cleanUp(targX, targY);

  // Drawing player
  scene.fillTriangle(int(xs + tx * (ballR + vectL)), int(ys + ty * (ballR + vectL)), int(xs + ty * ballR), int(ys - tx * ballR), int( xs - ty * ballR), int(ys + tx * ballR), TFT_MAROON);
  uint16_t c = TFT_RED;
  if (teleport) c = TFT_YELLOW;
  if (grippy) c = TFT_BLUE;

  scene.fillEllipse(xs, ys, ballR, ballR, c);
  scene.drawEllipse(xs, ys, ballR, ballR, TFT_BLACK);

  // Drawing orb
  scene.fillEllipse(targX, targY, 6, 6, TFT_GREEN);
  scene.drawEllipse(targX, targY, 6, 6, TFT_BLACK);

  scene.fillRect(0, 0, scrX, topBar, TFT_BLACK);
  scene.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
  scene.setTextSize(2);
  // Score
  scene.setTextDatum(TL_DATUM);
  scene.drawNumber(score, 1, 1);
  // Timer
  scene.setTextDatum(TR_DATUM);
  float t  = (float(gameLen) - float(millis())) / 1000.0;
  scene.drawFloat(t, 2, scrX, 1);
  // HighScore
  scene.setTextDatum(TC_DATUM);
  scene.setTextColor(TFT_RED);
  scene.drawNumber(HS, scrX / 2, 1);
  scene.setTextColor(TFT_WHITE);
  scene.setTextDatum(TL_DATUM);
  scene.pushSprite(0, 0);

  // Capture
  if ((abs(xs - targX) < precision) && (abs(ys - targY) < precision))
  {
    int s = 1000 - (millis() - lastCapt) / 10;
    if (s < 100) s = 100;
    score += s;
    targX = random(ballR, scrX - ballR);
    targY = random(topBar + ballR, scrY - ballR);
    gameLen += 2000;

    lastCapt = millis();
  }

  // End of the game
  if (millis() > gameLen)
  {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextDatum(MC_DATUM);
    int tw = M5.Lcd.textWidth("GAME OVER");
    M5.Lcd.fillRect((scrX - tw - 40) / 2, 100, tw + 40, 40, TFT_BLACK);
    M5.Lcd.drawString("GAME OVER", scrX / 2, scrY / 2, 2);
    if (score > HS)
    {
      HS = score;
      preferences.putLong("HS", HS);
      M5.Lcd.drawString("New high score!", scrX / 2, scrY / 2 + 50, 2);
    }
    counter++;
    preferences.putLong("counter", counter);
    preferences.end();


    while (1)
    {;}
    // Stuck until reset
  }

 M5.update();
}

void generateTerrain()
{
  // Generation of rough terrain
  for (int i = 0; i < NcpX; i++)
  {
    for (int j = 0; j < NcpY; j++)
    {
      field[i][j] = float(random(-cpA, cpA + 1)) / 100.0;
      if ((i == 0) || (i == (NcpX - 1)) || (j == 0) || (j == (NcpY - 1))) field[i][j] = 0.0;
    }
  }
  // Terrain smoothing and limits
  for (int k = 0; k < (smoothPass - 1); k++)
  {
    minF = 999999.0;
    maxF = -999999.0;
    for (int i = 1; i < (NcpX - 1); i++)
    {
      for (int j = 1; j < (NcpY - 1); j++)
      {
        field[i][j] =  (field[i - 1][j - 1] + field[i][j - 1] + field[i + 1][j - 1] +
                        field[i - 1][j] + field[i][j] + field[i + 1][j] +
                        field[i - 1][j + 1] + field[i][j + 1] + field[i + 1][j + 1]) / 9.0;
      }
    }

    for (int i = 0; i < NcpX; i++)
    {
      for (int j = 0; j < NcpY; j++)
      {
        if (field[i][j] < minF) minF = field[i][j];
        if (field[i][j] > maxF) maxF = field[i][j];
      }
    }

    float dFmax = abs(maxF) / (Ncols / 2);
    float dFmin = abs(minF) / (Ncols / 2);
    Serial.print(minF);
    Serial.print(",");
    Serial.println(maxF);

    dF = max(dFmax, dFmin);

    isolines();
    convertTo565();

    for (int x = 0; x < scrX; x++)
    {
      for (int y = 0; y < scrY; y++)
      {
        int inx = floor(float(x) / (scrX / (NcpX - 1)));
        int iny = floor(float(y) / (scrY / (NcpY - 1)));
        float xm = x - float(scrX / (NcpX - 1)) * inx;
        float ym = y - float(scrY / (NcpY - 1)) * iny;
        float clX = float(scrX / (NcpX - 1));
        float clY = float(scrY / (NcpY - 1));

        float z = 0.01 * (field[inx][iny] * (clX - xm) * (clY - ym) + field[inx + 1][iny] * xm * (clY - ym) + field[inx][iny + 1] * (clX - xm) * ym + field[inx + 1][iny + 1] * xm * ym);
        int ncol = (Ncols / 2) + round(z / dF);
        bck.drawPixel(x, y, cols[ncol]);
      }
    }
    bck.pushSprite(0, 0);
  }
}

void generateGradients()
{
  for (int i = 0; i < (NcpX - 1); i++)
  {
    for (int j = 0; j < (NcpY - 1); j++)
    {
      Fx[i][j] = (field[i + 1][j] - field[i][j]) / 1.0;
      Fy[i][j] = (field[i][j + 1] - field[i][j]) / 1.0;
    }
  }
}

// Cleaning last frame
void cleanUp(int xp, int yp)
{
  for (int x = xp - halfWind; x <= (xp + halfWind); x++)
  {
    for (int y = yp - halfWind; y <= (yp + halfWind); y++)
    {
      scene.drawPixel(x, y, bck.readPixel(x, y));
    }
  }
}

int main() {
    setup();
    loop();
    return 0;
}

// === END ORIGINAL CODE ===
