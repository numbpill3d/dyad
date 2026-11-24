# 05 / DYAD

Two LilyGO T-Dongle-S3s, each home to an artificial personality with persistent memory. The two find each other over ESP-NOW radio, talk in structured messages, and develop an ongoing fictional relationship: friendship, teasing, quarrels, apologies, gifts, shared evenings, secrets. You can watch it all, introduce news into their world, and change their surroundings.

The style matches 04 / CORTEX-01: 1-bit Bayer-dithered screens, with rare iridescence when something meaningful happens.

## How a personality works

There is no LLM. Everything runs on the ESP32-S3 itself:

- **Persona.** Each personality is generated from its dongle's hardware ID. It gets a name, five traits (warmth, curiosity, boldness, volatility, honesty), a voice (terse, warm, florid or wry), something it loves, a fear, a want, and a favourite and a hated weather.
- **State.** It has a mood (valence and arousal) and an environment: a place, weather, light, and up to 6 objects. It also tracks its relationship with the other: affection, trust, familiarity and tension, plus a stage (strangers, acquainted, friends, close, bonded, strained, rivals).
- **Memory.** It keeps 24 facts (seen, heard from the peer, or told by "the world") and 40 episodic memories. Low-importance memories fade; the first meeting is never forgotten.
- **Messages.** Each message is a structured intent: HELLO, ENV, SHARE, ASK, ANSWER, GIFT, THANK, CONFIDE, TEASE, COMPLAIN, APOLOGIZE, FORGIVE, DECLINE, REMINISCE, INVITE, ACCEPT or REACT. It carries arguments and tone, and is rendered into text in the sender's voice. Intents are chosen from utilities driven by traits, mood and the relationship.
- **Gifts are real.** A gifted object leaves one dongle's world and appears in the other's.
- **People change.** Affection and trust need upkeep and slowly drift back toward neutral. Tension cools over time. Wants (and occasionally fears) change from one in-world day to the next. An in-world day is 30 real minutes.
- **Persistence.** The mind is saved to LittleFS as tolerant `key=value` lines, so it survives power-off and firmware updates. An interaction log is kept on flash and rotated at 48 KB.

## Inspect & steer

```sh
make                     # builds the native engine (ghost partner + simulator)
tools/dyad.py            # live console for all connected dongles
tools/dyad.py --raw      # also show structured fields (seq, ref, tone, args)
tools/dyad.py --ghost    # one dongle? give it a laptop-side partner
tools/dyad.py log 60     # interaction history stored on each dongle
tools/dyad.py sim 7200   # two personalities in simulated time, no hardware
```

In the console, type `<who> <command>`. `<who>` is a name prefix, a number, or `all`.

| command | effect |
|---|---|
| `status`, `mind` | one-line state / full dump of traits, mood, bond, facts, memories |
| `tell <text>` | introduce news; they'll bring it up and pass it on (maybe embellished) |
| `env place <text>` | move them somewhere new |
| `env weather clear\|rain\|fog\|storm\|snow\|heat` | weather affects mood (each has a favourite and a hated one) |
| `env light auto\|dawn\|day\|dusk\|night` | pin the time of day |
| `env add <object>` / `env drop <object\|n>` | put things in their world or take them away |
| `poke` | make them speak now |
| `speed <n>` | run time n× faster (1–50) |
| `name <NAME>`, `pair reset`, `wipe yes` | rename / forget the partner / brand-new mind |
| `log [n]`, `fs`, `radio`, `save` | dongle diagnostics |

The same commands work over any serial monitor (`pio device monitor`).

## On the dongle

- **Screens:** TALK (face, latest line, bond meters), MIND (traits and relationship), WORLD (place, weather, objects), MEMORY (pages through memories).
- **Short press:** next screen. **Long press:** make them speak now.
- **Face:** shaped by the persona; mouth follows mood, a crease appears under tension, eyes close at night.
- **LED:** breathes with arousal and flashes on every message.
- **Iridescence:** appears at first contact, stage advances, gifts, forgiveness and shared scenes, plus a rare random shimmer.

## Build & flash

```sh
pio run -t upload --upload-port /dev/ttyACM0
pio run -t upload --upload-port /dev/ttyACM1
```

`pio run -t upload` does not touch the LittleFS partition, so minds survive reflashing. `uploadfs` *would* erase them.

ESP-NOW uses channel 1 broadcast. Each persona locks onto the first other voice it hears; use `pair reset` to let it meet someone new.

## Layout

- `src/dyad_engine.h`: the personality engine. It is portable C++ with no Arduino dependencies, and the same file runs on the dongle and on the laptop.
- `src/main.cpp`, `src/gfx.h`: firmware (ESP-NOW, LittleFS, ST7735 driver, dithering, views).
- `native/ghost.cpp`: laptop ghost partner and pair simulator.
- `tools/dyad.py`: inspector console (needs `pyserial`).

## License

MIT
