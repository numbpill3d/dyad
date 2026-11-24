#!/usr/bin/env python3
"""05 / DYAD inspector — watch and steer the personalities living on your dongles.

    dyad.py                 live console for every connected dongle
    dyad.py --ghost         if only one dongle is connected, give it a laptop-side partner
    dyad.py --raw           also show the structured message fields
    dyad.py --port DEV      use only this serial port
    dyad.py log [N]         print the last N interaction-log lines from each dongle
    dyad.py sim SECONDS     run two personalities in fast simulated time (no hardware)

Console commands are "<who> <command>", where <who> is a name prefix, a number, or "all":
    inve status | veeth mind | all tell a comet is passing over the city
    1 env weather snow | 2 env place a sunken ballroom | inve env add a brass bell
    veeth env drop 2 | inve poke | all speed 10 | inve log 40 | help | /raw | quit
"""
import os, subprocess, sys, threading, time, glob, shlex

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("needs pyserial:  pip install pyserial   (or: sudo pacman -S python-pyserial)")

HERE = os.path.dirname(os.path.abspath(__file__))
GHOST_BIN = os.path.join(HERE, "..", "build", "dyad_ghost")
GHOST_MIND = os.path.expanduser("~/.local/share/dyad/ghost.txt")

C = dict(dim="\033[2m", b="\033[1m", r="\033[0m", cyan="\033[36m", mag="\033[35m",
         yel="\033[33m", grn="\033[32m", red="\033[31m", blu="\033[34m")
if not sys.stdout.isatty():
    C = {k: "" for k in C}
COLORS = [C["cyan"], C["mag"], C["yel"], C["grn"]]
INTENT_COL = {"TEASE": C["yel"], "COMPLAIN": C["red"], "APOLOGIZE": C["blu"], "FORGIVE": C["grn"],
              "GIFT": C["grn"], "CONFIDE": C["mag"], "INVITE": C["grn"], "ACCEPT": C["grn"]}
lock = threading.Lock()
raw = "--raw" in sys.argv


def say(s):
    with lock:
        print(s, flush=True)


class Endpoint:
    def __init__(self, label, idx):
        self.label, self.idx, self.name, self.color = label, idx, label, COLORS[idx % len(COLORS)]
        self.peers = []

    def write(self, line):
        raise NotImplementedError

    def handle(self, line):
        line = line.rstrip("\r\n")
        if not line:
            return
        if line.startswith("OK ") and " day " in line and self.name == self.label:
            self.name = line.split()[1]
        if line.startswith("TX "):
            wire = line[3:]
            self.show_msg(wire)
            for p in self.peers:  # bridge to laptop-side partners (dongles also hear each other by radio)
                p.write("rx " + wire)
        elif line.startswith("RX "):
            pass  # the sender already printed it
        elif line.startswith("EV "):
            if line.startswith("EV boot") or "woke up" in line or "was born" in line:
                parts = line.split()
                if len(parts) > 2 and parts[1] not in ("boot:", "ghost"):
                    self.name = parts[1]
                elif parts[1] == "ghost":
                    self.name = parts[2]
            say(f"{C['dim']}        · {line[3:]}{C['r']}")
        elif line.startswith("[iris]"):
            say(f"{C['dim']}        ✦ iridescence on {self.name}{C['r']}")
        elif line.startswith("L "):
            say(f"{C['dim']}{self.name}:{C['r']} {line[2:]}")
        else:
            say(f"{self.color}{self.name:>8}{C['r']} {line}")

    def show_msg(self, wire):
        f = wire.split("|")
        if len(f) < 11:
            return
        intent, tone, a, b, text = f[6], f[7], f[8], f[9], f[10]
        ic = INTENT_COL.get(intent, C["dim"])
        t = time.strftime("%H:%M:%S")
        say(f"{C['dim']}{t}{C['r']} {self.color}{C['b']}{self.name:>8}{C['r']} {ic}{intent:<9}{C['r']} {text}")
        if raw:
            say(f"{C['dim']}                  seq={f[3]} ref={f[4]} depth={f[5]} tone={tone} a=\"{a}\" b=\"{b}\"{C['r']}")


class Dongle(Endpoint):
    def __init__(self, port, idx):
        super().__init__(os.path.basename(port), idx)
        self.port = port
        self.s = serial.Serial(port, 115200, timeout=0.2)
        threading.Thread(target=self.reader, daemon=True).start()

    def reader(self):
        buf = b""
        while True:
            try:
                buf += self.s.read(512)
            except serial.SerialException:
                say(f"{C['red']}{self.name} disconnected{C['r']}")
                return
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                self.handle(line.decode(errors="replace"))

    def write(self, line):
        self.s.write((line + "\n").encode())


class Ghost(Endpoint):
    def __init__(self, idx):
        super().__init__("ghost", idx)
        if not os.path.exists(GHOST_BIN):
            sys.exit("ghost not built: run  make -C " + os.path.join(HERE, ".."))
        os.makedirs(os.path.dirname(GHOST_MIND), exist_ok=True)
        self.p = subprocess.Popen([GHOST_BIN, "--ghost", GHOST_MIND], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        threading.Thread(target=self.reader, daemon=True).start()

    def reader(self):
        for line in self.p.stdout:
            self.handle(line)

    def write(self, line):
        try:
            self.p.stdin.write(line + "\n"); self.p.stdin.flush()
        except BrokenPipeError:
            pass


def find_ports():
    ports = [p.device for p in list_ports.comports() if p.vid == 0x303A]
    return sorted(ports) or sorted(glob.glob("/dev/ttyACM*"))


def resolve(eps, who):
    if who == "all":
        return eps
    if who.isdigit() and 1 <= int(who) <= len(eps):
        return [eps[int(who) - 1]]
    return [e for e in eps if e.name.lower().startswith(who.lower())]


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if args and args[0] == "sim":
        if not os.path.exists(GHOST_BIN):
            sys.exit("build first: make -C " + os.path.join(HERE, ".."))
        os.execv(GHOST_BIN, [GHOST_BIN, "--pair", args[1] if len(args) > 1 else "7200"] + args[2:])

    ports = find_ports()
    if "--port" in sys.argv:
        ports = [sys.argv[sys.argv.index("--port") + 1]]
        args = [a for a in args if a != ports[0]]
    eps = [Dongle(p, i) for i, p in enumerate(ports)]
    if "--ghost" in sys.argv:
        g = Ghost(len(eps))
        for d in eps:
            d.peers.append(g); g.peers.append(d)
        eps.append(g)
    if not eps:
        sys.exit("no dongles found (and no --ghost)")
    time.sleep(0.6)
    for e in eps:
        e.write("status")
    time.sleep(0.8)

    if args and args[0] == "log":
        for e in eps:
            if isinstance(e, Dongle):
                e.write(f"log {args[1] if len(args) > 1 else 40}")
        time.sleep(2.5)
        return

    say(f"{C['b']}05 / DYAD{C['r']} — " + ", ".join(f"{i+1}:{e.name}" for i, e in enumerate(eps)) +
        f"   {C['dim']}(type help){C['r']}")
    while True:
        try:
            line = input()
        except (EOFError, KeyboardInterrupt):
            break
        line = line.strip()
        if not line:
            continue
        if line in ("quit", "exit"):
            break
        if line == "help":
            say(__doc__.split("Console commands")[1].rstrip())
            continue
        if line == "/raw":
            global raw
            raw = not raw
            say(f"raw fields {'on' if raw else 'off'}")
            continue
        who, _, cmd = line.partition(" ")
        targets = resolve(eps, who)
        if not targets or not cmd:
            say(f"{C['red']}who? use a name, a number (1..{len(eps)}) or all{C['r']}")
            continue
        for t in targets:
            say(f"{C['dim']}  >>> {t.name}: {cmd}{C['r']}")
            t.write(cmd)
    for e in eps:
        if isinstance(e, Ghost):
            e.p.stdin.close(); e.p.wait(timeout=3)


if __name__ == "__main__":
    main()
