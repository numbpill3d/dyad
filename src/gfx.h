// gfx.h — display, dithering and iridescence, shared look with 04 / CORTEX-01
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <math.h>
#include <string.h>

#define PIN_CS     4
#define PIN_MOSI   3
#define PIN_SCLK   5
#define PIN_DC     2
#define PIN_RST    1
#define PIN_BL     38   // active LOW
#define PIN_LED_DI 40
#define PIN_LED_CI 39
#define PIN_BTN    0
#ifndef DYAD_FLIP
#define DYAD_FLIP 0
#endif

static const int W = 160, H = 80;
static const int XOFF = 1, YOFF = 26;
static uint8_t  gray[H][W];
static uint8_t  hl[H][W];
static uint16_t fb[W * H];
static uint32_t frameNo = 0;

static uint32_t grs = 0x9E3779B9;
static inline uint32_t gxr() { grs ^= grs << 13; grs ^= grs >> 17; grs ^= grs << 5; return grs; }
static inline float frand() { return (gxr() >> 8) * (1.0f / 16777216.0f); }
#define xr gxr
static inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

// ---------------------------------------------------------------- display
static SPIClass tftSPI(FSPI);
static const SPISettings tftCfg(27000000, MSBFIRST, SPI_MODE0);

static void tftCmd(uint8_t c, const uint8_t* d = nullptr, size_t n = 0) {
  tftSPI.beginTransaction(tftCfg);
  digitalWrite(PIN_CS, LOW);
  digitalWrite(PIN_DC, LOW);
  tftSPI.write(c);
  if (n) { digitalWrite(PIN_DC, HIGH); tftSPI.writeBytes(d, n); }
  digitalWrite(PIN_CS, HIGH);
  tftSPI.endTransaction();
}

static void tftInit() {
  pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  tftSPI.begin(PIN_SCLK, -1, PIN_MOSI, -1);
  digitalWrite(PIN_RST, HIGH); delay(5);
  digitalWrite(PIN_RST, LOW);  delay(20);
  digitalWrite(PIN_RST, HIGH); delay(150);

  tftCmd(0x01); delay(150);                 // SWRESET
  tftCmd(0x11); delay(255);                 // SLPOUT
  { const uint8_t d[] = {0x01, 0x2C, 0x2D}; tftCmd(0xB1, d, 3); tftCmd(0xB2, d, 3); }
  { const uint8_t d[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}; tftCmd(0xB3, d, 6); }
  { const uint8_t d[] = {0x07}; tftCmd(0xB4, d, 1); }
  { const uint8_t d[] = {0xA2, 0x02, 0x84}; tftCmd(0xC0, d, 3); }
  { const uint8_t d[] = {0xC5}; tftCmd(0xC1, d, 1); }
  { const uint8_t d[] = {0x0A, 0x00}; tftCmd(0xC2, d, 2); }
  { const uint8_t d[] = {0x8A, 0x2A}; tftCmd(0xC3, d, 2); }
  { const uint8_t d[] = {0x8A, 0xEE}; tftCmd(0xC4, d, 2); }
  { const uint8_t d[] = {0x0E}; tftCmd(0xC5, d, 1); }
  tftCmd(0x21);                             // INVON (IPS panel)
  { // MADCTL: landscape (MV) + BGR
    const uint8_t d[] = {(uint8_t)((DYAD_FLIP ? 0x60 : 0xA0) | 0x08)};
    tftCmd(0x36, d, 1);
  }
  { const uint8_t d[] = {0x05}; tftCmd(0x3A, d, 1); }  // 16-bit color
  { const uint8_t d[] = {0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2d,0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10};
    tftCmd(0xE0, d, 16); }
  { const uint8_t d[] = {0x03,0x1d,0x07,0x06,0x2E,0x2C,0x29,0x2D,0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10};
    tftCmd(0xE1, d, 16); }
  tftCmd(0x13); delay(10);                  // NORON
  tftCmd(0x29); delay(100);                 // DISPON
}

static void tftPush() {
  const uint8_t ca[] = {0, XOFF, 0, (uint8_t)(XOFF + W - 1)};
  const uint8_t ra[] = {0, YOFF, 0, (uint8_t)(YOFF + H - 1)};
  tftCmd(0x2A, ca, 4);
  tftCmd(0x2B, ra, 4);
  tftCmd(0x2C, (const uint8_t*)fb, sizeof(fb));
}

static void backlight(uint8_t level) {  // 0..255, panel is active-low
  ledcWrite(PIN_BL, 255 - level);
}

// ---------------------------------------------------------------- APA102
static void apaByte(uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(PIN_LED_DI, (b >> i) & 1);
    digitalWrite(PIN_LED_CI, HIGH);
    digitalWrite(PIN_LED_CI, LOW);
  }
}
static void ledSet(uint8_t r, uint8_t g, uint8_t b, uint8_t br) {
  for (int i = 0; i < 4; i++) apaByte(0);
  apaByte(0xE0 | (br & 31));
  apaByte(b); apaByte(g); apaByte(r);
  for (int i = 0; i < 4; i++) apaByte(0xFF);
}

// ---------------------------------------------------------------- color
static inline uint16_t rgb565s(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  return (uint16_t)((c >> 8) | (c << 8));
}
static uint16_t ON_C, OFF_C;
static uint16_t RAINBOW[256];
static uint8_t  RB_R[256], RB_G[256], RB_B[256];

static void hsv(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
  h = fmodf(h, 1.0f) * 6.0f;
  int i = (int)h; float f = h - i;
  float p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
  float R, G, B;
  switch (i % 6) {
    case 0: R = v; G = t; B = p; break;
    case 1: R = q; G = v; B = p; break;
    case 2: R = p; G = v; B = t; break;
    case 3: R = p; G = q; B = v; break;
    case 4: R = t; G = p; B = v; break;
    default: R = v; G = p; B = q; break;
  }
  r = (uint8_t)(R * 255); g = (uint8_t)(G * 255); b = (uint8_t)(B * 255);
}

static void colorInit() {
  ON_C = rgb565s(236, 236, 226);  // faintly warm paper-white
  OFF_C = 0;
  for (int i = 0; i < 256; i++) {
    // oil-film iridescence: pastel hue wheel with a little value ripple
    float h = i / 256.0f;
    float v = 0.92f + 0.08f * sinf(i * 0.1472f);
    hsv(h, 0.62f, v, RB_R[i], RB_G[i], RB_B[i]);
    RAINBOW[i] = rgb565s(RB_R[i], RB_G[i], RB_B[i]);
  }
}

static const uint8_t BAYER[8][8] = {
  { 0,32, 8,40, 2,34,10,42}, {48,16,56,24,50,18,58,26},
  {12,44, 4,36,14,46, 6,38}, {60,28,52,20,62,30,54,22},
  { 3,35,11,43, 1,33, 9,41}, {51,19,59,27,49,17,57,25},
  {15,47, 7,39,13,45, 5,37}, {63,31,55,23,61,29,53,21}};

// ---------------------------------------------------------------- iridescence
enum { IRIS_NONE = 0, IRIS_FOCUS, IRIS_SWEEP };
static int irisMode = IRIS_NONE;
static uint32_t irisStart = 0, irisDur = 0, irisEnd = 0;

static void irisTrigger(int mode, uint32_t dur) {
  irisMode = mode; irisStart = frameNo; irisDur = dur;
  Serial.printf("[iris] %s\n", mode == IRIS_FOCUS ? "focus" : "sweep");
}
// learning milestone: maybe light up (rare, never back-to-back)
static void milestone(const char* what) {
  Serial.printf("[event] %s\n", what);
  if (irisMode == IRIS_NONE && frameNo - irisEnd > 900 && frand() < 0.35f)
    irisTrigger(IRIS_FOCUS, 54);
}
static void irisTick() {
  if (irisMode != IRIS_NONE) {
    if (frameNo - irisStart >= irisDur) { irisMode = IRIS_NONE; irisEnd = frameNo; }
  } else if (frameNo - irisEnd > 600 && xr() % 2700 == 0) {
    irisTrigger(IRIS_SWEEP, 48);  // pure chance, ~once every couple of minutes
  }
}

static int fadeIn = 256;  // 0..256 global fade used for lab transitions

static void compose() {
  const uint32_t t = frameNo - irisStart;
  int envThr = 0, sweepPos = -1000;
  if (irisMode == IRIS_FOCUS) {
    // dithered dissolve in and out of color
    int e = (int)min(t, irisDur - t);
    envThr = clampf(e / 10.0f, 0, 1) * 256;
  } else if (irisMode == IRIS_SWEEP) {
    sweepPos = (int)(t * (W + H + 60) / irisDur) - 30;
  }
  const int band = 30;
  for (int y = 0; y < H; y++) {
    const uint8_t* gr = gray[y];
    const uint8_t* hr = hl[y];
    uint16_t* out = fb + y * W;
    const uint8_t* by = BAYER[y & 7];
    for (int x = 0; x < W; x++) {
      int v = gr[x];
      if (fadeIn < 256) v = (v * fadeIn) >> 8;
      int thr = by[x & 7] * 4 + 2;
      if (v <= thr) { out[x] = OFF_C; continue; }
      uint16_t c = ON_C;
      if (irisMode == IRIS_FOCUS) {
        if (hr[x] && thr < envThr) c = RAINBOW[(x * 5 + y * 7 + t * 6) & 255];
      } else if (irisMode == IRIS_SWEEP) {
        int d = x + y - sweepPos;
        if (d >= 0 && d < band) {
          // ragged dithered edges on the band
          if ((d > 3 && d < band - 4) || thr < 128)
            c = RAINBOW[(d * 9 + y * 4 + t * 5) & 255];
        }
      }
      out[x] = c;
    }
  }
}

// ---------------------------------------------------------------- drawing
static inline void px(int x, int y, uint8_t v, bool h = false) {
  if ((unsigned)x < (unsigned)W && (unsigned)y < (unsigned)H) {
    gray[y][x] = v;
    if (h) hl[y][x] = 1;
  }
}
static inline void pxMax(int x, int y, uint8_t v) {
  if ((unsigned)x < (unsigned)W && (unsigned)y < (unsigned)H && gray[y][x] < v) gray[y][x] = v;
}
static void line(int x0, int y0, int x1, int y1, uint8_t v, bool h = false, bool dash = false) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, k = 0;
  for (;;) {
    if (!dash || ((k >> 1) & 1) == 0) px(x0, y0, v, h);
    k++;
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}
static void fillRect(int x, int y, int w, int h, uint8_t v, bool hh = false) {
  for (int j = y; j < y + h; j++)
    for (int i = x; i < x + w; i++) px(i, j, v, hh);
}
static void rect(int x, int y, int w, int h, uint8_t v, bool hh = false) {
  for (int i = x; i < x + w; i++) { px(i, y, v, hh); px(i, y + h - 1, v, hh); }
  for (int j = y; j < y + h; j++) { px(x, j, v, hh); px(x + w - 1, j, v, hh); }
}
static void fillCircle(int cx, int cy, int r, uint8_t v, bool hh = false) {
  for (int dy = -r; dy <= r; dy++)
    for (int dx = -r; dx <= r; dx++)
      if (dx * dx + dy * dy <= r * r + r) px(cx + dx, cy + dy, v, hh);
}
static void circle(int cx, int cy, int r, uint8_t v, bool hh = false) {
  for (int dy = -r; dy <= r; dy++)
    for (int dx = -r; dx <= r; dx++) {
      int d = dx * dx + dy * dy;
      if (d <= r * r + r && d >= r * r - r) px(cx + dx, cy + dy, v, hh);
    }
}

// 3x5 font, rows packed as octal digits (top row first, bit2 = left)
static const char FCH[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ .:/-+%#><=()_[]!?*'^|,;\"&";
static const uint16_t FGL[] = {
  075557, 026227, 071747, 071317, 055711, 074717, 074757, 071222, 075757, 075717,
  025755, 065656, 034443, 065556, 074647, 074644, 034553, 055755, 072227, 011152,
  055655, 044447, 057755, 065555, 025552, 065644, 025563, 065655, 034216, 072222,
  055557, 055552, 055775, 055255, 055222, 071247,
  0, 000002, 002020, 011244, 000700, 002720, 051245, 057575, 042124, 012421,
  007070, 024442, 021112, 000007, 064446, 031113, 022202, 071202, 005250, 022000,
  025000, 022222, 000024, 002024, 055000, 025257};

static uint16_t glyph(char c) {
  if (c >= 'a' && c <= 'z') c -= 32;
  const char* p = strchr(FCH, c);
  return (p && c) ? FGL[p - FCH] : 071202;
}
static void text(int x, int y, const char* s, uint8_t v = 255, bool hh = false, int sc = 1) {
  for (; *s; s++, x += 4 * sc) {
    uint16_t g = glyph(*s);
    for (int r = 0; r < 5; r++)
      for (int c = 0; c < 3; c++)
        if (g & (1 << ((4 - r) * 3 + (2 - c))))
          fillRect(x + c * sc, y + r * sc, sc, sc, v, hh);
  }
}
static void textf(int x, int y, uint8_t v, bool hh, const char* fmt, ...) {
  char b[40];
  va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
  text(x, y, b, v, hh);
}
static void sparkBox(int x, int y, int w, int h) {
  for (int i = x; i < x + w; i++) { if (!(i & 1)) { px(i, y, 150); px(i, y + h - 1, 150); } }
  for (int j = y; j < y + h; j++) { if (!(j & 1)) { px(x, j, 150); px(x + w - 1, j, 150); } }
}

