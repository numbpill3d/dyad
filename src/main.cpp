// 05 / DYAD — each dongle holds an artificial personality with persistent memory.
// Two dongles find each other over ESP-NOW and keep an ongoing fictional relationship.
// Inspect / steer them over USB serial (see tools/dyad.py).
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <LittleFS.h>
#include "gfx.h"
#include "dyad_engine.h"

using namespace dyad;

static Engine E;
static uint32_t tickMs = 1000;       // 1 lived second per real second (speed command changes it)
static int view = 0;                 // 0 talk, 1 mind, 2 world, 3 memory
static const int NVIEWS = 4;
static int lastMilestones = 0;
static float ledFlash = 0;
static uint32_t sentOk = 0, sentFail = 0, heard = 0;
static bool fsOk = false;

// ------------------------------------------------------------------ radio queue
// ESP-NOW callbacks run in the WiFi task: copy only, parse in loop()
static const int QN = 8;
static char rq[QN][252];
static volatile int qHead = 0, qTail = 0;
static portMUX_TYPE qMux = portMUX_INITIALIZER_UNLOCKED;
static const uint8_t BCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void onRecv(const esp_now_recv_info_t*, const uint8_t* data, int len) {
  if (len <= 0 || len > 250) return;
  portENTER_CRITICAL_ISR(&qMux);
  int n = (qHead + 1) % QN;
  if (n != qTail) { memcpy(rq[qHead], data, len); rq[qHead][len] = 0; qHead = n; }
  portEXIT_CRITICAL_ISR(&qMux);
}
static void onSent(const esp_now_send_info_t*, esp_now_send_status_t st) {
  if (st == ESP_NOW_SEND_SUCCESS) sentOk++; else sentFail++;
}
static bool popRadio(char* out) {
  bool got = false;
  portENTER_CRITICAL(&qMux);
  if (qTail != qHead) { strcpy(out, rq[qTail]); qTail = (qTail + 1) % QN; got = true; }
  portEXIT_CRITICAL(&qMux);
  return got;
}

// ------------------------------------------------------------------ host hooks
static void logAppend(const char* line) {
  if (!fsOk) return;
  File f = LittleFS.open("/log.txt", "a");
  if (!f) return;
  f.printf("%lu %s\n", (unsigned long)E.age, line);
  size_t sz = f.size();
  f.close();
  if (sz > 48 * 1024) { LittleFS.remove("/log.old"); LittleFS.rename("/log.txt", "/log.old"); }
}
static void hostOut(void*, const char* line) {
  Serial.println(line);
  if (!strncmp(line, "TX ", 3) || !strncmp(line, "RX ", 3) || !strncmp(line, "EV ", 3)) logAppend(line);
  if (!strncmp(line, "TX ", 3) || !strncmp(line, "RX ", 3)) { ledFlash = 1; view = 0; }
  if (!strncmp(line, "RX ", 3)) heard++;
}
static void hostSend(void*, const char* wire) {
  esp_now_send(BCAST, (const uint8_t*)wire, strlen(wire));
}

// ------------------------------------------------------------------ persistence
static void fileLine(void* f, const char* line) { ((File*)f)->println(line); }
static void saveMind() {
  if (!fsOk) return;
  File f = LittleFS.open("/mind.tmp", "w");
  if (!f) return;
  E.save(fileLine, &f);
  f.close();
  LittleFS.remove("/mind.txt");
  LittleFS.rename("/mind.tmp", "/mind.txt");
  E.dirty = false;
}
static bool loadMind() {
  if (!fsOk || !LittleFS.exists("/mind.txt")) return false;
  File f = LittleFS.open("/mind.txt", "r");
  if (!f) return false;
  E.create(1); E.beginLoad();
  char line[300];
  while (f.available()) {
    size_t n = f.readBytesUntil('\n', line, sizeof line - 1);
    line[n] = 0;
    if (n && line[n - 1] == '\r') line[n - 1] = 0;
    E.loadLine(line);
  }
  f.close();
  return E.name[0] != 0;
}
static void printLog(int n) {
  if (!fsOk || !LittleFS.exists("/log.txt")) { Serial.println("OK log empty"); return; }
  File f = LittleFS.open("/log.txt", "r");
  // count lines, then print the last n
  int total = 0;
  while (f.available()) if (f.read() == '\n') total++;
  f.seek(0);
  int skip = total - n, ln = 0;
  char line[300];
  while (f.available()) {
    size_t k = f.readBytesUntil('\n', line, sizeof line - 1); line[k] = 0;
    if (ln++ >= skip) { Serial.print("L "); Serial.println(line); }
  }
  f.close();
  Serial.println("OK end of log");
}

// ------------------------------------------------------------------ drawing
static int wrap(const char* s, int maxc, char lines[][34], int maxLines) {
  int n = 0; const char* p = s;
  while (*p && n < maxLines) {
    while (*p == ' ') p++;
    int len = strlen(p);
    if (len <= maxc) { strncpy(lines[n], p, 33); lines[n][33] = 0; n++; break; }
    int cut = maxc;
    while (cut > 0 && p[cut] != ' ') cut--;
    if (cut == 0) cut = maxc;
    memcpy(lines[n], p, cut); lines[n][cut] = 0; n++;
    p += cut;
  }
  return n;
}
static void bar(int x, int y, int w, float v, uint8_t val, bool h = false) {
  for (int i = 0; i < w; i += 2) px(x + i, y + 2, 70);
  int f = (int)(clampf(v, 0, 1) * w);
  fillRect(x, y, f, 4, val, h);
}

static void drawFace(int cx, int cy) {
  uint8_t lk = E.look;
  bool speaking = E.last.mine && E.age - E.last.at < 3 && (frameNo & 4);
  int r = 13;
  // head
  if (lk & 16) { fillRect(cx - r, cy - r + 2, 2 * r + 1, 2 * r - 1, (uint8_t)(30 + E.aro * 80)); rect(cx - r, cy - r + 2, 2 * r + 1, 2 * r - 1, 255, true); }
  else { fillCircle(cx, cy, r, (uint8_t)(30 + E.aro * 80)); circle(cx, cy, r, 255, true); }
  if (lk & 4) { line(cx, cy - r, cx + 3, cy - r - 6, 220, true); fillCircle(cx + 3, cy - r - 7, 1, 255, true); }
  if (lk & 8) { fillRect(cx - r - 3, cy - 3, 3, 6, 200); fillRect(cx + r + 1, cy - 3, 3, 6, 200); }
  // eyes
  bool closed = (frameNo % 131) < 4 || (E.light() == 3 && E.aro < 0.3f);
  for (int s = -1; s <= 1; s += 2) {
    int ex = cx + s * 5, ey = cy - 3;
    fillRect(ex - 2, ey - 2, 5, 5, 0);
    if (closed) { line(ex - 2, ey, ex + 2, ey, 255); continue; }
    switch (lk & 3) {
      case 0: fillRect(ex - 1, ey - 1, 2, 2, 255); break;
      case 1: circle(ex, ey, 2, 255); break;
      case 2: fillRect(ex - 2, ey - 1, 5, 3, 255); px(ex, ey, 0); break;
      default: line(ex - 2, ey + 1, ex, ey - 1, 255); line(ex, ey - 1, ex + 2, ey + 1, 255); break;
    }
  }
  // mouth: valence bends it
  if (speaking) { fillRect(cx - 3, cy + 5, 7, 3, 0); rect(cx - 3, cy + 5, 7, 3, 255); }
  else for (int x = -5; x <= 5; x++) {
    float d = x / 5.0f;
    int y = cy + 6 + (int)roundf(E.val * 2.6f * (1 - d * d)) - (int)roundf(E.val * 1.2f);
    px(cx + x, y, 255);
  }
  // tension: a crease
  if (E.rel.tension > 0.35f) { line(cx - 7, cy - 8, cx - 3, cy - 7, 200); line(cx + 7, cy - 8, cx + 3, cy - 7, 200); }
}

static void drawTalk() {
  drawFace(19, 27);
  char w[16]; snprintf(w, sizeof w, "%s", WEATHER_NAME[E.env.weather]);
  text(2, 47, w, 220);
  text(2, 54, LIGHT_NAME[E.light()], 160);
  // speech
  if (E.last.text[0]) {
    char hdr[40];
    if (E.last.mine) snprintf(hdr, sizeof hdr, "%s > %s", E.last.who, INTENT_NAME[E.last.intent]);
    else snprintf(hdr, sizeof hdr, "%s < %s", E.last.who, INTENT_NAME[E.last.intent]);
    text(40, 11, hdr, E.last.mine ? 150 : 255);
    for (int x = 40; x < 158; x += 2) px(x, 17, 90);
    static uint32_t shownAt = 0; static char shownText[112] = "";
    if (strcmp(shownText, E.last.text)) { strcpy(shownText, E.last.text); shownAt = frameNo; }
    int reveal = (int)(frameNo - shownAt) * 2;  // typewriter
    char vis[112]; strncpy(vis, E.last.text, sizeof vis); vis[sizeof vis - 1] = 0;
    if (reveal < (int)strlen(vis)) vis[reveal] = 0;
    char lines[6][34];
    int n = wrap(vis, 29, lines, 6);
    for (int i = 0; i < n; i++) text(40, 20 + i * 7, lines[i], 255, !E.last.mine);
  } else {
    text(40, 20, E.rel.peer ? "..." : "LISTENING FOR", 200);
    if (!E.rel.peer) text(40, 27, "ANOTHER VOICE", 200);
  }
  // bond strip
  if (E.rel.peer) {
    text(2, 66, "AFF", 180); bar(16, 66, 30, (E.rel.aff + 1) / 2, 255, true);
    text(52, 66, "TRU", 180); bar(66, 66, 30, E.rel.trust, 255);
    text(102, 66, "TEN", 180); bar(116, 66, 30, E.rel.tension, 200);
    char st[24]; snprintf(st, sizeof st, "%s", STAGE_NAME[E.rel.stage]);
    text(2, 73, st, 255, E.rel.stage == S_BONDED);
    char with[32]; snprintf(with, sizeof with, "WITH %s", E.rel.peerName[0] ? E.rel.peerName : "?");
    text(W - (int)strlen(with) * 4, 73, with, 160);
  } else {
    text(2, 73, "ALONE", 160);
  }
}

static void drawMind() {
  for (int i = 0; i < T_COUNT; i++) {
    text(2, 11 + i * 7, TRAIT_NAME[i], 200);
    bar(46, 11 + i * 7, 30, E.trait[i], 255);
  }
  textf(2, 47, 255, false, "VOICE %s", VOICE_NAME[E.voice]);
  textf(2, 54, 200, false, "MOOD %+.2f", E.val);
  textf(2, 61, 200, false, "%s", E.feelingWord());
  int x = 84;
  text(x, 11, "AFFECTION", 200); bar(x, 17, 72, (E.rel.aff + 1) / 2, 255, true);
  text(x, 24, "TRUST", 200); bar(x, 30, 72, E.rel.trust, 255);
  text(x, 37, "FAMILIARITY", 200); bar(x, 43, 72, E.rel.fam, 255);
  text(x, 50, "TENSION", 200); bar(x, 56, 72, E.rel.tension, 200);
  textf(x, 64, 255, false, "%lu MSGS", (unsigned long)E.rel.exchanged);
  textf(2, 73, 150, false, "FEARS %s", E.fears);
}

static void drawWorld() {
  char lines[3][34];
  int n = wrap(E.env.place, 38, lines, 2);
  for (int i = 0; i < n; i++) text(2, 11 + i * 7, lines[i], 255, true);
  textf(2, 26, 200, false, "%s / %s%s", WEATHER_PHRASE[E.env.weather], LIGHT_NAME[E.light()], E.env.lightPin ? " *" : "");
  for (int x = 2; x < 158; x += 2) px(x, 33, 90);
  for (int i = 0; i < E.env.nobj; i++) {
    char o[40]; snprintf(o, sizeof o, "- %s", E.env.obj[i]);
    text(2, 36 + i * 6, o, 230);
  }
  if (!E.env.nobj) text(2, 36, "NOTHING HERE", 120);
  if (E.rel.peerPlace[0]) { char p[44]; snprintf(p, sizeof p, "THEM: %s", E.rel.peerPlace); text(2, 74, p, 140); }
}

static void drawMemory() {
  text(2, 11, "MEMORIES", 255);
  textf(60, 11, 150, false, "%d KEPT  %d FACTS", E.nmem, E.nfacts);
  // page through memories, newest first
  int per = 5, pages = (E.nmem + per - 1) / per;
  int page = pages ? (int)((frameNo / 150) % pages) : 0;
  for (int k = 0; k < per; k++) {
    int i = E.nmem - 1 - (page * per + k);
    if (i < 0) break;
    const Memory& m = E.mem[i];
    char s[48]; snprintf(s, sizeof s, "%s %s", MEM_NAME[m.kind], m.arg);
    s[38] = 0;
    text(2, 19 + k * 7, s, m.imp > 60 ? 255 : 170, m.imp > 70);
  }
  if (pages > 1) textf(130, 73, 120, false, "%d/%d", page + 1, pages);
}

static void drawHeader() {
  text(1, 1, "05/DYAD", 255, true);
  char t[32]; snprintf(t, sizeof t, "%s D%lu", E.name, (unsigned long)E.day());
  text(W - (int)strlen(t) * 4, 1, t, 220);
  for (int x = 0; x < W; x += 2) px(x, 8, 140);
}

// ------------------------------------------------------------------ input
static bool btnDown = false, longFired = false;
static uint32_t btnT = 0;
static void pollButton() {
  bool d = digitalRead(PIN_BTN) == LOW;
  uint32_t now = millis();
  if (d && !btnDown) { btnDown = true; btnT = now; longFired = false; }
  else if (d && btnDown && !longFired && now - btnT > 700) {
    longFired = true; E.command("poke"); fadeIn = 0;
  } else if (!d && btnDown) {
    btnDown = false;
    if (!longFired && now - btnT > 30) { view = (view + 1) % NVIEWS; fadeIn = 0; }
  }
}

static char cmdBuf[300]; static int cmdLen = 0;
static void handleLine(const char* line) {
  if (!strncmp(line, "log", 3)) { int n = atoi(line + 3); printLog(n > 0 ? n : 30); return; }
  if (!strcmp(line, "fs")) {
    Serial.printf("OK littlefs %s used %u / %u bytes\n", fsOk ? "mounted" : "FAILED", fsOk ? (unsigned)LittleFS.usedBytes() : 0, fsOk ? (unsigned)LittleFS.totalBytes() : 0);
    return;
  }
  if (!strcmp(line, "radio")) { Serial.printf("OK radio sent ok %lu fail %lu heard %lu\n", (unsigned long)sentOk, (unsigned long)sentFail, (unsigned long)heard); return; }
  if (!strncmp(line, "speed", 5)) { int s = atoi(line + 5); if (s < 1) s = 1; if (s > 50) s = 50; tickMs = 1000 / s; Serial.printf("OK speed %d\n", s); return; }
  if (!strcmp(line, "save")) { saveMind(); Serial.println("OK saved"); return; }
  if (!strcmp(line, "wipe yes")) { E.command(line); if (fsOk) LittleFS.remove("/log.txt"); saveMind(); return; }
  if (!strcmp(line, "help")) { E.command("help"); Serial.println("OK   log [n] | fs | radio | save | speed <n>"); return; }
  if (!E.command(line)) Serial.println("ERR unknown command (try help)");
}
static void pollSerial() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdLen) { cmdBuf[cmdLen] = 0; handleLine(cmdBuf); cmdLen = 0; }
    } else if (cmdLen < (int)sizeof cmdBuf - 1) cmdBuf[cmdLen++] = (char)c;
  }
}

static void updateLed() {
  if (frameNo & 1) return;
  if (irisMode != IRIS_NONE) {
    uint8_t i = (uint8_t)(frameNo * 7);
    ledSet(RB_R[i] / 3, RB_G[i] / 3, RB_B[i] / 3, 2);
  } else {
    float breathe = 0.5f + 0.5f * sinf(frameNo * 0.05f);
    uint8_t w = (uint8_t)(3 + breathe * (6 + E.aro * 10) + ledFlash * 70);
    ledSet(w, w, w, 1);
  }
  ledFlash *= 0.85f;
}

// ------------------------------------------------------------------ main
void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);  // never stall when nobody is listening on USB
  grs ^= esp_random(); if (!grs) grs = 1;
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED_DI, OUTPUT); pinMode(PIN_LED_CI, OUTPUT);
  ledSet(0, 0, 0, 0);
  ledcAttach(PIN_BL, 20000, 8);
  backlight(0);
  colorInit();
  tftInit();
  memset(fb, 0, sizeof fb); tftPush();
  backlight(220);

  fsOk = LittleFS.begin(true);
  E.host = {nullptr, hostOut, hostSend};
  uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_WIFI_STA);
  uint32_t macSeed = ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
  if (loadMind()) {
    E.rng ^= esp_random(); if (!E.rng) E.rng = 1;
    Serial.printf("EV %s woke up (day %lu, %d memories)\n", E.name, (unsigned long)E.day(), E.nmem);
  } else {
    E.create(macSeed ^ 0x5eed);
    Serial.printf("EV %s was born\n", E.name);
    saveMind();
  }
  E.dirty = false;
  lastMilestones = E.milestones;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  bool nowOk = esp_now_init() == ESP_OK;
  if (nowOk) {
    esp_now_register_recv_cb(onRecv);
    esp_now_register_send_cb(onSent);
    esp_now_peer_info_t p = {};
    memcpy(p.peer_addr, BCAST, 6);
    p.channel = 1; p.ifidx = WIFI_IF_STA; p.encrypt = false;
    nowOk = esp_now_add_peer(&p) == ESP_OK;
  }
  Serial.printf("EV boot: mac %02x:%02x:%02x:%02x:%02x:%02x espnow %s littlefs %s (%u/%u bytes)\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], nowOk ? "ok" : "FAILED",
                fsOk ? "ok" : "FAILED", fsOk ? (unsigned)LittleFS.usedBytes() : 0, fsOk ? (unsigned)LittleFS.totalBytes() : 0);

  // boot: name fades in, one shimmer
  for (int f = 0; f <= 60; f++) {
    memset(gray, 0, sizeof gray); memset(hl, 0, sizeof hl);
    uint8_t v = (uint8_t)clampf(f * 7, 0, 255);
    text(52, 18, "05 / DYAD", v, true, 2);
    char nm[24]; snprintf(nm, sizeof nm, "%s WAKES", E.name);
    text(80 - (int)strlen(nm) * 2, 42, nm, (uint8_t)clampf((f - 20) * 9, 0, 230));
    if (f == 30) irisTrigger(IRIS_SWEEP, 30);
    irisTick();
    compose(); tftPush(); updateLed();
    frameNo++;
    delay(30);
  }
  irisMode = IRIS_NONE; irisEnd = frameNo;
}

void loop() {
  static uint32_t lastFrame = 0, lastTick = 0, lastSave = 0;
  uint32_t now = millis();
  pollSerial();
  pollButton();
  char pkt[252];
  while (popRadio(pkt)) E.receive(pkt);
  while (now - lastTick >= tickMs) { lastTick += tickMs; E.tick(1); }
  if (E.dirty && now - lastSave > 5000) { saveMind(); lastSave = now; }
  if (E.milestones != lastMilestones) {
    lastMilestones = E.milestones;
    if (irisMode == IRIS_NONE) irisTrigger(IRIS_FOCUS, 60);
  }
  if (now - lastFrame < 33) return;
  lastFrame = now;

  memset(gray, 0, sizeof gray); memset(hl, 0, sizeof hl);
  switch (view) {
    case 0: drawTalk(); break;
    case 1: drawMind(); break;
    case 2: drawWorld(); break;
    default: drawMemory(); break;
  }
  drawHeader();
  irisTick();
  if (fadeIn < 256) fadeIn = min(256, fadeIn + 32);
  compose(); tftPush(); updateLed();
  frameNo++;
}
