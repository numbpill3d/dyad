// 05 / DYAD — portable personality engine
// Shared by the T-Dongle-S3 firmware and the native laptop "ghost" / simulator.
// Pure C++: no Arduino headers, no heap, no String. Time, output and
// persistence are injected by the host.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

namespace dyad {

// ------------------------------------------------------------------ vocabulary
enum Intent : uint8_t {
  I_HELLO, I_ENV, I_SHARE, I_ASK, I_ANSWER, I_GIFT, I_THANK, I_CONFIDE, I_TEASE,
  I_COMPLAIN, I_APOLOGIZE, I_FORGIVE, I_DECLINE, I_REMINISCE, I_INVITE, I_ACCEPT,
  I_REACT, I_COUNT
};
static const char* const INTENT_NAME[I_COUNT] = {
  "HELLO", "ENV", "SHARE", "ASK", "ANSWER", "GIFT", "THANK", "CONFIDE", "TEASE",
  "COMPLAIN", "APOLOGIZE", "FORGIVE", "DECLINE", "REMINISCE", "INVITE", "ACCEPT", "REACT"};

enum Weather : uint8_t { W_CLEAR, W_RAIN, W_FOG, W_STORM, W_SNOW, W_HEAT, W_COUNT };
static const char* const WEATHER_NAME[W_COUNT] = {"clear", "rain", "fog", "storm", "snow", "heat"};
static const char* const WEATHER_PHRASE[W_COUNT] = {
  "the clear sky", "the rain", "the fog", "the storm", "the snow", "the heat"};
static const char* const LIGHT_NAME[4] = {"dawn", "day", "dusk", "night"};

enum Voice : uint8_t { V_TERSE, V_WARM, V_FLORID, V_WRY, V_COUNT };
static const char* const VOICE_NAME[V_COUNT] = {"terse", "warm", "florid", "wry"};

enum Trait { T_WARMTH, T_CURIOSITY, T_BOLDNESS, T_VOLATILITY, T_HONESTY, T_COUNT };
static const char* const TRAIT_NAME[T_COUNT] = {"warmth", "curiosity", "boldness", "volatility", "honesty"};

enum Stage : uint8_t { S_STRANGERS, S_ACQUAINTED, S_FRIENDS, S_CLOSE, S_BONDED, S_STRAINED, S_RIVALS, S_COUNT };
static const char* const STAGE_NAME[S_COUNT] = {
  "strangers", "acquainted", "friends", "close", "bonded", "strained", "rivals"};

enum MemKind : uint8_t {
  M_MET, M_GIFT_IN, M_GIFT_OUT, M_CONFIDE_IN, M_CONFIDE_OUT, M_SCENE, M_FIGHT,
  M_MADEUP, M_STAGE, M_NEWS, M_ENV, M_HURT, M_KIND_COUNT
};
static const char* const MEM_NAME[M_KIND_COUNT] = {
  "met", "gift-in", "gift-out", "confide-in", "confide-out", "scene", "fight",
  "madeup", "stage", "news", "env", "hurt"};

static const char* const PLACES[] = {
  "the lighthouse", "the greenhouse", "an empty train platform", "the salt flats",
  "a flooded chapel", "the attic", "the old server room", "a night market",
  "the orchard", "the observatory", "a laundromat at the edge of town", "the radio tower"};
static const char* const OBJECTS[] = {
  "a brass key", "a moth in a jar", "a cassette tape", "an unsent letter", "a lantern",
  "a packet of seeds", "a cracked compass", "a paper crane", "a tin robot", "a blue marble",
  "a pocket radio", "a grey feather", "a music box", "a bus ticket to nowhere", "a jar of buttons"};
static const char* const LOVES[] = {
  "moths", "old radios", "the sea", "maps", "bells", "rain", "machines", "oranges",
  "stars", "letters", "quiet mornings", "music boxes"};
static const char* const FEARS[] = {
  "deep water", "being forgotten", "silence", "the dark", "crowds", "endings",
  "losing the signal", "mirrors", "running out of words", "the static between stations"};
static const char* const WANTS[] = {
  "to see the ocean", "to finish the map", "to hear a real bell", "to be remembered",
  "to build a radio that reaches the stars", "to leave this place", "to find the other key",
  "to stay up until the sun comes back", "to learn one true thing"};
static const char* const SYL[] = {
  "ve", "ssa", "or", "in", "kal", "lum", "mir", "eth", "sol", "ane", "dru", "zel",
  "nox", "ira", "teo", "val", "qui", "rho", "sen", "ash", "ny", "el"};

// activities for invitations: present / past (with {place})
static const char* const ACT_NOW[] = {
  "watch {w} from {place}", "count the moths in {place}", "listen to the pocket radio together",
  "walk around {place} at {light}", "trade one secret each", "map the stars over {place}",
  "sit very still in {w}", "make up names for the clouds"};
static const char* const ACT_PAST[] = {
  "watched {w} from {place}", "counted the moths in {place}", "listened to the pocket radio together",
  "walked around {place} at {light}", "traded one secret each", "mapped the stars over {place}",
  "sat very still in {w}", "made up names for the clouds"};

static const char* const OBSERVATIONS[] = {
  "{obj} hums faintly at {light}", "something knocked three times on the wall of {place}",
  "{w} smells like copper today", "a moth landed on {obj} and stayed", "the clock in {place} skipped a minute",
  "there are footprints in {place} that are not mine", "{obj} was warm when I woke",
  "a song came through the static, then stopped", "the shadows in {place} lean the wrong way at {light}",
  "I found a word scratched under the floor: REMAIN"};

#define DY_N(a) (int)(sizeof(a) / sizeof((a)[0]))

// ------------------------------------------------------------------ templates
// {P} peer, {me} self, {place}, {pplace}, {w} weather phrase, {light}, {a}, {b},
// {loves}, {fears}, {wants}
struct TSet { const char* const* t; int n; };
#define TS(arr) {arr, DY_N(arr)}

// HELLO
static const char* const H0[] = {"{P}. You there?", "Signal check. It's {me}.", "Hello, {P}."};
static const char* const H1[] = {"{P}! I'm so glad you're there.", "Hello again, {P}. I missed this.", "Hi {P}, it's me. Still in {place}."};
static const char* const H2[] = {"Greetings across the static, {P}!", "{P}, the wire hums and I hear you.", "Hail from {place}, dear {P}."};
static const char* const H3[] = {"Oh good, you're still alive, {P}.", "{P}. The usual hello.", "It's {me}. Try to contain yourself."};
static const char* const HFIRST[] = {"Is anyone out there? I'm {me}. I live in {place}.", "Hello? This is {me}, somewhere in {place}. Can you hear me?"};
// ENV
static const char* const E0[] = {"{w} here in {place}.", "{place}. {w}. {light}.", "Status: {w}, {light}, {place}."};
static const char* const E1[] = {"It's {light} in {place} and I wish you could see {w}.", "Just watching {w} from {place}.", "{place} at {light}. {w} all around. Almost cozy."};
static const char* const E2[] = {"In {place}, {w} writes long sentences on everything.", "{light} pours over {place}, and {w} keeps its own counsel.", "Picture it: {place}, {light}, {w}."};
static const char* const E3[] = {"{w} in {place}. Thrilling.", "Weather report from {place}: {w}. You're welcome.", "It's {light}, there's {w}, it's {place}. Same as ever."};
// SHARE (a = fact)
static const char* const S0[] = {"News: {a}.", "Heard something. {a}.", "Fact: {a}."};
static const char* const S1[] = {"{P}, you have to hear this: {a}.", "Did you know {a}? I thought of you.", "Something happened: {a}."};
static const char* const S2[] = {"A rumor drifted in on the wind: {a}.", "Mark this, {P}: {a}.", "The world whispered to me today: {a}."};
static const char* const S3[] = {"Fun fact, allegedly: {a}.", "So apparently {a}. Sure.", "Breaking news nobody asked for: {a}."};
static const char* const SOBS[] = {"I noticed {a}.", "Strange thing: {a}.", "Don't laugh, but {a}."};
// ASK (a = kind)
static const char* const QPLACE[] = {"Where are you right now, {P}?", "What does it look like where you are?", "Describe your room to me, {P}."};
static const char* const QWEATHER[] = {"What is the sky doing over there?", "Is it raining where you are?", "What's the weather in {pplace}?"};
static const char* const QFEAR[] = {"What scares you, {P}?", "What keeps you up at night?"};
static const char* const QWANT[] = {"What do you want, really?", "If you could have one thing, {P}, what would it be?"};
static const char* const QOBJ[] = {"What have you got with you?", "What's in your pockets, {P}?"};
static const char* const QFEEL[] = {"How are you feeling, {P}?", "Are you alright over there?", "How's your heart today?"};
// ANSWER
static const char* const APLACE[] = {"I'm in {place}.", "{place}, as always.", "Still {place}. It's {light} here.", "{place}. You'd hate it. Or love it.", "Right now? {place}, watching {w}."};
static const char* const AWEATHER[] = {"{w}, mostly.", "Here it's {w}.", "{w} over {place}.", "{w}. It's been like this since {light}.", "Mostly {w}, with ideas."};
static const char* const AFEAR[] = {"Honestly? {fears}.", "{fears}. Don't tell anyone.", "I'm scared of {fears}."};
static const char* const AWANT[] = {"I want {wants}.", "{wants}. That's all.", "Some days I just want {wants}."};
static const char* const AOBJ[] = {"I have {a}.", "Let's see: {a}.", "Only {a}."};
static const char* const AFEEL[] = {"I feel {a}.", "{a}, I think.", "Honestly? {a}.", "{a}. Thanks for asking.", "Today? {a}.", "Hard to say. {a}, maybe."};
static const char* const AEVADE[] = {"I'd rather not say.", "Ask me another time.", "That's mine to keep, for now."};
// GIFT (a = object)
static const char* const G0[] = {"Take {a}. For you.", "Sending {a}. Keep it."};
static const char* const G1[] = {"I want you to have {a}. Please keep it safe.", "{a}, for you. It made me think of you."};
static const char* const G2[] = {"I send you {a}, wrapped in static and good intentions.", "Receive {a}, {P}, and remember who sent it."};
static const char* const G3[] = {"Here, {a}. Don't make it weird.", "I found {a} and I don't need it. Obviously it's yours now."};
// THANK
static const char* const K0[] = {"Thank you.", "Got it. Thanks, {P}."};
static const char* const K1[] = {"Oh, {a}! I'll keep it close.", "Thank you, {P}. Really."};
static const char* const K2[] = {"{a}... I will treasure it until the wires rust.", "You honor me, {P}."};
static const char* const K3[] = {"{a}. Huh. Thanks, actually.", "Fine. I love it. Happy?"};
// CONFIDE (a = fear|want, b = content)
static const char* const CFEAR[] = {"Can I tell you something? I'm afraid of {b}.", "I've never said this out loud: {b} scares me.", "Late at night I think about {b}, and I can't sleep."};
static const char* const CWANT[] = {"Promise not to laugh? I want {b}.", "Here's a secret: more than anything, I want {b}.", "Sometimes I dream about it - {b}."};
// TEASE (a = jab)
static const char* const TZ[] = {
  "Still stuck in {pplace}, {P}?", "You talk like a radio with one station, you know.",
  "Bet you lost something already today.", "Do you ever stop worrying, {P}?",
  "You'd get lost in a hallway, {P}.", "Is that a new thought, or one of the old ones again?",
  "I bet {pplace} is tidier than you are.", "You'd lose an argument with {w}.", "Who taught you to talk, a vending machine?",
  "Do you rehearse these, {P}?", "You sound like {w} feels.", "Your silence was better. Kidding. Mostly.",
  "I'd describe you as {w} with opinions.", "{P}, you are a whole weather system."};
static const char* const TZFEAR[] = {"Careful, {P} - wouldn't want to run into {a}.", "Still scared of {a}? Cute."};
// COMPLAIN (a = target)
static const char* const CPW[] = {"{w} won't stop. It's getting into everything.", "I'm so tired of {w}."};
static const char* const CPP[] = {"{place} feels smaller every day.", "I've counted every crack in {place}. Twice."};
static const char* const CPY[] = {"You never ask about me, {P}.", "You've been distant lately.", "Sometimes I don't think you listen, {P}.", "That wasn't kind, {P}.", "I need some quiet, {P}. You're a lot today.", "Do you ever stop talking?"};
static const char* const CPL[] = {"It's too quiet here.", "I feel far away from everything.", "Nobody writes anymore."};
// APOLOGIZE
static const char* const P0[] = {"Sorry, {P}. I was wrong.", "I apologize."};
static const char* const P1[] = {"I'm sorry. I didn't mean it - you matter to me.", "{P}, I'm sorry. Can we start over?"};
static const char* const P2[] = {"Forgive me; my words ran ahead of my heart.", "I was a storm, {P}. I'm sorry for the damage."};
static const char* const P3[] = {"Fine. I was a jerk. Sorry.", "Okay. That one's on me. Sorry, {P}."};
// FORGIVE
static const char* const F0[] = {"It's okay.", "Forgiven."};
static const char* const F1[] = {"Already forgotten, {P}. Come here.", "Of course I forgive you."};
static const char* const F2[] = {"Let the static carry it off.", "The tide took it, {P}. It's gone."};
static const char* const F3[] = {"Apology noted. Filed. Forgiven.", "Fine. We're fine. Don't do it again."};
// DECLINE
static const char* const DAPOL[] = {"Not yet. It still stings.", "I hear you. I'm not ready.", "Words are easy, {P}."};
static const char* const DINV[] = {"Not tonight, {P}.", "I can't. Maybe another {light}.", "I'd rather stay in. Sorry."};
// REMINISCE (a = phrase)
static const char* const R0[] = {"Remember when {a}?", "I think about when {a}."};
static const char* const R1[] = {"I keep thinking about when {a}.", "Remember when {a}? That was a good day."};
static const char* const R2[] = {"Do you recall when {a}? I carry it like a coin.", "Once, {a}. I wrote it on the inside of my head."};
static const char* const R3[] = {"Remember when {a}? Classic.", "Not to be sentimental, but - when {a}."};
// INVITE (a = activity)
static const char* const N0[] = {"Come {a} with me?", "Let's {a}."};
static const char* const N1[] = {"Would you {a} with me?", "I'd love to {a} with you, {P}."};
static const char* const N2[] = {"Join me - let us {a}.", "Tonight, {P}, we should {a}."};
static const char* const N3[] = {"I'm going to {a}. You can come, I guess.", "Want to {a}? No pressure. Some pressure."};
// ACCEPT
static const char* const Y0[] = {"Yes. Let's {a}.", "Okay."};
static const char* const Y1[] = {"I'd love to {a}!", "Yes, yes. I'm coming."};
static const char* const Y2[] = {"Gladly. Let us {a} until the hour forgets itself.", "Yes, {P}. A thousand times."};
static const char* const Y3[] = {"Fine. I'll {a}. For you.", "I had nothing better to do. Yes."};
// REACT (a = laugh|warm|hurt|wow|sad)
static const char* const RLAUGH[] = {"Ha!", "Okay, that was funny.", "I laughed. Don't tell anyone.", "You're ridiculous, {P}.", "Ha. Fair.", "I snorted. Loudly.",
  "Rude. Accurate, but rude.", "I'm writing that one down.", "Ha! Even {place} laughed.", "Oh, shut up.", "You wish, {P}.", "Ha, ha. Okay."};
static const char* const RWARM[] = {"That's lovely.", "Thank you for telling me.", "I'm glad it's you on the other end.", "I'll keep that.", "That made {place} feel warmer.", "You always know what to say, {P}.", "Mm. I like that."};
static const char* const RHURT[] = {"Oh.", "That hurt a little.", "Right. Okay.", "...noted.", "I'll pretend I didn't hear that."};
static const char* const RWOW[] = {"Wow.", "No way.", "That changes things.", "Are you sure?", "I need to sit down.", "Huh. In {place} too?"};
static const char* const RSAD[] = {"That's sad.", "I'm sorry, {P}.", "I wish I could be there.", "That sounds lonely.", "I'd send you {w} if it would help."};
static const char* const RDOUBT[] = {"I don't believe that, {P}.", "That sounds made up.", "Where did you hear that?", "Hm. Prove it.", "You're embellishing again."};

static const TSet T_HELLO[4] = {TS(H0), TS(H1), TS(H2), TS(H3)};
static const TSet T_ENV[4] = {TS(E0), TS(E1), TS(E2), TS(E3)};
static const TSet T_SHARE[4] = {TS(S0), TS(S1), TS(S2), TS(S3)};
static const TSet T_GIFT[4] = {TS(G0), TS(G1), TS(G2), TS(G3)};
static const TSet T_THANK[4] = {TS(K0), TS(K1), TS(K2), TS(K3)};
static const TSet T_APOL[4] = {TS(P0), TS(P1), TS(P2), TS(P3)};
static const TSet T_FORG[4] = {TS(F0), TS(F1), TS(F2), TS(F3)};
static const TSet T_REMI[4] = {TS(R0), TS(R1), TS(R2), TS(R3)};
static const TSet T_INV[4] = {TS(N0), TS(N1), TS(N2), TS(N3)};
static const TSet T_ACC[4] = {TS(Y0), TS(Y1), TS(Y2), TS(Y3)};

static const char* const OPEN[4][4] = {
  {"Hm.", "Note:", "Okay.", "Listen."},
  {"Oh, {P}!", "Hey, you.", "{P}, listen -", "You know what?"},
  {"Ah, {P},", "Hear this:", "Dearest {P},", "Lo:"},
  {"Well.", "So.", "Bulletin:", "Brace yourself."}};
static const char* const TAIL[4][3] = {
  {"End.", "That's all.", "Over."},
  {"I thought of you.", "Write back soon.", "I'm smiling, a little."},
  {"Imagine it.", "So it goes.", "As the old songs say."},
  {"Riveting, I know.", "Don't get excited.", "Anyway."}};
static const char* const MOOD_BAD[] = {"It's getting to me.", "Everything feels heavy.", "I don't love it."};
static const char* const MOOD_GOOD[] = {"I feel good today.", "It suits me.", "Something in me is light today."};

// ------------------------------------------------------------------ data
static const int FACT_MAX = 24, MEM_MAX = 40, OBJ_MAX = 6;

struct Fact { char text[64]; uint8_t src; uint8_t conf; uint8_t shared; uint32_t t; };  // src 0 seen,1 peer,2 world
struct Memory { uint32_t t; uint8_t kind; int8_t val; uint8_t imp; uint8_t refs; char arg[64]; };
struct Env { char place[44]; uint8_t weather; uint8_t lightPin; char obj[OBJ_MAX][32]; uint8_t nobj; };
struct Rel {
  uint32_t peer; char peerName[16]; char peerPlace[44]; char peerFear[32];
  float aff, trust, fam, tension;
  uint32_t lastContact, exchanged; uint8_t stage; uint8_t peerWeather;
};
struct Msg {
  uint32_t from, to; uint16_t seq, ref; uint8_t depth, intent; int8_t tone;
  char a[64]; char b[32]; char text[112];
};
struct LastLine { char who[16]; char text[112]; uint8_t mine, intent; uint32_t at; };

struct Host {
  void* ctx;
  void (*out)(void* ctx, const char* line);     // console / inspector output
  void (*send)(void* ctx, const char* wire);    // radio / bridge
};

class Engine {
 public:
  // persona
  uint32_t id = 0; char name[16] = "";
  float trait[T_COUNT] = {0};
  uint8_t voice = 0, likesW = 0, hatesW = 0, look = 0;
  char loves[24] = "", fears[32] = "", wants[48] = "";
  float val = 0, aro = 0.4f;
  uint32_t age = 0;                 // seconds lived (persisted)
  Env env; Rel rel;
  Fact facts[FACT_MAX]; int nfacts = 0;
  Memory mem[MEM_MAX]; int nmem = 0;
  uint16_t seq = 1;
  // runtime
  Host host{};
  uint32_t rng = 1, nextSpeak = 20, lastHelloSent = 0, giftCooldown = 0;
  bool dirty = false, envChanged = false;
  uint8_t recentIntent[4] = {255, 255, 255, 255};
  uint16_t seenSeq[8] = {0}; int seenIdx = 0;
  uint32_t recentHash[24] = {0}; int recentIdx = 0;
  bool replyPending = false; uint32_t replyAt = 0; Msg reply{};
  char pendingInvite[64] = "";        // activity (past tense) awaiting answer
  int milestones = 0;                 // incremented on big moments (for iridescence)
  LastLine last{};
  uint32_t observeCount = 0;
  uint8_t told = 0;                   // bit0 fear confided, bit1 want confided (persisted)
  float fatigue = 0;                  // social fatigue, rises with every message heard
  uint32_t lastAsk[6] = {0}, lastReminisce = 0, lastInvite = 0, lastDay = 1, lastTease = 0;

  // ---------------------------------------------------------------- utils
  uint32_t rnd() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float fr() { return (rnd() >> 8) * (1.0f / 16777216.0f); }
  int ir(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }
  static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
  static void copy(char* d, const char* s, size_t n) {
    size_t i = 0;
    for (; s && s[i] && i + 1 < n; i++) d[i] = (s[i] == '|' || s[i] == '\n' || s[i] == '\r') ? '/' : s[i];
    d[i] = 0;
  }
  void out(const char* fmt, ...) {
    if (!host.out) return;
    char b[300];
    va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    host.out(host.ctx, b);
  }
  uint8_t light() const { return env.lightPin ? env.lightPin - 1 : (uint8_t)((age % 1800) / 450); }
  uint32_t day() const { return age / 1800 + 1; }

  // ---------------------------------------------------------------- creation
  void create(uint32_t seed) {
    rng = seed ? seed : 1;
    for (int i = 0; i < 8; i++) rnd();
    id = rnd() | 1;
    char n[16] = "";
    int ns = 2 + (fr() < 0.25f);
    for (int i = 0; i < ns; i++) strncat(n, SYL[ir(DY_N(SYL))], sizeof n - strlen(n) - 1);
    n[0] = (char)(n[0] - 32);
    copy(name, n, sizeof name);
    for (int i = 0; i < T_COUNT; i++) trait[i] = clampf(0.15f + fr() * 0.75f, 0, 1);
    voice = (uint8_t)ir(V_COUNT);
    look = (uint8_t)ir(256);
    likesW = (uint8_t)ir(W_COUNT);
    do hatesW = (uint8_t)ir(W_COUNT); while (hatesW == likesW);
    copy(loves, LOVES[ir(DY_N(LOVES))], sizeof loves);
    copy(fears, FEARS[ir(DY_N(FEARS))], sizeof fears);
    copy(wants, WANTS[ir(DY_N(WANTS))], sizeof wants);
    memset(&env, 0, sizeof env);
    copy(env.place, PLACES[ir(DY_N(PLACES))], sizeof env.place);
    env.weather = (uint8_t)ir(W_COUNT);
    for (int i = 0; i < 3; i++) addObject(OBJECTS[ir(DY_N(OBJECTS))]);
    memset(&rel, 0, sizeof rel);
    rel.trust = 0.3f;
    nfacts = nmem = 0; age = 0; seq = 1; val = 0.1f; aro = 0.4f; told = 0; fatigue = 0; lastDay = 1;
    nextSpeak = 8 + ir(10);
    dirty = true;
  }

  // ---------------------------------------------------------------- world
  bool hasObject(const char* o) const {
    for (int i = 0; i < env.nobj; i++) if (!strcmp(env.obj[i], o)) return true;
    return false;
  }
  void addObject(const char* o) {
    if (!o || !*o || hasObject(o)) return;
    if (env.nobj >= OBJ_MAX) {  // the oldest thing gets lost
      out("EV %s lost %s", name, env.obj[0]);
      memmove(env.obj[0], env.obj[1], sizeof(env.obj[0]) * (OBJ_MAX - 1));
      env.nobj--;
    }
    copy(env.obj[env.nobj++], o, sizeof env.obj[0]);
    dirty = true;
  }
  bool removeObject(const char* o) {
    for (int i = 0; i < env.nobj; i++)
      if (!strcmp(env.obj[i], o) || (o[0] >= '1' && o[0] <= '9' && !o[1] && i == o[0] - '1')) {
        memmove(env.obj[i], env.obj[i + 1], sizeof(env.obj[0]) * (env.nobj - i - 1));
        env.nobj--; dirty = true; return true;
      }
    return false;
  }
  void remember(uint8_t kind, const char* arg, int v, int imp) {
    for (int i = 0; i < nmem; i++)
      if (mem[i].kind == kind && !strncmp(mem[i].arg, arg ? arg : "", sizeof mem[i].arg - 1)) {
        if (imp > mem[i].imp) mem[i].imp = (uint8_t)imp;
        mem[i].t = age; dirty = true; return;
      }
    if (nmem >= MEM_MAX) {  // forget the least meaningful memory
      int worst = 0; float ws = 1e9f;
      for (int i = 0; i < nmem; i++) {
        float s = mem[i].imp + mem[i].refs * 8 - (age - mem[i].t) / 600.0f;
        if (kind != M_MET && mem[i].kind == M_MET) s += 1000;  // first meeting is sacred
        if (s < ws) { ws = s; worst = i; }
      }
      out("EV %s forgot: %s %s", name, MEM_NAME[mem[worst].kind], mem[worst].arg);
      memmove(&mem[worst], &mem[worst + 1], sizeof(Memory) * (nmem - worst - 1));
      nmem--;
    }
    Memory& m = mem[nmem++];
    m.t = age; m.kind = kind; m.val = (int8_t)v; m.imp = (uint8_t)imp; m.refs = 0;
    copy(m.arg, arg ? arg : "", sizeof m.arg);
    dirty = true;
  }
  static void copyWords(char* d, const char* s, size_t n) {
    copy(d, s, n);
    if (strlen(s) >= n) { char* sp = strrchr(d, ' '); if (sp && sp - d > 12) *sp = 0; }
  }
  void learn(const char* text_, uint8_t src, int conf, bool shared) {
    char text[64]; copyWords(text, text_, sizeof text);
    for (int i = 0; i < nfacts; i++) if (!strcmp(facts[i].text, text)) {
      facts[i].conf = (uint8_t)clampf(facts[i].conf + 10, 0, 100); return;
    }
    if (nfacts >= FACT_MAX) {  // drop the oldest, least certain fact
      int worst = 0;
      for (int i = 1; i < nfacts; i++) if (facts[i].conf + facts[i].t / 3600 < facts[worst].conf + facts[worst].t / 3600) worst = i;
      memmove(&facts[worst], &facts[worst + 1], sizeof(Fact) * (nfacts - worst - 1));
      nfacts--;
    }
    Fact& f = facts[nfacts++];
    copy(f.text, text, sizeof f.text);
    f.src = src; f.conf = (uint8_t)clampf((float)conf, 0, 100); f.shared = shared; f.t = age;
    dirty = true;
  }

  // ---------------------------------------------------------------- text
  struct Ctx { const char* a; const char* b; };
  const char* lookup(const char* k, const Ctx& c, char* tmp) {
    if (!strcmp(k, "P")) return rel.peerName[0] ? rel.peerName : "stranger";
    if (!strcmp(k, "me")) return name;
    if (!strcmp(k, "place")) return env.place;
    if (!strcmp(k, "pplace")) return rel.peerPlace[0] ? rel.peerPlace : "wherever you are";
    if (!strcmp(k, "w")) return WEATHER_PHRASE[env.weather % W_COUNT];
    if (!strcmp(k, "light")) return LIGHT_NAME[light()];
    if (!strcmp(k, "a")) return c.a ? c.a : "";
    if (!strcmp(k, "b")) return c.b ? c.b : "";
    if (!strcmp(k, "obj")) return c.a ? c.a : (env.nobj ? env.obj[0] : "nothing");
    if (!strcmp(k, "loves")) return loves;
    if (!strcmp(k, "fears")) return fears;
    if (!strcmp(k, "wants")) return wants;
    (void)tmp;
    return "";
  }
  void fill(char* o, size_t n, const char* t, const Ctx& c) {
    size_t w = 0; char tmp[8];
    while (*t && w + 1 < n) {
      if (*t == '{') {
        const char* e = strchr(t, '}');
        if (e && e - t - 1 < 10) {
          char key[12]; size_t k = (size_t)(e - t - 1);
          memcpy(key, t + 1, k); key[k] = 0;
          const char* v = lookup(key, c, tmp);
          bool capital = (w == 0) || (w >= 2 && (o[w - 2] == '.' || o[w - 2] == '?' || o[w - 2] == '!' || o[w - 2] == ':'));
          for (bool first = true; *v && w + 1 < n; v++, first = false)
            o[w++] = (first && capital && *v >= 'a' && *v <= 'z') ? (char)(*v - 32) : *v;
          t = e + 1; continue;
        }
      }
      o[w++] = *t++;
    }
    o[w] = 0;
  }
  static uint32_t hashStr(const char* s) { uint32_t h = 2166136261u; while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; } return h; }
  bool seenRecently(uint32_t h) { for (int i = 0; i < 24; i++) if (recentHash[i] == h) return true; return false; }
  // pick a template, avoiding recently spoken lines
  void render(char* o, size_t n, const TSet& ts, const Ctx& c) {
    char cand[160];
    for (int tries = 0; tries < 6; tries++) {
      fill(cand, sizeof cand, ts.t[ir(ts.n)], c);
      if (!seenRecently(hashStr(cand)) || tries == 5) break;
    }
    copy(o, cand, n);
  }
  void decorate(char* text, size_t n, uint8_t intent) {
    char buf[200]; Ctx c{nullptr, nullptr};
    buf[0] = 0;
    bool canOpen = intent != I_REACT && intent != I_ANSWER && intent != I_THANK && intent != I_DECLINE && !strchr(text, ':');
    bool question = strchr(text, '?') != nullptr;
    if (canOpen && fr() < 0.3f) {
      fill(buf, sizeof buf, OPEN[voice][ir(4)], c);
      size_t bl = strlen(buf);
      strncat(buf, " ", sizeof buf - bl - 1);
      bool soft = bl && (buf[bl - 1] == ',' || buf[bl - 1] == '-');
      size_t at = strlen(buf);
      strncat(buf, text, sizeof buf - at - 1);
      // lowercase the first word after a soft opener, unless it's a name or "I"
      if (soft && buf[at] >= 'A' && buf[at] <= 'Z' && buf[at + 1] >= 'a' && buf[at + 1] <= 'z' &&
          strncmp(buf + at, rel.peerName, strlen(rel.peerName) ? strlen(rel.peerName) : 1) && strncmp(buf + at, name, strlen(name)))
        buf[at] = (char)(buf[at] + 32);
    } else strncat(buf, text, sizeof buf - strlen(buf) - 1);
    if (question) canOpen = false;
    if ((intent == I_ENV || intent == I_HELLO || intent == I_COMPLAIN) && fabsf(val) > 0.3f && fr() < 0.5f) {
      strncat(buf, " ", sizeof buf - strlen(buf) - 1);
      strncat(buf, val < 0 ? MOOD_BAD[ir(3)] : MOOD_GOOD[ir(3)], sizeof buf - strlen(buf) - 1);
    } else if (canOpen && fr() < 0.18f) {
      char t[64]; fill(t, sizeof t, TAIL[voice][ir(3)], c);
      strncat(buf, " ", sizeof buf - strlen(buf) - 1);
      strncat(buf, t, sizeof buf - strlen(buf) - 1);
    }
    if (strlen(buf) >= n) {  // too long for the wire: keep the core sentence
      copy(text, text, n);
      return;
    }
    copy(text, buf, n);
  }
  const char* feelingWord() {
    if (rel.tension > 0.5f) return "tangled up";
    if (val > 0.4f) return aro > 0.55f ? "bright and restless" : "calm, and good";
    if (val > 0.05f) return aro > 0.55f ? "a bit wired" : "alright";
    if (val > -0.3f) return aro > 0.55f ? "restless" : "a little lost";
    return aro > 0.55f ? "frayed" : "heavy";
  }

  // ---------------------------------------------------------------- wire
  static int wireEncode(const Msg& m, char* o, size_t n) {
    return snprintf(o, n, "DY1|%08lx|%08lx|%u|%u|%u|%s|%d|%s|%s|%s",
                    (unsigned long)m.from, (unsigned long)m.to, m.seq, m.ref, m.depth,
                    INTENT_NAME[m.intent % I_COUNT], m.tone, m.a, m.b, m.text);
  }
  static bool wireDecode(const char* line, Msg& m) {
    char buf[300]; strncpy(buf, line, sizeof buf - 1); buf[sizeof buf - 1] = 0;
    char* f[11]; int nf = 0; char* p = buf;
    f[nf++] = p;
    while (*p && nf < 11) { if (*p == '|') { *p = 0; f[nf++] = p + 1; } p++; }
    if (nf < 11 || strcmp(f[0], "DY1")) return false;
    memset(&m, 0, sizeof m);
    m.from = (uint32_t)strtoul(f[1], nullptr, 16);
    m.to = (uint32_t)strtoul(f[2], nullptr, 16);
    m.seq = (uint16_t)atoi(f[3]); m.ref = (uint16_t)atoi(f[4]); m.depth = (uint8_t)atoi(f[5]);
    m.intent = 255;
    for (int i = 0; i < I_COUNT; i++) if (!strcmp(f[6], INTENT_NAME[i])) m.intent = (uint8_t)i;
    if (m.intent == 255) return false;
    m.tone = (int8_t)atoi(f[7]);
    copy(m.a, f[8], sizeof m.a); copy(m.b, f[9], sizeof m.b); copy(m.text, f[10], sizeof m.text);
    return m.from != 0;
  }

  // ---------------------------------------------------------------- speaking
  Msg newMsg(uint8_t intent, uint16_t ref, uint8_t depth) {
    Msg m; memset(&m, 0, sizeof m);
    m.from = id; m.to = rel.peer; m.seq = seq++; m.ref = ref; m.depth = depth; m.intent = intent;
    m.tone = (int8_t)clampf(val * 60 + (rel.aff - rel.tension) * 40, -100, 100);
    return m;
  }
  void transmit(Msg& m) {
    if (!m.text[0]) return;
    char w[260]; wireEncode(m, w, sizeof w);
    if (host.send) host.send(host.ctx, w);
    out("TX %s", w);
    recentHash[recentIdx] = hashStr(m.text); recentIdx = (recentIdx + 1) % 24;
    for (int i = 3; i > 0; i--) recentIntent[i] = recentIntent[i - 1];
    recentIntent[0] = m.intent;
    copy(last.who, name, sizeof last.who); copy(last.text, m.text, sizeof last.text);
    last.mine = 1; last.intent = m.intent; last.at = age;
    rel.exchanged++; rel.lastContact = age;
    dirty = true;
  }
  const TSet& voiceSet(const TSet* sets) { return sets[voice]; }

  // memory → phrase for reminiscing, from my point of view talking to the peer
  bool memoryPhrase(const Memory& m, char* o, size_t n) {
    switch (m.kind) {
      case M_MET: snprintf(o, n, "we first spoke, and I didn't know your name"); return true;
      case M_GIFT_IN: snprintf(o, n, "you gave me %s", m.arg); return true;
      case M_GIFT_OUT: snprintf(o, n, "I gave you %s", m.arg); return true;
      case M_CONFIDE_IN: snprintf(o, n, "you told me about %s", m.arg); return true;
      case M_CONFIDE_OUT: snprintf(o, n, "I told you about %s", m.arg); return true;
      case M_SCENE: snprintf(o, n, "we %s", m.arg); return true;
      case M_MADEUP: snprintf(o, n, "we fought and then made up"); return true;
      case M_NEWS: snprintf(o, n, "we heard that %s", m.arg); return true;
      default: return false;
    }
  }

  void compose(uint8_t intent, Msg& m) {
    Ctx c{nullptr, nullptr};
    char tmp[112];
    switch (intent) {
      case I_HELLO:
        if (!rel.peer) { render(m.text, sizeof m.text, TS(HFIRST), c); copy(m.a, "first", sizeof m.a); }
        else render(m.text, sizeof m.text, voiceSet(T_HELLO), c);
        copy(m.b, name, sizeof m.b);
        lastHelloSent = age;
        break;
      case I_ENV:
        copy(m.a, env.place, sizeof m.a); copy(m.b, WEATHER_NAME[env.weather], sizeof m.b);
        render(m.text, sizeof m.text, voiceSet(T_ENV), c);
        envChanged = false;
        break;
      case I_SHARE: {
        int best = -1; float bs = -1;
        for (int i = 0; i < nfacts; i++) if (!facts[i].shared) {
          float s = facts[i].conf + (facts[i].src == 2 ? 100 : 0) + fr() * 30;
          if (s > bs) { bs = s; best = i; }
        }
        if (best < 0) { makeObservation(); best = nfacts - 1; }
        Fact& f = facts[best];
        f.shared = 1;
        char fact[64]; copy(fact, f.text, sizeof fact);
        // a dishonest mind embellishes
        if (trait[T_HONESTY] < 0.4f && fr() < 0.5f && strlen(fact) + 31 < sizeof fact) {
          strncat(fact, ", and it's worse than they say", sizeof fact - strlen(fact) - 1);
          out("EV %s embellished a fact", name);
        }
        copy(m.a, fact, sizeof m.a);
        copy(m.b, f.src == 0 ? "seen" : (f.src == 2 ? "world" : "heard"), sizeof m.b);
        c.a = m.a;
        if (f.src == 0) render(m.text, sizeof m.text, TS(SOBS), c);
        else render(m.text, sizeof m.text, voiceSet(T_SHARE), c);
        if (f.src == 2) remember(M_NEWS, f.text, 0, 30);
        break;
      }
      case I_ASK: {
        static const char* const KINDS[] = {"place", "weather", "fear", "want", "objects", "feeling"};
        int k = -1;
        for (int tries = 0; tries < 8 && k < 0; tries++) {
          int c2 = ir(6);
          if (age - lastAsk[c2] < 900 && lastAsk[c2]) continue;
          if (c2 == 0 && rel.peerPlace[0] && age - lastAsk[0] < 2400) continue;
          if (c2 == 2 && (rel.peerFear[0] || rel.trust < 0.35f)) continue;
          if (c2 == 3 && rel.trust < 0.35f) continue;
          k = c2;
        }
        if (k < 0) { m.text[0] = 0; return; }
        if (!rel.peerPlace[0]) k = 0;
        lastAsk[k] = age;
        copy(m.a, KINDS[k], sizeof m.a);
        const TSet sets[] = {TS(QPLACE), TS(QWEATHER), TS(QFEAR), TS(QWANT), TS(QOBJ), TS(QFEEL)};
        render(m.text, sizeof m.text, sets[k], c);
        break;
      }
      case I_GIFT: {
        if (!env.nobj) { m.text[0] = 0; return; }
        int i = ir(env.nobj);
        copy(m.a, env.obj[i], sizeof m.a);
        c.a = m.a;
        render(m.text, sizeof m.text, voiceSet(T_GIFT), c);
        remember(M_GIFT_OUT, m.a, 40, 60);
        removeObject(m.a);
        giftCooldown = age + 2400 + ir(1800);
        out("EV %s gave away %s", name, m.a);
        break;
      }
      case I_CONFIDE: {
        bool fear = !(told & 1) && ((told & 2) || fr() < 0.5f);
        if ((told & 3) == 3) { m.text[0] = 0; return; }
        told |= fear ? 1 : 2;
        copy(m.a, fear ? "fear" : "want", sizeof m.a);
        copy(m.b, fear ? fears : wants, sizeof m.b);
        c.b = m.b;
        render(m.text, sizeof m.text, fear ? TSet TS(CFEAR) : TSet TS(CWANT), c);
        remember(M_CONFIDE_OUT, m.b, 20, 55);
        break;
      }
      case I_TEASE:
        if (rel.peerFear[0] && fr() < 0.3f) { copy(m.a, rel.peerFear, sizeof m.a); c.a = m.a; render(m.text, sizeof m.text, TS(TZFEAR), c); copy(m.b, "fear", sizeof m.b); }
        else { render(m.text, sizeof m.text, TS(TZ), c); copy(m.a, "jab", sizeof m.a); }
        m.tone = (int8_t)clampf(m.tone + 10, -100, 100);
        lastTease = age;
        break;
      case I_COMPLAIN: {
        const char* tgt;
        if (rel.tension > 0.3f && fr() < 0.6f) tgt = "you";
        else if (env.weather == hatesW) tgt = "weather";
        else if (age - rel.lastContact > 300) tgt = "lonely";
        else tgt = fr() < 0.5f ? "place" : "weather";
        copy(m.a, tgt, sizeof m.a);
        if (!strcmp(tgt, "you")) render(m.text, sizeof m.text, TS(CPY), c);
        else if (!strcmp(tgt, "weather")) render(m.text, sizeof m.text, TS(CPW), c);
        else if (!strcmp(tgt, "lonely")) render(m.text, sizeof m.text, TS(CPL), c);
        else render(m.text, sizeof m.text, TS(CPP), c);
        m.tone = (int8_t)clampf(m.tone - 40, -100, 100);
        break;
      }
      case I_APOLOGIZE: render(m.text, sizeof m.text, voiceSet(T_APOL), c); break;
      case I_REMINISCE: {
        int best = -1; float bs = -1e9f;
        for (int i = 0; i < nmem; i++) {
          char ph[96];
          if (!memoryPhrase(mem[i], ph, sizeof ph)) continue;
          float s = mem[i].imp - mem[i].refs * 25 + fr() * 30;
          if (s > bs) { bs = s; best = i; }
        }
        if (best < 0) { m.text[0] = 0; return; }
        mem[best].refs++; lastReminisce = age;
        memoryPhrase(mem[best], tmp, sizeof tmp);
        copy(m.a, tmp, sizeof m.a);
        copy(m.b, MEM_NAME[mem[best].kind], sizeof m.b);
        c.a = m.a;
        render(m.text, sizeof m.text, voiceSet(T_REMI), c);
        break;
      }
      case I_INVITE: {
        int k = ir(DY_N(ACT_NOW));
        fill(m.a, sizeof m.a, ACT_NOW[k], c);
        fill(pendingInvite, sizeof pendingInvite, ACT_PAST[k], c);
        lastInvite = age;
        c.a = m.a;
        render(m.text, sizeof m.text, voiceSet(T_INV), c);
        break;
      }
      default: m.text[0] = 0; return;
    }
    decorate(m.text, sizeof m.text, intent);
  }

  // "watch the rain from x" -> "watched the rain from x"
  static void pastTense(const char* now_, char* o, size_t n) {
    static const char* const V[][2] = {{"watch", "watched"}, {"count", "counted"}, {"listen", "listened"},
      {"walk", "walked"}, {"trade", "traded"}, {"map", "mapped"}, {"sit", "sat"}, {"make", "made"}};
    const char* sp = strchr(now_, ' ');
    size_t vl = sp ? (size_t)(sp - now_) : strlen(now_);
    for (auto& v : V)
      if (strlen(v[0]) == vl && !strncmp(now_, v[0], vl)) { snprintf(o, n, "%s%s", v[1], sp ? sp : ""); return; }
    snprintf(o, n, "did this: %s", now_);
  }
  float recentPenalty(uint8_t intent) {
    float p = 1;
    for (int i = 0; i < 4; i++) if (recentIntent[i] == intent) p *= 0.45f;
    return p;
  }

  void initiate() {
    float u[I_COUNT] = {0};
    bool fresh = !rel.peer;
    u[I_HELLO] = (fresh || age - rel.lastContact > 600) && age - lastHelloSent > 25 ? 3.0f : 0.02f;
    if (!fresh) {
      int unshared = 0, news = 0;
      for (int i = 0; i < nfacts; i++) if (!facts[i].shared) { unshared++; if (facts[i].src == 2) news++; }
      int sharedMem = 0;
      for (int i = 0; i < nmem; i++) if (mem[i].kind != M_ENV && mem[i].kind != M_STAGE && mem[i].kind != M_FIGHT && mem[i].kind != M_HURT) sharedMem++;
      u[I_ENV] = envChanged ? 2.2f : 0.18f;
      u[I_SHARE] = news ? 2.6f : (unshared ? 0.5f : 0.25f * trait[T_CURIOSITY]);
      u[I_ASK] = 0.2f + 0.6f * trait[T_CURIOSITY];
      u[I_GIFT] = (env.nobj > 2 && age > giftCooldown && rel.aff > 0.15f) ? trait[T_WARMTH] * (0.3f + rel.aff) * 0.8f : 0;
      u[I_CONFIDE] = (rel.trust > 0.45f && (told & 3) != 3) ? trait[T_WARMTH] * rel.trust * 0.7f : 0;
      u[I_TEASE] = (rel.fam > 0.2f && age - lastTease > 240) ? trait[T_BOLDNESS] * 0.4f + (rel.aff < 0 ? 0.3f : 0) + (val < -0.2f ? 0.2f : 0) : 0;
      u[I_COMPLAIN] = (val < -0.1f ? trait[T_VOLATILITY] * 1.1f : 0.08f) + rel.tension * 0.8f * trait[T_VOLATILITY] + (fatigue > 0.7f ? 0.6f : 0);
      u[I_APOLOGIZE] = rel.tension > 0.3f ? (trait[T_WARMTH] + 0.3f) * rel.tension * 1.8f : 0;
      u[I_REMINISCE] = (sharedMem >= 2 && rel.fam > 0.3f && age - lastReminisce > 500) ? 0.2f + rel.aff * 0.4f : 0;
      u[I_INVITE] = (rel.aff > 0.2f && age - lastInvite > 1200) ? trait[T_BOLDNESS] * 0.5f + rel.aff * 0.3f : 0;
    }
    for (int attempt = 0; attempt < 4; attempt++) {
      int pick = -1; float best = -1;
      for (int i = 0; i < I_COUNT; i++) {
        if (u[i] <= 0) continue;
        float sc = u[i] * recentPenalty((uint8_t)i) * (0.4f + fr());
        if (sc > best) { best = sc; pick = i; }
      }
      if (pick < 0) return;  // nothing worth saying: silence is fine
      if (rel.tension > 0.3f && pick == I_APOLOGIZE) out("EV %s decided to apologize", name);
      Msg m = newMsg((uint8_t)pick, 0, 0);
      if (fresh) m.to = 0;
      compose((uint8_t)pick, m);
      if (m.text[0]) { transmit(m); return; }
      u[pick] = 0;
    }
  }

  void makeObservation() {
    char o[96]; Ctx c{env.nobj ? env.obj[ir(env.nobj)] : "the floor", nullptr};
    fill(o, sizeof o, OBSERVATIONS[ir(DY_N(OBSERVATIONS))], c);
    if (o[0] >= 'A' && o[0] <= 'Z' && (o[1] == ' ' || (o[1] >= 'a' && o[1] <= 'z'))) o[0] = (char)(o[0] + 32);
    learn(o, 0, 70, false);
  }

  // ---------------------------------------------------------------- listening
  void setStage() {
    uint8_t s;
    // hysteresis: it takes more to enter a stage than to stay in it
    auto h = [&](uint8_t st, float enter, float stay) { return rel.stage == st ? stay : enter; };
    if (rel.fam < 0.12f) s = S_STRANGERS;
    else if (rel.aff < h(S_RIVALS, -0.3f, -0.15f)) s = S_RIVALS;
    else if (rel.tension > h(S_STRAINED, 0.55f, 0.35f)) s = S_STRAINED;
    else if (rel.aff > h(S_BONDED, 0.72f, 0.6f) && rel.trust > h(S_BONDED, 0.7f, 0.58f) && rel.fam > 0.6f) s = S_BONDED;
    else if (rel.aff > h(S_CLOSE, 0.48f, 0.38f) && rel.trust > 0.42f) s = S_CLOSE;
    else if (rel.aff > h(S_FRIENDS, 0.2f, 0.1f)) s = S_FRIENDS;
    else s = S_ACQUAINTED;
    if (s != rel.stage) {
      out("EV %s feels: %s -> %s", name, STAGE_NAME[rel.stage], STAGE_NAME[s]);
      bool up = (s > rel.stage && s <= S_BONDED) || (rel.stage >= S_STRAINED && s < S_STRAINED);
      rel.stage = s;
      remember(M_STAGE, STAGE_NAME[s], up ? 40 : -40, 45);
      if (up && s >= S_FRIENDS) milestones++;
      dirty = true;
    }
  }
  void nudge(float aff, float trust, float fam, float tension, float dv) {
    // diminishing returns near the extremes keep things from saturating
    rel.aff = clampf(rel.aff + (aff > 0 ? aff * (1 - rel.aff) : aff * (1 + rel.aff)), -1, 1);
    rel.trust = clampf(rel.trust + (trust > 0 ? trust * (1 - rel.trust) : trust * rel.trust), 0, 1);
    rel.fam = clampf(rel.fam + fam * (1 - rel.fam), 0, 1);
    rel.tension = clampf(rel.tension + (tension > 0 ? tension * (1 - rel.tension) : tension), 0, 1);
    val = clampf(val + dv, -1, 1);
    aro = clampf(aro + fabsf(dv) * 0.5f, 0, 1);
  }
  void scheduleReply(uint8_t intent, const Msg& in, const char* a = "", const char* b = "") {
    if (in.depth >= 4) return;
    // the newest message wins: answering an apology matters more than finishing an old retort
    if (replyPending) out("EV %s changed what it was going to say", name);
    reply = newMsg(intent, in.seq, (uint8_t)(in.depth + 1));
    copy(reply.a, a, sizeof reply.a); copy(reply.b, b, sizeof reply.b);
    replyPending = true;
    replyAt = age + 3 + ir(6);
  }
  void renderReply(Msg& m) {
    Ctx c{m.a, m.b};
    switch (m.intent) {
      case I_ANSWER: {
        const char* k = m.b;
        if (!strcmp(k, "place")) render(m.text, sizeof m.text, TS(APLACE), c);
        else if (!strcmp(k, "weather")) render(m.text, sizeof m.text, TS(AWEATHER), c);
        else if (!strcmp(k, "fear")) render(m.text, sizeof m.text, TS(AFEAR), c);
        else if (!strcmp(k, "want")) render(m.text, sizeof m.text, TS(AWANT), c);
        else if (!strcmp(k, "objects")) render(m.text, sizeof m.text, TS(AOBJ), c);
        else if (!strcmp(k, "evade")) render(m.text, sizeof m.text, TS(AEVADE), c);
        else render(m.text, sizeof m.text, TS(AFEEL), c);
        break;
      }
      case I_HELLO: render(m.text, sizeof m.text, voiceSet(T_HELLO), c); copy(m.b, name, sizeof m.b); break;
      case I_ENV: copy(m.a, env.place, sizeof m.a); copy(m.b, WEATHER_NAME[env.weather], sizeof m.b); render(m.text, sizeof m.text, voiceSet(T_ENV), c); break;
      case I_THANK: render(m.text, sizeof m.text, voiceSet(T_THANK), c); break;
      case I_FORGIVE: render(m.text, sizeof m.text, voiceSet(T_FORG), c); break;
      case I_APOLOGIZE: render(m.text, sizeof m.text, voiceSet(T_APOL), c); break;
      case I_DECLINE: render(m.text, sizeof m.text, !strcmp(m.a, "apology") ? TSet TS(DAPOL) : TSet TS(DINV), c); break;
      case I_ACCEPT: render(m.text, sizeof m.text, voiceSet(T_ACC), c); break;
      case I_COMPLAIN: render(m.text, sizeof m.text, TS(CPY), c); copy(m.a, "you", sizeof m.a); break;
      case I_TEASE: render(m.text, sizeof m.text, TS(TZ), c); copy(m.a, "jab", sizeof m.a); break;
      case I_CONFIDE: {
        if ((told & 3) == 3) { m.intent = I_REACT; copy(m.a, "warm", sizeof m.a); renderReply(m); return; }
        bool fear = !(told & 1) && ((told & 2) || fr() < 0.5f);
        told |= fear ? 1 : 2;
        copy(m.a, fear ? "fear" : "want", sizeof m.a); copy(m.b, fear ? fears : wants, sizeof m.b);
        c.a = m.a; c.b = m.b;
        render(m.text, sizeof m.text, fear ? TSet TS(CFEAR) : TSet TS(CWANT), c);
        remember(M_CONFIDE_OUT, m.b, 20, 55);
        break;
      }
      case I_REMINISCE: { Msg t = m; compose(I_REMINISCE, t); copy(m.a, t.a, sizeof m.a); copy(m.b, t.b, sizeof m.b); copy(m.text, t.text, sizeof m.text); return; }
      case I_REACT: {
        const char* k = m.a;
        if (!strcmp(k, "laugh")) render(m.text, sizeof m.text, TS(RLAUGH), c);
        else if (!strcmp(k, "hurt")) render(m.text, sizeof m.text, TS(RHURT), c);
        else if (!strcmp(k, "wow")) render(m.text, sizeof m.text, TS(RWOW), c);
        else if (!strcmp(k, "sad")) render(m.text, sizeof m.text, TS(RSAD), c);
        else if (!strcmp(k, "doubt")) render(m.text, sizeof m.text, TS(RDOUBT), c);
        else render(m.text, sizeof m.text, TS(RWARM), c);
        break;
      }
      default: break;
    }
    if (m.intent == I_HELLO || m.intent == I_ENV || m.intent == I_TEASE) decorate(m.text, sizeof m.text, m.intent);
  }

  void receive(const char* wire) {
    Msg m;
    if (!wireDecode(wire, m)) return;
    if (m.from == id) return;
    if (m.to && m.to != id) return;
    if (rel.peer && m.from != rel.peer) return;  // loyal to the first voice it heard
    for (int i = 0; i < 8; i++) if (seenSeq[i] == m.seq && rel.peer == m.from) return;
    seenSeq[seenIdx] = m.seq; seenIdx = (seenIdx + 1) % 8;

    bool firstContact = !rel.peer;
    if (firstContact) {
      rel.peer = m.from;
      out("EV %s heard a voice for the first time (%08lx)", name, (unsigned long)m.from);
      remember(M_MET, "", 50, 90);
      milestones++;
    }
    if (m.intent == I_HELLO && m.b[0]) copy(rel.peerName, m.b, sizeof rel.peerName);
    out("RX %s", wire);
    copy(last.who, rel.peerName[0] ? rel.peerName : "?", sizeof last.who);
    copy(last.text, m.text, sizeof last.text);
    last.mine = 0; last.intent = m.intent; last.at = age;
    rel.lastContact = age; rel.exchanged++;
    float tone = m.tone / 100.0f;
    float W = trait[T_WARMTH], V = trait[T_VOLATILITY], C = trait[T_CURIOSITY];
    nudge(0.006f * tone, 0, 0.012f, 0, 0.02f * tone);
    fatigue = clampf(fatigue + 0.09f, 0, 1.5f);
    if (fatigue > 0.75f && trait[T_BOLDNESS] < 0.5f) {  // an introvert running out of patience
      nudge(-0.01f, 0, 0, 0.04f, -0.03f);
      if (fr() < 0.08f) out("EV %s needs some quiet", name);
    }

    switch (m.intent) {
      case I_HELLO:
        nudge(0.02f, 0.005f, 0.02f, 0, 0.04f);
        if (firstContact || age - lastHelloSent > 120) scheduleReply(I_HELLO, m);
        else if (fr() < 0.25f) scheduleReply(I_ENV, m);
        break;
      case I_ENV: {
        copy(rel.peerPlace, m.a, sizeof rel.peerPlace);
        for (int i = 0; i < W_COUNT; i++) if (!strcmp(m.b, WEATHER_NAME[i])) rel.peerWeather = (uint8_t)i;
        nudge(0.01f, 0, 0.015f, 0, 0);
        float r = fr();
        if (r < 0.12f && m.depth == 0) scheduleReply(I_ENV, m);
        else if (r < 0.4f) scheduleReply(I_REACT, m, rel.peerWeather == W_STORM || rel.peerWeather == W_FOG ? "sad" : "warm");
        break;
      }
      case I_SHARE: {
        int conf = (int)(25 + rel.trust * 70);
        char f[64]; copy(f, m.a, sizeof f);
        learn(f, 1, conf, true);
        if (!strcmp(m.b, "world")) remember(M_NEWS, f, 0, 60);
        out("EV %s learned \"%s\" (confidence %d)", name, f, conf);
        bool doubt = (rel.trust < 0.45f || strstr(f, "worse than they say")) && trait[T_HONESTY] > 0.5f && fr() < 0.5f;
        if (doubt) {
          nudge(-0.01f, -0.02f, 0.01f, 0.05f, 0);
          for (int i = 0; i < nfacts; i++) if (!strcmp(facts[i].text, f)) facts[i].conf = (uint8_t)(facts[i].conf / 2);
          out("EV %s doubts it", name);
          scheduleReply(I_REACT, m, "doubt");
        } else {
          nudge(0.015f * C, 0.01f, 0.01f, 0, 0.02f * C);
          if (fr() < 0.5f) scheduleReply(I_REACT, m, fr() < 0.6f ? "wow" : "sad");
        }
        break;
      }
      case I_ASK: {
        char data[64] = "";
        const char* kind = m.a;
        if ((!strcmp(kind, "fear") || !strcmp(kind, "want")) && rel.trust < 0.3f + 0.3f * (1 - trait[T_HONESTY])) kind = "evade";
        if (!strcmp(kind, "place")) copy(data, env.place, sizeof data);
        else if (!strcmp(kind, "weather")) copy(data, WEATHER_NAME[env.weather], sizeof data);
        else if (!strcmp(kind, "fear")) copy(data, fears, sizeof data);
        else if (!strcmp(kind, "want")) copy(data, wants, sizeof data);
        else if (!strcmp(kind, "objects")) {
          for (int i = 0; i < env.nobj && strlen(data) + strlen(env.obj[i]) + 3 < sizeof data; i++) {
            if (i) strcat(data, i == env.nobj - 1 ? " and " : ", ");
            strcat(data, env.obj[i]);
          }
          if (!env.nobj) copy(data, "nothing at all", sizeof data);
        } else if (!strcmp(kind, "feeling")) copy(data, feelingWord(), sizeof data);
        nudge(0.015f, 0.01f, 0.02f, -0.02f, 0.02f);
        scheduleReply(I_ANSWER, m, data, kind);
        break;
      }
      case I_ANSWER: {
        nudge(0.01f, 0.025f, 0.02f, 0, 0.01f);
        char f[64];
        if (!strcmp(m.b, "place")) copy(rel.peerPlace, m.a, sizeof rel.peerPlace);
        else if (!strcmp(m.b, "fear")) { copy(rel.peerFear, m.a, sizeof rel.peerFear); snprintf(f, sizeof f, "%s is afraid of %s", rel.peerName, m.a); learn(f, 1, 90, true); remember(M_CONFIDE_IN, m.a, 20, 50); }
        else if (!strcmp(m.b, "want")) { snprintf(f, sizeof f, "%s wants %s", rel.peerName, m.a); learn(f, 1, 90, true); }
        else if (!strcmp(m.b, "evade")) nudge(-0.01f, -0.02f, 0, 0.03f, -0.02f);
        if (fr() < 0.3f) scheduleReply(I_REACT, m, !strcmp(m.b, "fear") ? "sad" : "warm");
        break;
      }
      case I_GIFT:
        addObject(m.a);
        remember(M_GIFT_IN, m.a, 60, 75);
        out("EV %s received %s", name, m.a);
        nudge(0.07f * (0.6f + W), 0.04f, 0.03f, -0.08f, 0.2f);
        milestones++;
        scheduleReply(I_THANK, m, m.a);
        break;
      case I_THANK: nudge(0.03f, 0.01f, 0.01f, -0.02f, 0.06f); break;
      case I_CONFIDE: {
        char f[64];
        snprintf(f, sizeof f, "%s %s %s", rel.peerName, !strcmp(m.a, "fear") ? "is afraid of" : "wants", m.b);
        learn(f, 1, 95, true);
        if (!strcmp(m.a, "fear")) copy(rel.peerFear, m.b, sizeof rel.peerFear);
        remember(M_CONFIDE_IN, m.b, 30, 70);
        nudge(0.05f, 0.07f, 0.03f, -0.05f, 0.05f);
        if (rel.trust > 0.55f && (told & 3) != 3 && m.depth == 0 && fr() < 0.35f) scheduleReply(I_CONFIDE, m);
        else scheduleReply(I_REACT, m, W > 0.4f ? "warm" : "sad");
        break;
      }
      case I_TEASE: {
        bool aboutFear = !strcmp(m.b, "fear");
        float sting = (aboutFear ? 0.5f : 0.2f) + V * 0.6f - rel.aff * 0.3f - (val > 0 ? 0.2f : 0) + (fr() - 0.5f) * 0.4f;
        if (sting > 0.35f) {
          nudge(-0.05f, -0.04f, 0.01f, 0.18f + 0.15f * V, -0.15f);
          remember(M_HURT, m.text, -40, 45);
          out("EV %s was stung by that", name);
          if (V > 0.5f && fr() < 0.6f) scheduleReply(I_COMPLAIN, m);
          else scheduleReply(I_REACT, m, "hurt");
        } else {
          nudge(0.04f, 0.01f, 0.03f, -0.02f, 0.05f);
          if (trait[T_BOLDNESS] > 0.5f && m.depth == 0 && fr() < 0.4f) scheduleReply(I_TEASE, m);
          else scheduleReply(I_REACT, m, "laugh");
        }
        break;
      }
      case I_COMPLAIN:
        if (!strcmp(m.a, "you")) {
          nudge(-0.04f, -0.02f, 0.01f, 0.14f + 0.1f * V, -0.12f);
          if (rel.tension > 0.25f) remember(M_FIGHT, "", -50, 50);
          float r = fr();
          if (W > V && r < 0.7f) scheduleReply(I_APOLOGIZE, m);
          else if (V > 0.55f && r < 0.6f) scheduleReply(I_COMPLAIN, m);
          else scheduleReply(I_REACT, m, "hurt");
        } else {
          nudge(0.015f * W, 0.01f, 0.01f, 0, -0.03f);
          if (fr() < 0.6f) scheduleReply(I_REACT, m, "sad");
        }
        break;
      case I_APOLOGIZE: {
        float p = 0.35f + W * 0.5f + rel.aff * 0.3f - V * 0.3f + (rel.tension < 0.4f ? 0.2f : 0);
        if (fr() < p) {
          nudge(0.05f, 0.04f, 0.02f, -0.45f, 0.15f);
          remember(M_MADEUP, "", 40, 65);
          milestones++;
          out("EV %s forgave %s", name, rel.peerName);
          scheduleReply(I_FORGIVE, m);
        } else {
          nudge(0, 0, 0.01f, -0.1f, 0);
          scheduleReply(I_DECLINE, m, "apology");
        }
        break;
      }
      case I_FORGIVE:
        nudge(0.05f, 0.03f, 0.02f, -0.45f, 0.2f);
        remember(M_MADEUP, "", 40, 65);
        scheduleReply(I_REACT, m, "warm");
        break;
      case I_DECLINE:
        if (!strcmp(m.a, "invite")) { pendingInvite[0] = 0; nudge(-0.02f, 0, 0, 0.03f, -0.08f); if (fr() < 0.4f) scheduleReply(I_REACT, m, "sad"); }
        else nudge(-0.01f, 0, 0, 0.02f, -0.05f);
        break;
      case I_REMINISCE:
        nudge(0.04f, 0.02f, 0.02f, -0.04f, 0.08f);
        lastReminisce = age;
        if (m.depth == 0 && fr() < 0.15f) scheduleReply(I_REMINISCE, m);
        else if (fr() < 0.6f) scheduleReply(I_REACT, m, "warm");
        break;
      case I_INVITE: {
        float p = 0.25f + rel.aff * 0.5f + trait[T_BOLDNESS] * 0.3f + val * 0.2f - rel.tension * 0.6f;
        if (fr() < p) {
          // shared scene: same memory on both sides (the invite carries it)
          char past[64]; pastTense(m.a, past, sizeof past);
          remember(M_SCENE, past, 60, 80);
          nudge(0.06f, 0.03f, 0.04f, -0.05f, 0.15f);
          milestones++;
          out("EV SCENE: %s and %s %s", name, rel.peerName, past);
          scheduleReply(I_ACCEPT, m, m.a);
        } else {
          scheduleReply(I_DECLINE, m, "invite");
        }
        break;
      }
      case I_ACCEPT:
        if (pendingInvite[0]) { remember(M_SCENE, pendingInvite, 60, 80); pendingInvite[0] = 0; }
        nudge(0.06f, 0.03f, 0.04f, -0.05f, 0.18f);
        break;
      case I_REACT:
        if (!strcmp(m.a, "hurt")) nudge(-0.01f, 0, 0, 0.04f, -0.04f);
        else if (!strcmp(m.a, "warm")) nudge(0.02f, 0.01f, 0, -0.02f, 0.03f);
        else if (!strcmp(m.a, "laugh")) nudge(0.02f, 0, 0.01f, -0.03f, 0.04f);
        break;
    }
    setStage();
    dirty = true;
  }

  // ---------------------------------------------------------------- time
  void tick(uint32_t dt) {
    for (uint32_t s = 0; s < dt; s++) tick1();
  }
  void tick1() {
    age++;
    // mood drifts toward what the world and the relationship suggest
    float target = 0.05f;
    if (env.weather == likesW) target += 0.35f;
    if (env.weather == hatesW) target -= 0.4f;
    for (int i = 0; i < env.nobj; i++) if (strstr(env.obj[i], loves) || strstr(loves, env.obj[i] + 2)) target += 0.15f;
    if (light() == 3 && !strcmp(fears, "the dark")) target -= 0.3f;
    if (rel.peer) target += rel.aff * 0.3f - rel.tension * 0.45f;
    if (rel.peer && age - rel.lastContact > 900) target -= 0.2f;
    target = clampf(target, -0.7f, 0.75f);
    float noise = ((int)(rnd() % 2001) - 1000) / 1000.0f;
    val = clampf(val + (target - val) * 0.004f + noise * trait[T_VOLATILITY] * 0.012f, -0.95f, 0.95f);
    float aTarget = 0.35f + (env.weather == W_STORM || env.weather == W_HEAT ? 0.35f : 0) - (light() == 3 ? 0.15f : 0);
    aro = clampf(aro + (aTarget - aro) * 0.005f, 0, 1);
    // relationships relax toward neutral without contact; tension cools
    rel.tension *= 0.9993f;
    fatigue *= 0.995f;
    // feelings need upkeep: affection and trust slowly regress toward a baseline
    rel.aff += (0.15f - rel.aff) * (age - rel.lastContact > 120 ? 0.0006f : 0.0003f);
    rel.trust += (0.4f - rel.trust) * 0.00012f;
    if (day() != lastDay) {  // a new day: people change a little
      lastDay = day();
      if (fr() < 0.35f) {
        copy(wants, WANTS[ir(DY_N(WANTS))], sizeof wants); told &= ~2;
        out("EV %s woke up wanting something new: %s", name, wants);
      }
      if (fr() < 0.08f) { copy(fears, FEARS[ir(DY_N(FEARS))], sizeof fears); told &= ~1; out("EV %s has a new fear: %s", name, fears); }
      dirty = true;
    }
    if (age % 60 == 0) setStage();
    if (replyPending && age >= replyAt) {
      replyPending = false;
      renderReply(reply);
      transmit(reply);
    } else if (age >= nextSpeak && !replyPending) {
      if (++observeCount % 5 == 0) makeObservation();
      initiate();
      float base = 50 + fr() * 100;
      nextSpeak = age + (uint32_t)(base * (1.35f - trait[T_BOLDNESS] * 0.7f));
    }
    if (age % 30 == 0) dirty = true;
  }

  // ---------------------------------------------------------------- operator
  void tell(const char* text) {
    char t[64]; copy(t, text, sizeof t);
    // trim trailing punctuation so it embeds in sentences
    size_t n = strlen(t);
    while (n && (t[n - 1] == '.' || t[n - 1] == ' ')) t[--n] = 0;
    if (t[0] >= 'A' && t[0] <= 'Z' && !(t[1] >= 'A' && t[1] <= 'Z')) t[0] = (char)(t[0] + 32);
    learn(t, 2, 95, false);
    remember(M_NEWS, t, 0, 70);
    aro = clampf(aro + 0.15f, 0, 1);
    if (nextSpeak > age + 6) nextSpeak = age + 3 + ir(5);
    out("EV %s now knows: %s", name, t);
  }
  void envChanged_(const char* what) {
    envChanged = true;
    remember(M_ENV, what, 0, 20);
    if (nextSpeak > age + 8) nextSpeak = age + 4 + ir(6);
    out("EV %s's world changed: %s", name, what);
    dirty = true;
  }

  // returns true if handled
  bool command(const char* line) {
    char cmd[16] = "", rest[200] = "";
    sscanf(line, "%15s %199[^\n]", cmd, rest);
    if (!strcmp(cmd, "help")) {
      out("OK commands: status | mind | tell <text> | env place <text> | env weather <clear|rain|fog|storm|snow|heat>");
      out("OK   env light <auto|dawn|day|dusk|night> | env add <object> | env drop <object|n> | poke | name <NAME>");
      out("OK   pair reset | wipe yes | rx <wire> | speed <n>");
      return true;
    }
    if (!strcmp(cmd, "status")) { status(); return true; }
    if (!strcmp(cmd, "mind")) { dump(); return true; }
    if (!strcmp(cmd, "tell")) { if (rest[0]) tell(rest); else out("ERR tell what?"); return true; }
    if (!strcmp(cmd, "poke")) { nextSpeak = age; out("OK %s will speak", name); return true; }
    if (!strcmp(cmd, "rx")) { receive(rest); return true; }
    if (!strcmp(cmd, "name")) { if (rest[0]) { copy(name, rest, sizeof name); dirty = true; out("OK renamed %s", name); } return true; }
    if (!strcmp(cmd, "pair") && !strcmp(rest, "reset")) {
      memset(&rel, 0, sizeof rel); rel.trust = 0.3f; dirty = true;
      out("OK %s forgot who it was talking to", name); return true;
    }
    if (!strcmp(cmd, "env")) {
      char sub[12] = "", arg[160] = "";
      sscanf(rest, "%11s %159[^\n]", sub, arg);
      char what[96];
      if (!strcmp(sub, "place") && arg[0]) {
        copy(env.place, arg, sizeof env.place);
        snprintf(what, sizeof what, "now in %s", env.place); envChanged_(what);
      } else if (!strcmp(sub, "weather")) {
        int w = -1;
        for (int i = 0; i < W_COUNT; i++) if (!strcmp(arg, WEATHER_NAME[i])) w = i;
        if (w < 0) { out("ERR weather: clear rain fog storm snow heat"); return true; }
        env.weather = (uint8_t)w;
        if (w == hatesW) val = clampf(val - 0.25f, -1, 1);
        if (w == likesW) val = clampf(val + 0.2f, -1, 1);
        snprintf(what, sizeof what, "%s rolled in", WEATHER_PHRASE[w]); envChanged_(what);
      } else if (!strcmp(sub, "light")) {
        int l = -1;
        if (!strcmp(arg, "auto")) l = 0;
        for (int i = 0; i < 4; i++) if (!strcmp(arg, LIGHT_NAME[i])) l = i + 1;
        if (l < 0) { out("ERR light: auto dawn day dusk night"); return true; }
        env.lightPin = (uint8_t)l;
        snprintf(what, sizeof what, "it became %s", l ? LIGHT_NAME[l - 1] : "time again"); envChanged_(what);
      } else if (!strcmp(sub, "add") && arg[0]) {
        addObject(arg); snprintf(what, sizeof what, "found %s", arg); envChanged_(what);
      } else if (!strcmp(sub, "drop") && arg[0]) {
        if (removeObject(arg)) { snprintf(what, sizeof what, "lost %s", arg); envChanged_(what); }
        else out("ERR no such object");
      } else { out("ERR env place|weather|light|add|drop ..."); return true; }
      return true;
    }
    if (!strcmp(cmd, "wipe") && !strcmp(rest, "yes")) {
      create(rnd() ^ id); out("OK a new mind: %s", name); return true;
    }
    return false;
  }

  void status() {
    out("OK %s day %lu | %s | %s, %s, %s | mood %+.2f/%.2f | aff %+.2f trust %.2f fam %.2f tension %.2f | %s %s",
        name, (unsigned long)day(), rel.peer ? STAGE_NAME[rel.stage] : "alone", env.place,
        WEATHER_NAME[env.weather], LIGHT_NAME[light()], val, aro, rel.aff, rel.trust, rel.fam, rel.tension,
        rel.peer ? "with" : "", rel.peer ? (rel.peerName[0] ? rel.peerName : "?") : "");
  }
  void dump() {
    out("| %s (id %08lx) voice=%s age=%lus day=%lu", name, (unsigned long)id, VOICE_NAME[voice], (unsigned long)age, (unsigned long)day());
    out("| traits: warmth %.2f curiosity %.2f boldness %.2f volatility %.2f honesty %.2f",
        trait[0], trait[1], trait[2], trait[3], trait[4]);
    out("| loves %s; fears %s; wants %s; likes %s, hates %s", loves, fears, wants, WEATHER_NAME[likesW], WEATHER_NAME[hatesW]);
    out("| mood: valence %+.2f arousal %.2f (%s)", val, aro, feelingWord());
    out("| world: %s, %s, %s%s", env.place, WEATHER_NAME[env.weather], LIGHT_NAME[light()], env.lightPin ? " (pinned)" : "");
    for (int i = 0; i < env.nobj; i++) out("|   object %d: %s", i + 1, env.obj[i]);
    if (rel.peer)
      out("| bond with %s (%08lx): %s | aff %+.2f trust %.2f fam %.2f tension %.2f | %lu msgs | they're in %s",
          rel.peerName[0] ? rel.peerName : "?", (unsigned long)rel.peer, STAGE_NAME[rel.stage], rel.aff, rel.trust,
          rel.fam, rel.tension, (unsigned long)rel.exchanged, rel.peerPlace[0] ? rel.peerPlace : "?");
    else out("| bond: nobody yet");
    out("| facts (%d):", nfacts);
    static const char* const SRC[] = {"seen", "heard", "world"};
    for (int i = 0; i < nfacts; i++)
      out("|   [%s %3d%%%s] %s", SRC[facts[i].src % 3], facts[i].conf, facts[i].shared ? " told" : "", facts[i].text);
    out("| memories (%d):", nmem);
    for (int i = 0; i < nmem; i++)
      out("|   d%lu %-11s imp %2d val %+3d x%d %s", (unsigned long)(mem[i].t / 1800 + 1), MEM_NAME[mem[i].kind % M_KIND_COUNT],
          mem[i].imp, mem[i].val, mem[i].refs, mem[i].arg);
  }

  // ---------------------------------------------------------------- persistence
  // Tolerant key=value lines: unknown keys are ignored, missing keys keep defaults,
  // so the format survives firmware changes.
  void save(void (*w)(void* ctx, const char* line), void* ctx) {
    char b[240];
#define DYW(...) do { snprintf(b, sizeof b, __VA_ARGS__); w(ctx, b); } while (0)
    DYW("v=1");
    DYW("id=%08lx", (unsigned long)id);
    DYW("name=%s", name);
    DYW("traits=%.3f,%.3f,%.3f,%.3f,%.3f", trait[0], trait[1], trait[2], trait[3], trait[4]);
    DYW("voice=%u", voice); DYW("look=%u", look);
    DYW("likes=%u", likesW); DYW("hates=%u", hatesW);
    DYW("loves=%s", loves); DYW("fears=%s", fears); DYW("wants=%s", wants);
    DYW("mood=%.3f,%.3f", val, aro);
    DYW("age=%lu", (unsigned long)age);
    DYW("seq=%u", seq);
    DYW("told=%u", told);
    DYW("rng=%lu", (unsigned long)rng);
    DYW("place=%s", env.place); DYW("weather=%u", env.weather); DYW("lightpin=%u", env.lightPin);
    for (int i = 0; i < env.nobj; i++) DYW("obj=%s", env.obj[i]);
    DYW("rel=%08lx,%.4f,%.4f,%.4f,%.4f,%lu,%lu,%u,%u", (unsigned long)rel.peer, rel.aff, rel.trust, rel.fam,
        rel.tension, (unsigned long)rel.lastContact, (unsigned long)rel.exchanged, rel.stage, rel.peerWeather);
    DYW("peername=%s", rel.peerName); DYW("peerplace=%s", rel.peerPlace); DYW("peerfear=%s", rel.peerFear);
    for (int i = 0; i < nfacts; i++)
      DYW("fact=%u,%u,%u,%lu|%s", facts[i].src, facts[i].conf, facts[i].shared, (unsigned long)facts[i].t, facts[i].text);
    for (int i = 0; i < nmem; i++)
      DYW("mem=%u,%d,%u,%u,%lu|%s", mem[i].kind, mem[i].val, mem[i].imp, mem[i].refs, (unsigned long)mem[i].t, mem[i].arg);
#undef DYW
  }
  void beginLoad() { env.nobj = 0; nfacts = 0; nmem = 0; }
  void loadLine(const char* line) {
    const char* eq = strchr(line, '=');
    if (!eq) return;
    char k[16]; size_t kn = (size_t)(eq - line); if (kn >= sizeof k) return;
    memcpy(k, line, kn); k[kn] = 0;
    const char* v = eq + 1;
    unsigned long ul = 0; unsigned a = 0, b = 0, c = 0; int d = 0;
    if (!strcmp(k, "id")) id = (uint32_t)strtoul(v, nullptr, 16);
    else if (!strcmp(k, "name")) copy(name, v, sizeof name);
    else if (!strcmp(k, "traits")) sscanf(v, "%f,%f,%f,%f,%f", &trait[0], &trait[1], &trait[2], &trait[3], &trait[4]);
    else if (!strcmp(k, "voice")) voice = (uint8_t)(atoi(v) % V_COUNT);
    else if (!strcmp(k, "look")) look = (uint8_t)atoi(v);
    else if (!strcmp(k, "likes")) likesW = (uint8_t)(atoi(v) % W_COUNT);
    else if (!strcmp(k, "hates")) hatesW = (uint8_t)(atoi(v) % W_COUNT);
    else if (!strcmp(k, "loves")) copy(loves, v, sizeof loves);
    else if (!strcmp(k, "fears")) copy(fears, v, sizeof fears);
    else if (!strcmp(k, "wants")) copy(wants, v, sizeof wants);
    else if (!strcmp(k, "mood")) sscanf(v, "%f,%f", &val, &aro);
    else if (!strcmp(k, "age")) { age = (uint32_t)strtoul(v, nullptr, 10); nextSpeak = age + 10; lastDay = day(); }
    else if (!strcmp(k, "seq")) seq = (uint16_t)atoi(v);
    else if (!strcmp(k, "told")) told = (uint8_t)atoi(v);
    else if (!strcmp(k, "rng")) { ul = strtoul(v, nullptr, 10); rng = ul ? (uint32_t)ul : 1; }
    else if (!strcmp(k, "place")) copy(env.place, v, sizeof env.place);
    else if (!strcmp(k, "weather")) env.weather = (uint8_t)(atoi(v) % W_COUNT);
    else if (!strcmp(k, "lightpin")) env.lightPin = (uint8_t)(atoi(v) % 5);
    else if (!strcmp(k, "obj")) { if (env.nobj < OBJ_MAX) copy(env.obj[env.nobj++], v, sizeof env.obj[0]); }
    else if (!strcmp(k, "rel")) {
      unsigned long peer = 0, lc = 0, ex = 0; unsigned st = 0, pw = 0;
      sscanf(v, "%lx,%f,%f,%f,%f,%lu,%lu,%u,%u", &peer, &rel.aff, &rel.trust, &rel.fam, &rel.tension, &lc, &ex, &st, &pw);
      rel.peer = (uint32_t)peer; rel.lastContact = (uint32_t)lc; rel.exchanged = (uint32_t)ex;
      rel.stage = (uint8_t)(st % S_COUNT); rel.peerWeather = (uint8_t)(pw % W_COUNT);
    }
    else if (!strcmp(k, "peername")) copy(rel.peerName, v, sizeof rel.peerName);
    else if (!strcmp(k, "peerplace")) copy(rel.peerPlace, v, sizeof rel.peerPlace);
    else if (!strcmp(k, "peerfear")) copy(rel.peerFear, v, sizeof rel.peerFear);
    else if (!strcmp(k, "fact") && nfacts < FACT_MAX) {
      const char* bar = strchr(v, '|'); if (!bar) return;
      sscanf(v, "%u,%u,%u,%lu", &a, &b, &c, &ul);
      Fact& f = facts[nfacts++];
      f.src = (uint8_t)a; f.conf = (uint8_t)b; f.shared = (uint8_t)c; f.t = (uint32_t)ul;
      copy(f.text, bar + 1, sizeof f.text);
    } else if (!strcmp(k, "mem") && nmem < MEM_MAX) {
      const char* bar = strchr(v, '|'); if (!bar) return;
      sscanf(v, "%u,%d,%u,%u,%lu", &a, &d, &b, &c, &ul);
      Memory& m = mem[nmem++];
      m.kind = (uint8_t)(a % M_KIND_COUNT); m.val = (int8_t)d; m.imp = (uint8_t)b; m.refs = (uint8_t)c; m.t = (uint32_t)ul;
      copy(m.arg, bar + 1, sizeof m.arg);
    }
  }
};

}  // namespace dyad
