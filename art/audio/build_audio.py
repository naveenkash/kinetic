"""Generates the tells' sound cues (docs/arenas/the-yard/events-and-moving-parts.md section 6;
bible/05 section 11) as 16-bit mono WAV files, from code, with the Python standard library:

    python3 art/audio/build_audio.py [--out art/out/audio]

Each cue is a short synthesis recipe: tones with harmonics for the horns and the siren, filtered
noise for hisses and blasts, inharmonic partials for the clang. Unreal imports them as
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


def klaxon(freqs: list[float], length: float = 1.6):
    def sample(t: float) -> float:
        step = int(t / (length / len(freqs)))
        return horn(freqs[min(step, len(freqs) - 1)], t) * envelope(t, length, 0.03, 0.15)
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
    """The press ram at the top of its stroke: inharmonic partials, a long ring."""
    partials = [(220.0, 1.0, 2.0), (523.0, 0.6, 3.0), (811.0, 0.4, 4.0), (1290.0, 0.3, 6.0), (1873.0, 0.2, 8.0)]
    return render(length, lambda t: sum(a * math.sin(2 * math.pi * f * t) * math.exp(-d * t) for f, a, d in partials) *
                  envelope(t, length, 0.002, 0.2))


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
        write(os.path.join(args.out, f"SW_{name}.wav"), make())
    print(f"{len(CUES)} cues in {os.path.abspath(args.out)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
