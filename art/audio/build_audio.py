"""Generates the tells' sound cues (docs/arenas/the-yard/events-and-moving-parts.md section 6;
bible/05 section 11) as 16-bit mono WAV files, from code, with the Python standard library:

    python3 art/audio/build_audio.py [--out art/out/audio]

Each cue is a short synthesis recipe: a buzzing diaphragm through its bell's resonances for the
horns, tones for the siren, filtered noise for hisses and blasts, inharmonic partials and a knock for
the clang; then the Yard's acoustics (its steel walls' slap and a reverb tail) at the cue's level. Unreal imports them as
/Game/Audio/SW_<Name> (game/Tools/unreal/import_audio.py, make import-audio).
"""
from __future__ import annotations

import argparse
import math
import os
import random
import struct
import wave

RATE = 44100


def envelope(t: float, length: float, attack: float = 0.01, release: float = 0.08) -> float:
    if t < attack:
        return t / attack
    if t > length - release:
        return max(0.0, (length - t) / release)
    return 1.0


def horn(freq: float, t: float) -> float:
    """A brassy horn: odd and even harmonics falling off, a slight vibrato."""
    f = freq * (1.0 + 0.004 * math.sin(2 * math.pi * 5.5 * t))
    return sum(math.sin(2 * math.pi * f * k * t) / k ** 1.2 for k in range(1, 9))


class Noise:
    """White noise through a one-pole low-pass, so a hiss and a rumble come from one source."""

    def __init__(self, seed: int, cutoff_hz: float) -> None:
        self.random = random.Random(seed)
        self.alpha = 1.0 - math.exp(-2 * math.pi * cutoff_hz / RATE)
        self.y = 0.0

    def next(self) -> float:
        self.y += self.alpha * (self.random.uniform(-1.0, 1.0) - self.y)
        return self.y


def render(length: float, sample) -> list[float]:
    out = [sample(i / RATE) for i in range(int(length * RATE))]
    peak = max(1e-9, max(abs(v) for v in out))
    return [0.9 * v / peak for v in out]


def yard(dry: list[float], wet: float, tail: float = 1.6, level: float = 0.5) -> list[float]:
    """The Yard's acoustics: the slap off the steel walls (a few early echoes), then a diffuse
    tail (a Schroeder reverb: four combs into two all-passes), mixed under the dry sound, and the
    whole cue brought down to `level` of full scale so nothing arrives at the ear as a raw tone."""
    n = len(dry) + int(tail * RATE)
    x, x1, y1 = [], 0.0, 0.0
    for v in dry + [0.0] * (n - len(dry)):  # A DC blocker first (the horns' buzz is lopsided)
        y1 = v - x1 + 0.995 * y1
        x1 = v
        x.append(y1)
    out = list(x)
    for delay_s, gain in ((0.023, 0.35), (0.041, 0.28), (0.067, 0.2), (0.11, 0.14)):  # Early slaps
        d = int(delay_s * RATE)
        for i in range(d, n):
            out[i] += gain * wet * x[i - d]
    combs = []
    for delay_s in (0.0297, 0.0371, 0.0411, 0.0437):
        d = int(delay_s * RATE)
        g = 10 ** (-3 * delay_s / tail)  # -60 dB over the tail
        y = [0.0] * n
        lp = 0.0
        for i in range(n):
            fb = y[i - d] if i >= d else 0.0
            lp += 0.35 * (fb - lp)  # The steel's highs die first
            y[i] = x[i] + g * lp
        combs.append(y)
    diffuse = [sum(c[i] for c in combs) * 0.25 for i in range(n)]
    for delay_s, g in ((0.005, 0.7), (0.0017, 0.7)):  # All-passes
        d = int(delay_s * RATE)
        y = [0.0] * n
        for i in range(n):
            xd = diffuse[i - d] if i >= d else 0.0
            yd = y[i - d] if i >= d else 0.0
            y[i] = -g * diffuse[i] + xd + g * yd
        diffuse = y
    mixed = [out[i] + wet * 0.6 * diffuse[i] for i in range(n)]
    peak = max(1e-9, max(abs(v) for v in mixed))
    return [level * v / peak for v in mixed]


class Resonator:
    """A two-pole band-pass: the horn's bell and diaphragm cavity, which shape the buzz."""

    def __init__(self, freq: float, q: float) -> None:
        w = 2 * math.pi * freq / RATE
        r = 1.0 - w / (2 * q)
        self.a1, self.a2 = -2 * r * math.cos(w), r * r
        self.gain = 1.0 - r
        self.y1 = self.y2 = 0.0

    def next(self, x: float) -> float:
        y = self.gain * x - self.a1 * self.y1 - self.a2 * self.y2
        self.y2, self.y1 = self.y1, y
        return y


def klaxon(freqs: list[float], length: float = 1.6):
    """An electromechanical horn: a diaphragm slapped against its pole piece (a buzz rich in odd
    and even partials, its pitch sagging as the coil warms and fluttering), coloured by the bell's
    two resonances and a little rattle noise, coming up to pitch as the diaphragm gets going."""
    phase = [0.0]
    bells = [Resonator(1250.0, 4.0), Resonator(2900.0, 6.0)]
    rattle = Noise(11, 3000.0)
    flutter = random.Random(12)
    wobble = [0.0]

    def sample(t: float) -> float:
        step = int(t / (length / len(freqs)))
        f = freqs[min(step, len(freqs) - 1)]
        f *= 0.93 + 0.07 * min(1.0, t / 0.06)  # Coming up to pitch
        wobble[0] += 0.002 * (flutter.uniform(-1, 1) - wobble[0])
        f *= 1.0 + 0.6 * wobble[0] - 0.01 * t / length
        phase[0] = (phase[0] + f / RATE) % 1.0
        p = phase[0]
        buzz = (1.0 if p < 0.18 else -0.22) + 0.5 * (2 * p - 1)  # The slap: a narrow pulse on a ramp
        body = 6.0 * sum(b.next(buzz) for b in bells) + 0.25 * buzz + 0.15 * rattle.next()
        return math.tanh(1.6 * body) * envelope(t, length, 0.04, 0.12)
    return render(length, sample)


def siren(length: float = 5.0):
    phase = [0.0]

    def sample(t: float) -> float:
        f = 650.0 + 300.0 * math.sin(2 * math.pi * t / 2.5 - math.pi / 2)  # Two rises over 5 s
        phase[0] += 2 * math.pi * f / RATE
        return (math.sin(phase[0]) + 0.3 * math.sin(2 * phase[0])) * envelope(t, length, 0.2, 0.4)
    return render(length, sample)


def drop_tone(length: float = 2.0):
    """Two-tone descending, 2 s: the Gantry's drop tell."""
    def sample(t: float) -> float:
        f = 880.0 if (t % 0.5) < 0.25 else 660.0
        f *= 1.0 - 0.15 * t / length
        return (math.sin(2 * math.pi * f * t) + 0.25 * math.sin(4 * math.pi * f * t)) * envelope(t, length)
    return render(length, sample)


def hiss(length: float = 0.6, cutoff: float = 9000.0, seed: int = 1):
    noise = Noise(seed, cutoff)
    return render(length, lambda t: noise.next() * envelope(t, length, 0.02, 0.3))


def shriek(length: float = 1.0):
    noise = Noise(2, 7000.0)
    return render(length, lambda t: (0.6 * math.sin(2 * math.pi * (2400 + 300 * t) * t) + 0.5 * noise.next()) *
                  envelope(t, length, 0.05, 0.2))


def blast(length: float = 1.4):
    rumble, crack = Noise(3, 300.0), Noise(4, 6000.0)
    return render(length, lambda t: (2.5 * rumble.next() * math.exp(-2.5 * t) + crack.next() * math.exp(-14 * t) +
                                     0.8 * math.sin(2 * math.pi * 45 * t) * math.exp(-4 * t)) * envelope(t, length, 0.002, 0.3))


def snap(length: float = 0.2):
    crack = Noise(5, 8000.0)
    return render(length, lambda t: crack.next() * math.exp(-40 * t) + 0.4 * math.sin(2 * math.pi * 1800 * t) * math.exp(-60 * t))


def thud(length: float, freq: float, seed: int):
    body = Noise(seed, 400.0)
    return render(length, lambda t: (math.sin(2 * math.pi * freq * t * (1 - 0.3 * t)) * math.exp(-6 * t) +
                                     0.6 * body.next() * math.exp(-10 * t)) * envelope(t, length, 0.002, 0.1))


def groan(length: float = 1.2):
    """A structural groan: a low, wavering, rough tone."""
    grit = Noise(6, 200.0)

    def sample(t: float) -> float:
        f = 55.0 + 12.0 * math.sin(2 * math.pi * 1.3 * t)
        return (math.sin(2 * math.pi * f * t) + 0.5 * math.sin(2 * math.pi * 2.03 * f * t) + 0.8 * grit.next()) * \
            envelope(t, length, 0.15, 0.3)
    return render(length, sample)


def clang(length: float = 1.5):
    """The press ram at the top of its stroke: steel on steel, a dull knock (filtered noise) and a
    heavy body's inharmonic partials, each a pair beating slightly, dying at their own rates."""
    partials = [(97.0, 1.0, 3.0), (231.0, 0.7, 4.0), (377.0, 0.45, 6.0), (612.0, 0.3, 9.0), (1043.0, 0.18, 14.0),
                (1688.0, 0.1, 20.0)]
    knock = Noise(13, 900.0)
    return render(length, lambda t: (sum(a * (math.sin(2 * math.pi * f * t) + 0.6 * math.sin(2 * math.pi * f * 1.004 * t)) *
                                          math.exp(-d * t) for f, a, d in partials) +
                                      3.0 * knock.next() * math.exp(-30 * t)) * envelope(t, length, 0.001, 0.2))


CUES = {
    "KlaxonLow": lambda: klaxon([180.0]),
    "KlaxonHigh": lambda: klaxon([420.0]),
    "KlaxonTwoTone": lambda: klaxon([300.0, 400.0, 300.0, 400.0]),
    "Siren": siren,
    "DropTone": drop_tone,
    "Hiss": lambda: hiss(0.6),
    "ValveShriek": shriek,
    "GasJet": lambda: hiss(3.0, 4000.0, 7),
    "Blast": blast,
    "Snap": snap,
    "Groan": groan,
    "MagnetRelease": lambda: thud(0.6, 160.0, 8),
    "PlateLand": lambda: thud(1.5, 60.0, 9),
    "RamClang": clang,
}

# Each cue in the Yard: (how much of the yard you hear with it, the tail's length in s, its level).
# The horns carry and ring off the walls; the close mechanical sounds are mostly dry.
SPACE = {
    "KlaxonLow": (0.8, 2.2, 0.45), "KlaxonHigh": (0.8, 2.2, 0.4), "KlaxonTwoTone": (0.8, 2.2, 0.42),
    "Siren": (0.7, 2.5, 0.45), "DropTone": (0.6, 1.8, 0.4), "Blast": (0.9, 2.8, 0.8), "RamClang": (0.7, 2.0, 0.45),
    "Snap": (0.4, 0.8, 0.45), "Hiss": (0.3, 0.8, 0.4), "GasJet": (0.4, 1.2, 0.5),
}


def write(path: str, samples: list[float]) -> None:
    with wave.open(path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, v)) * 32767)) for v in samples))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "out", "audio"))
    args = parser.parse_args()
    os.makedirs(args.out, exist_ok=True)
    for name, make in CUES.items():
        wet, tail, level = SPACE.get(name, (0.5, 1.4, 0.5))
        write(os.path.join(args.out, f"SW_{name}.wav"), yard(make(), wet, tail, level))
    print(f"{len(CUES)} cues in {os.path.abspath(args.out)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
