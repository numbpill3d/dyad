// 05 / DYAD — native host for the personality engine.
//
//   dyad_ghost --ghost <mind.txt>     a laptop-side persona speaking the same
//                                     serial protocol as a dongle (stdin/stdout)
//   dyad_ghost --pair <seconds> [--at T:A|B:command ...] [--seed N]
//                                     two personas in-process, simulated time,
//                                     prints a readable transcript
#include "../src/dyad_engine.h"
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <vector>

using dyad::Engine;

// ------------------------------------------------------------------ pair sim
struct SimSide { Engine e; std::vector<std::string> outbox; const char* tag; };
static uint32_t simT = 0;
static bool verbose = false;

static void simOut(void* ctx, const char* line) {
  SimSide* s = (SimSide*)ctx;
  if (!strncmp(line, "TX ", 3)) {
    dyad::Msg m;
    if (Engine::wireDecode(line + 3, m)) {
      printf("[d%u %02u:%02u] %-8s %-9s| %s\n", simT / 1800 + 1, (simT % 1800) / 60, simT % 60,
             s->e.name, dyad::INTENT_NAME[m.intent], m.text);
      if (verbose) printf("            a=\"%s\" b=\"%s\" tone=%d depth=%u\n", m.a, m.b, m.tone, m.depth);
    }
    return;
  }
  if (!strncmp(line, "RX ", 3)) return;
  printf("            %s %s\n", s->tag, line);
}
static void simSend(void* ctx, const char* wire) { ((SimSide*)ctx)->outbox.push_back(wire); }

static int runPair(uint32_t seconds, std::vector<std::string>& script, uint32_t seed) {
  static SimSide A, B;
  A.tag = "A"; B.tag = "B";
  A.e.create(seed * 2654435761u + 11);
  B.e.create(seed * 2246822519u + 97);
  A.e.host = {&A, simOut, simSend};
  B.e.host = {&B, simOut, simSend};
  printf("A = %s (%s voice) in %s\nB = %s (%s voice) in %s\n\n", A.e.name, dyad::VOICE_NAME[A.e.voice], A.e.env.place,
         B.e.name, dyad::VOICE_NAME[B.e.voice], B.e.env.place);
  for (simT = 0; simT < seconds; simT++) {
    for (auto& s : script) {
      unsigned t; char who; char cmd[200];
      if (sscanf(s.c_str(), "%u:%c:%199[^\n]", &t, &who, cmd) == 3 && t == simT) {
        printf("            >>> operator -> %c: %s\n", who, cmd);
        (who == 'A' ? A : B).e.command(cmd);
      }
    }
    A.e.tick(1); B.e.tick(1);
    auto a = A.outbox; A.outbox.clear();
    auto b = B.outbox; B.outbox.clear();
    for (auto& w : a) B.e.receive(w.c_str());
    for (auto& w : b) A.e.receive(w.c_str());
  }
  printf("\n");
  A.e.host.out = [](void*, const char* l) { printf("A %s\n", l); };
  B.e.host.out = [](void*, const char* l) { printf("B %s\n", l); };
  A.e.dump(); printf("\n"); B.e.dump();
  return 0;
}

// ------------------------------------------------------------------ ghost
static std::string mindPath;
static void ghostOut(void*, const char* line) { printf("%s\n", line); fflush(stdout); }
static void ghostSend(void*, const char*) {}  // TX lines on stdout are the transport
static void fileWrite(void* f, const char* line) { fprintf((FILE*)f, "%s\n", line); }

static void saveMind(Engine& e) {
  std::string tmp = mindPath + ".tmp";
  FILE* f = fopen(tmp.c_str(), "w");
  if (!f) return;
  e.save(fileWrite, f);
  fclose(f);
  rename(tmp.c_str(), mindPath.c_str());
  e.dirty = false;
}

static int runGhost(int speed) {
  static Engine e;
  e.host = {nullptr, ghostOut, ghostSend};
  FILE* f = fopen(mindPath.c_str(), "r");
  if (f) {
    e.create(1); e.beginLoad();
    char line[300];
    while (fgets(line, sizeof line, f)) { line[strcspn(line, "\r\n")] = 0; e.loadLine(line); }
    fclose(f);
    e.rng ^= (uint32_t)time(nullptr);
    printf("EV ghost %s woke up (day %lu)\n", e.name, (unsigned long)e.day());
  } else {
    e.create((uint32_t)time(nullptr) ^ (uint32_t)getpid() << 8);
    printf("EV ghost %s was born\n", e.name);
    saveMind(e);
  }
  fflush(stdout);
  struct timespec last; clock_gettime(CLOCK_MONOTONIC, &last);
  uint32_t lastSave = 0;
  char line[400];
  for (;;) {
    struct pollfd p = {0, POLLIN, 0};
    if (poll(&p, 1, 50) > 0) {
      if (!fgets(line, sizeof line, stdin)) break;
      line[strcspn(line, "\r\n")] = 0;
      if (!strncmp(line, "speed ", 6)) { speed = atoi(line + 6); if (speed < 1) speed = 1; printf("OK speed %d\n", speed); fflush(stdout); }
      else if (!strcmp(line, "log")) { printf("ERR the ghost keeps no log; the host tool records the transcript\n"); fflush(stdout); }
      else if (line[0] && !e.command(line)) { printf("ERR unknown command (try help)\n"); fflush(stdout); }
    }
    struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
    long ms = (now.tv_sec - last.tv_sec) * 1000 + (now.tv_nsec - last.tv_nsec) / 1000000;
    if (ms >= 1000 / speed) { last = now; e.tick(1); }
    if (e.dirty && e.age - lastSave >= 5) { saveMind(e); lastSave = e.age; }
  }
  saveMind(e);
  return 0;
}

int main(int argc, char** argv) {
  std::vector<std::string> script;
  uint32_t pair = 0, seed = 1; int speed = 1;
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    if (a == "--ghost" && i + 1 < argc) mindPath = argv[++i];
    else if (a == "--pair" && i + 1 < argc) pair = (uint32_t)atoi(argv[++i]);
    else if (a == "--at" && i + 1 < argc) script.push_back(argv[++i]);
    else if (a == "--seed" && i + 1 < argc) seed = (uint32_t)atoi(argv[++i]);
    else if (a == "--speed" && i + 1 < argc) speed = atoi(argv[++i]);
    else if (a == "-v") verbose = true;
  }
  if (pair) return runPair(pair, script, seed);
  if (!mindPath.empty()) return runGhost(speed);
  fprintf(stderr, "usage: dyad_ghost --ghost mind.txt [--speed N] | --pair SECONDS [--seed N] [--at T:A:cmd] [-v]\n");
  return 1;
}
