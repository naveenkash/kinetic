"""Seamless tiling surfaces for the arenas (docs/arenas/the-yard/lighting-and-mood.md, Palette), from
code:

    blender -b --factory-startup -P art/surfaces/build_surfaces.py -- [--res 2048] [--only concrete]

Each surface is a procedural shader baked from a 1 m (or larger) square to BaseColor, ORM (R
occlusion, G roughness, B metallic) and a tangent-space Normal map (OpenGL convention, as the props
are), written to art/out/surfaces/<name>/. Every noise is sampled on a 4D torus (the square's U and
V each go once round a circle, or several times for a streak), so each map tiles with no seam. The
arena kit (art/kit) UVs its meshes in metres and repeats these at their stated size.

The surfaces carry the small things the kit's geometry does not: concrete aggregate, bug holes and
hairline cracks; paint chipped to primer and to bare steel, rust bloom and the runs of rust and
grime under it (down the V axis, which is up the kit's walls); pitting, weld spatter and scratches;
zinc spangle and white rust; oil and wet patches smooth, dust and scale rough. Plate seams, panel
joints and corrugation are geometry now, so the maps leave them out. Heights are in metres and
the bump is scaled by the tile, so a 2 mm pit reads as 2 mm whatever the surface's size.
"""
from __future__ import annotations

import argparse
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:0] = [os.path.join(HERE, "..", "lib")]

import bpy  # noqa: E402

import kinetic_art as ka  # noqa: E402

OUT = os.path.join(HERE, "..", "out", "surfaces")


class Graph:
    """A small helper over a material's node tree, with seamless noise."""

    def __init__(self, name: str):
        self.mat = bpy.data.materials.new(name)
        self.mat.use_nodes = True
        self.nt = self.mat.node_tree
        self.bsdf = self.nt.nodes["Principled BSDF"]
        self.x = -2000
        uv = self.node("ShaderNodeTexCoord")
        sep = self.node("ShaderNodeSeparateXYZ")
        self.link(uv.outputs["UV"], sep.inputs[0])
        self.u, self.v = sep.outputs["X"], sep.outputs["Y"]
        self._torus = {}

    def node(self, kind: str, **inputs):
        n = self.nt.nodes.new(kind)
        n.location = (self.x, 0)
        self.x += 40
        for k, val in inputs.items():
            n.inputs[k].default_value = val
        return n

    def link(self, a, b):
        self.nt.links.new(a, b)

    def math(self, op: str, a, b=None):
        n = self.node("ShaderNodeMath")
        n.operation = op
        for i, val in enumerate((a, b)):
            if val is None:
                continue
            if isinstance(val, (int, float)):
                n.inputs[i].default_value = val
            else:
                self.link(val, n.inputs[i])
        return n.outputs[0]

    def add(self, *terms):
        """The sum of sockets and numbers."""
        out = terms[0]
        for t in terms[1:]:
            out = self.math("ADD", out, t)
        return out

    def mul(self, a, b):
        return self.math("MULTIPLY", a, b)

    def inv(self, a):
        return self.math("SUBTRACT", 1.0, a)

    def torus(self, repeats: int = 1, rv: int | None = None):
        """(vector, w) on a 4D torus: U `repeats` times round one circle and V `rv` (default the
        same) times round the other, so a noise on it tiles, stretched along V when rv < repeats."""
        rv = repeats if rv is None else rv
        if (repeats, rv) in self._torus:
            return self._torus[repeats, rv]
        ru_r, rv_r = repeats / (2 * math.pi), rv / (2 * math.pi)  # Noise detail per tile scales with the repeats
        cu = self.math("COSINE", self.math("MULTIPLY", self.u, 2 * math.pi * repeats))
        su = self.math("SINE", self.math("MULTIPLY", self.u, 2 * math.pi * repeats))
        cv = self.math("COSINE", self.math("MULTIPLY", self.v, 2 * math.pi * rv))
        sv = self.math("SINE", self.math("MULTIPLY", self.v, 2 * math.pi * rv))
        comb = self.node("ShaderNodeCombineXYZ")
        self.link(self.math("MULTIPLY", cu, ru_r), comb.inputs["X"])
        self.link(self.math("MULTIPLY", su, ru_r), comb.inputs["Y"])
        self.link(self.math("MULTIPLY", cv, rv_r), comb.inputs["Z"])
        self._torus[repeats, rv] = (comb.outputs["Vector"], self.math("MULTIPLY", sv, rv_r))
        return self._torus[repeats, rv]

    def noise(self, scale: float, detail: float = 4.0, roughness: float = 0.55, repeats: int = 1, offset: float = 0.0,
              rv: int | None = None, distortion: float = 0.0):
        vec, w = self.torus(repeats, rv)
        n = self.node("ShaderNodeTexNoise", Scale=scale, Detail=detail, Roughness=roughness, Distortion=distortion)
        n.noise_dimensions = "4D"
        self.link(vec, n.inputs["Vector"])
        self.link(self.math("ADD", w, offset) if offset else w, n.inputs["W"])
        return n.outputs["Fac"]

    def cells(self, scale: float, repeats: int = 1, feature: str = "F1", out: str = "Distance", offset: float = 0.0,
              warp: float = 0.0, randomness: float = 1.0):
        """Voronoi on the torus: F1 distance (stones, pits), DISTANCE_TO_EDGE (cracks, flakes), or a
        cell's random Color; `warp` bends the cells with a noise first."""
        vec, w = self.torus(repeats)
        if warp:
            wn = self.node("ShaderNodeTexNoise", Scale=scale * 0.6, Detail=3.0, Roughness=0.5)
            wn.noise_dimensions = "4D"
            self.link(vec, wn.inputs["Vector"])
            self.link(w, wn.inputs["W"])
            off = self.node("ShaderNodeVectorMath")
            off.operation = "SCALE"
            off.inputs["Scale"].default_value = warp / scale
            centred = self.node("ShaderNodeVectorMath")
            centred.operation = "SUBTRACT"
            self.link(wn.outputs["Color"], centred.inputs[0])
            centred.inputs[1].default_value = (0.5, 0.5, 0.5)
            self.link(centred.outputs[0], off.inputs[0])
            moved = self.node("ShaderNodeVectorMath")
            moved.operation = "ADD"
            self.link(vec, moved.inputs[0])
            self.link(off.outputs[0], moved.inputs[1])
            vec = moved.outputs[0]
        n = self.node("ShaderNodeTexVoronoi", Scale=scale, Randomness=randomness)
        n.voronoi_dimensions = "4D"
        n.feature = feature
        self.link(vec, n.inputs["Vector"])
        self.link(self.math("ADD", w, offset) if offset else w, n.inputs["W"])
        if out == "Color":
            bw = self.node("ShaderNodeRGBToBW")
            self.link(n.outputs["Color"], bw.inputs[0])
            return bw.outputs[0]
        return n.outputs[out]

    def ramp(self, fac, lo: float, hi: float):
        n = self.node("ShaderNodeMapRange")
        self.link(fac, n.inputs["Value"])
        n.inputs["From Min"].default_value = lo
        n.inputs["From Max"].default_value = hi
        n.clamp = True
        return n.outputs["Result"]

    def mix(self, fac, a, b):
        n = self.node("ShaderNodeMix")
        n.data_type = "RGBA"
        if isinstance(fac, float):
            n.inputs["Factor"].default_value = fac
        else:
            self.link(fac, n.inputs["Factor"])
        for sock, val in ((n.inputs[6], a), (n.inputs[7], b)):
            if isinstance(val, str):
                sock.default_value = ka.hex_rgb(val)
            else:
                self.link(val, sock)
        return n.outputs[2]

    def lerp(self, fac, a, b):
        """a to b (numbers or sockets) by fac."""
        n = self.node("ShaderNodeMix")
        n.data_type = "FLOAT"
        for sock, val in ((n.inputs[0], fac), (n.inputs[2], a), (n.inputs[3], b)):
            if isinstance(val, (int, float)):
                sock.default_value = val
            else:
                self.link(val, sock)
        return n.outputs[0]

    def grid_line(self, coord, count: float, width: float):
        """1 on lines at every 1/count of the tile (seams, joints), 0 between, soft-edged."""
        f = self.math("FRACT", self.math("MULTIPLY", coord, count))
        edge = self.math("MINIMUM", f, self.math("SUBTRACT", 1.0, f))
        return self.math("SUBTRACT", 1.0, self.ramp(edge, 0.0, width * count))

    def finish(self, colour, roughness, metallic, height, tile: float, gain: float = 3.0):
        """Wire up the BSDF; `height` is in metres on a tile `tile` metres across, its relief
        exaggerated `gain` times (a normal map has to say at a glance what touch would tell)."""
        self.link(colour, self.bsdf.inputs["Base Color"])
        for socket, val in (("Roughness", roughness), ("Metallic", metallic)):
            if isinstance(val, float):
                self.bsdf.inputs[socket].default_value = val
            else:
                self.link(val, self.bsdf.inputs[socket])
        if height is not None:
            b = self.node("ShaderNodeBump", Strength=1.0, Distance=gain / tile)
            self.link(height, b.inputs["Height"])
            self.link(b.outputs["Normal"], self.bsdf.inputs["Normal"])
        return self.mat, tile


# Shared layers --------------------------------------------------------------------------------------

def streaks(g: Graph, scale: float, offset: float, lo: float = 0.55, hi: float = 0.78, stretch: int = 14):
    """Runs down the V axis (rust, rain, grime): noise stretched `stretch` times along V, broken up."""
    run = g.ramp(g.noise(scale, 4.0, 0.55, repeats=stretch, rv=1, offset=offset), lo, hi)
    return g.mul(run, g.ramp(g.noise(scale * 0.7, 3.0, 0.5, offset=offset + 0.5), 0.4, 0.62))


def scratches(g: Graph, scale: float, offset: float, width: float = 0.012):
    """Fine scratches both ways across the tile: the thin contours of two stretched noises."""
    out = None
    for ru, rv, off in ((24, 1, 0.0), (1, 24, 0.7)):
        n = g.noise(scale, 2.0, 0.4, repeats=ru, rv=rv, offset=offset + off)
        line = g.inv(g.ramp(g.math("ABSOLUTE", g.math("SUBTRACT", n, 0.5)), 0.0, width))
        line = g.mul(line, g.ramp(g.noise(scale * 0.25, 2.0, 0.5, offset=offset + off + 3.0), 0.5, 0.65))
        out = line if out is None else g.math("MAXIMUM", out, line)
    return out


def spatter(g: Graph, scale: float, offset: float, amount: float = 0.64):
    """Weld spatter: tiny beads in clusters."""
    dots = g.inv(g.ramp(g.cells(scale, offset=offset, randomness=0.9), 0.0, 0.09))
    return g.mul(dots, g.ramp(g.noise(4.0, 3.0, 0.5, offset=offset + 1.0), amount, amount + 0.06))


def painted(g: Graph, lo: str, hi: str, tile: float, chips_at: float = 0.66, dust: str = "#7A6E60", gloss: float = 0.48):
    """Painted steel: the coat mottled and faded, chipped (to grey primer at the chip's rim and
    bare steel or rust inside), scratched, dusty, grimy, with rust bleeding from the chips."""
    fade = g.noise(3.0, 4.0, 0.55)
    coat = g.mix(fade, lo, hi)
    coat = g.mix(g.mul(g.noise(40.0, 3.0, 0.5, offset=0.3), 0.25), coat, "#000000")
    chip_n = g.noise(14.0 * tile, 8.0, 0.72, offset=1.0)
    chip = g.ramp(chip_n, chips_at, chips_at + 0.015)
    core = g.ramp(chip_n, chips_at + 0.035, chips_at + 0.07)
    rusty = g.ramp(g.noise(5.0, 4.0, 0.6, offset=2.0), 0.45, 0.6)
    bare = g.mix(rusty, "#5A5C5E", "#5A3018")
    col = g.mix(chip, coat, "#6E6A63")
    col = g.mix(core, col, bare)
    bleed = g.mul(streaks(g, 6.0, 3.0, 0.6, 0.8), g.ramp(chip_n, chips_at - 0.1, chips_at))
    col = g.mix(g.mul(bleed, 0.55), col, "#5E3820")
    scr = scratches(g, 9.0 * tile, 4.0)
    col = g.mix(g.mul(scr, 0.6), col, "#8A8C8E")
    dirt = g.ramp(g.noise(4.0, 5.0, 0.6, offset=5.0), 0.5, 0.82)
    col = g.mix(g.mul(dirt, 0.55), col, dust)
    grime = g.ramp(g.noise(2.0, 3.0, 0.5, offset=6.0), 0.55, 0.85)
    col = g.mix(g.mul(grime, 0.5), col, "#1E1B18")
    metal = g.add(g.mul(core, g.lerp(rusty, 0.9, 0.1)), g.mul(scr, 0.8))
    rough = g.lerp(chip, g.add(gloss, g.mul(fade, 0.08)), 0.6)
    rough = g.lerp(core, rough, g.lerp(rusty, 0.38, 0.85))
    rough = g.lerp(g.mul(dirt, 0.7), rough, 0.88)
    rough = g.lerp(g.mul(scr, 0.6), rough, 0.3)
    height = g.add(g.mul(g.inv(chip), 0.00025), g.mul(g.mul(core, rusty), g.mul(g.noise(300.0, 4.0, 0.6, offset=7.0), 0.0006)),
                   g.mul(scr, -0.00012), g.mul(g.noise(200.0 * tile, 2.0, 0.5, offset=8.0), 0.00003))
    return col, rough, metal, height


# The surfaces ------------------------------------------------------------------------------------

def concrete(g: Graph):
    """The docks' concrete apron and the trench walls: mottled grey paste with its aggregate
    showing where the surface has worn, bug holes, hairline and open cracks, oil stains and wet
    patches smooth, dust rough, faint rain runs (a tile is 4 m; the joints are geometry)."""
    T = 4.0
    mottle = g.noise(4.0, 6.0, 0.6)
    mottle2 = g.noise(20.0, 5.0, 0.6, offset=1.0)
    agg = g.cells(240.0, offset=2.0)
    agg_tone = g.cells(240.0, out="Color", offset=2.0)
    worn = g.ramp(g.noise(7.0, 5.0, 0.62, offset=2.5), 0.45, 0.72)
    stone = g.mul(g.inv(g.ramp(agg, 0.22, 0.42)), worn)
    fine = g.cells(700.0, offset=3.0)
    sand = g.mul(g.inv(g.ramp(fine, 0.15, 0.4)), 0.6)
    pores = g.mul(g.inv(g.ramp(g.cells(110.0, offset=4.0, randomness=1.0), 0.0, 0.07)),
                  g.ramp(g.noise(5.0, 3.0, 0.5, offset=4.5), 0.42, 0.6))
    crack_d = g.cells(2.6, feature="DISTANCE_TO_EDGE", offset=5.0, warp=0.9)
    crack = g.mul(g.inv(g.ramp(crack_d, 0.0, 0.0025)), g.ramp(g.noise(3.0, 3.0, 0.5, offset=5.5), 0.57, 0.65))
    hair_d = g.cells(9.0, feature="DISTANCE_TO_EDGE", offset=6.0, warp=1.2)
    hair = g.mul(g.inv(g.ramp(hair_d, 0.0, 0.001)), g.ramp(g.noise(6.0, 3.0, 0.5, offset=6.5), 0.58, 0.68))
    oil = g.ramp(g.noise(2.2, 5.0, 0.62, offset=7.0), 0.6, 0.7)
    wet = g.ramp(g.noise(1.6, 4.0, 0.55, offset=8.0), 0.56, 0.68)
    dust = g.ramp(g.noise(3.0, 5.0, 0.6, offset=9.0), 0.48, 0.75)
    runs = streaks(g, 3.0, 10.0, 0.6, 0.82, stretch=10)
    base = g.mix(mottle, "#4E4A45", "#6A645C")
    base = g.mix(g.mul(mottle2, 0.35), base, "#3E3A36")
    base = g.mix(g.mul(sand, 0.25), base, "#7C766C")
    base = g.mix(stone, base, g.mix(agg_tone, "#3F3C39", "#8C857A"))
    base = g.mix(g.mul(runs, 0.4), base, "#2E2A26")
    base = g.mix(g.mul(dust, 0.35), base, "#8A8378")
    base = g.mix(pores, base, "#24211E")
    base = g.mix(g.math("MAXIMUM", crack, g.mul(hair, 0.7)), base, "#1A1816")
    base = g.mix(g.mul(oil, 0.85), base, "#17130F")
    base = g.mix(g.mul(wet, 0.55), base, "#262320")
    rough = g.lerp(g.mul(stone, 0.6), 0.9, 0.72)
    rough = g.lerp(dust, rough, 0.96)
    rough = g.lerp(oil, rough, 0.22)
    rough = g.lerp(wet, rough, 0.32)
    height = g.add(g.mul(mottle2, 0.0012), g.mul(stone, 0.0012), g.mul(sand, 0.0003), g.mul(pores, -0.003),
                   g.mul(crack, -0.003), g.mul(hair, -0.0008), g.mul(g.noise(1500.0, 2.0, 0.5, offset=11.0), 0.00012))
    return g.finish(base, rough, 0.0, height, T)


def slag(g: Graph):
    """The trench beds: oiled slag ballast over the rail bed, dark angular stones and fines, a few
    rusty, oil-black and glossy in the wet (a tile is 2 m)."""
    T = 2.0
    big = g.cells(55.0, feature="SMOOTH_F1", offset=1.0)
    small = g.cells(150.0, offset=2.0)
    tone = g.cells(55.0, out="Color", offset=1.0)
    rusty = g.ramp(g.cells(55.0, out="Color", offset=3.0), 0.82, 0.86)
    stones = g.inv(g.ramp(big, 0.05, 0.5))
    pebbles = g.inv(g.ramp(small, 0.05, 0.45))
    fines = g.noise(500.0, 3.0, 0.6, offset=4.0)
    wet = g.ramp(g.noise(2.5, 4.0, 0.55, offset=7.0), 0.52, 0.62)
    base = g.mix(tone, "#1A1714", "#423B34")
    base = g.mix(rusty, base, "#4A2C1A")
    base = g.mix(g.mul(g.inv(stones), 0.6), base, g.mix(fines, "#15120F", "#2C2722"))
    base = g.mix(g.mul(wet, 0.75), base, "#0C0A08")
    rough = g.lerp(wet, g.lerp(stones, 0.97, 0.85), 0.14)
    height = g.add(g.mul(stones, 0.012), g.mul(pebbles, 0.004), g.mul(fines, 0.0008))
    return g.finish(base, rough, 0.0, height, T)


def hull_steel(g: Graph):
    """The Stacks' cut hull plate: structure steel paint (#4A5560) faded, chipped to red-oxide
    primer and to rust, rust bloom with its runs down the plate, grime, pitting under the rust,
    weld spatter, scratches (a tile is 4 m; the seams and their beads are geometry)."""
    T = 4.0
    col, rough, metal, height = painted(g, "#435060", "#56616E", T, chips_at=0.6, dust="#6E665C")
    bloom = g.ramp(g.noise(3.5, 6.0, 0.66, offset=11.0), 0.55, 0.69)
    bloom_core = g.ramp(g.noise(3.5, 6.0, 0.66, offset=11.0), 0.63, 0.76)
    runs = g.mul(streaks(g, 5.0, 12.0, 0.55, 0.8, stretch=18), g.ramp(g.noise(3.5, 4.0, 0.6, offset=11.0), 0.45, 0.62))
    col = g.mix(g.mul(bloom, 0.8), col, g.mix(g.noise(30.0, 4.0, 0.6, offset=13.0), "#6A3A1C", "#8A4E24"))
    col = g.mix(g.mul(bloom_core, 0.7), col, "#3A2014")
    col = g.mix(g.mul(runs, 0.8), col, "#5A3420")
    pits = g.mul(g.inv(g.ramp(g.cells(420.0, offset=14.0), 0.0, 0.16)), bloom)
    col = g.mix(pits, col, "#24140C")
    sp = spatter(g, 650.0, 15.0)
    col = g.mix(sp, col, "#7C7A76")
    rough = g.lerp(bloom, rough, 0.88)
    rough = g.lerp(g.mul(runs, 0.5), rough, 0.42)
    metal = g.lerp(bloom, metal, 0.05)
    metal = g.lerp(sp, metal, 0.9)
    height = g.add(height, g.mul(bloom, g.mul(g.noise(90.0, 6.0, 0.65, offset=16.0), 0.0012)), g.mul(pits, -0.0007), g.mul(sp, 0.0006))
    return g.finish(col, rough, metal, height, T)


def rust(g: Graph):
    """The Well's walls and old steel: heavy rust, flaking in curled plates over pitted scale,
    orange to near black, damp in places (a tile is 2 m)."""
    T = 2.0
    hue = g.noise(3.0, 5.0, 0.6)
    flake_edge = g.cells(11.0, feature="DISTANCE_TO_EDGE", offset=1.0, warp=2.5)
    flake_tone = g.cells(11.0, out="Color", offset=1.0)
    flaking = g.ramp(g.noise(3.0, 4.0, 0.6, offset=1.5), 0.48, 0.62)  # Where the scale is lifting at all
    lifted = g.ramp(flake_tone, 0.55, 0.75)
    scale_n = g.noise(45.0, 8.0, 0.7, offset=2.0)
    pits = g.inv(g.ramp(g.cells(260.0, offset=3.0), 0.0, 0.2))
    damp = g.ramp(g.noise(2.0, 4.0, 0.55, offset=4.0), 0.55, 0.7)
    runs = streaks(g, 4.0, 5.0, 0.58, 0.8, stretch=12)
    base = g.mix(hue, "#4A2614", "#7A4020")
    base = g.mix(g.ramp(g.noise(10.0, 5.0, 0.6, offset=5.0), 0.55, 0.75), base, "#8C5A2C")
    base = g.mix(g.mul(lifted, 0.5), base, "#5C2E16")
    base = g.mix(g.mul(g.inv(g.ramp(flake_edge, 0.0, 0.014)), g.mul(flaking, 0.75)), base, "#24140C")
    base = g.mix(g.mul(scale_n, 0.4), base, "#2E1A10")
    base = g.mix(g.mul(pits, 0.7), base, "#1A0E08")
    base = g.mix(g.mul(runs, 0.5), base, "#2A1810")
    base = g.mix(g.mul(damp, 0.5), base, "#1E120A")
    rough = g.lerp(damp, g.lerp(scale_n, 0.82, 0.97), 0.55)
    height = g.add(g.mul(g.mul(g.ramp(flake_edge, 0.0, 0.05), flaking), 0.0018), g.mul(g.mul(lifted, flaking), 0.0012),
                   g.mul(scale_n, 0.0016), g.mul(pits, -0.0008),
                   g.mul(g.noise(400.0, 3.0, 0.6, offset=6.0), 0.0003))
    return g.finish(base, rough, 0.08, height, T)


def corrugated(g: Graph):
    """The perimeter walls' galvanised sheet (its corrugation is geometry): zinc spangle, dulled
    and white-rusted in patches, red rust breaking through at spots with its runs below, grime
    (a tile is 2 m)."""
    T = 2.0
    spangle = g.cells(110.0, out="Color", offset=1.0)
    dull = g.noise(3.0, 4.0, 0.55, offset=2.0)
    white = g.ramp(g.noise(5.0, 6.0, 0.62, offset=3.0), 0.63, 0.74)
    red = g.ramp(g.noise(8.0, 7.0, 0.68, offset=4.0), 0.68, 0.74)
    red_core = g.ramp(g.noise(8.0, 7.0, 0.68, offset=4.0), 0.72, 0.8)
    runs = g.mul(streaks(g, 6.0, 5.0, 0.55, 0.8, stretch=16), g.ramp(g.noise(8.0, 7.0, 0.68, offset=4.0), 0.5, 0.66))
    grime = g.ramp(g.noise(2.5, 4.0, 0.55, offset=6.0), 0.5, 0.8)
    base = g.mix(spangle, "#8D9297", "#999EA2")
    base = g.mix(g.mul(dull, 0.5), base, "#6E7276")
    base = g.mix(g.mul(white, 0.6), base, "#A9A9A2")
    base = g.mix(g.mul(runs, 0.7), base, "#6A4026")
    base = g.mix(red, base, "#7A3E1E")
    base = g.mix(red_core, base, "#3E2214")
    base = g.mix(g.mul(grime, 0.45), base, "#2E2A26")
    sc = scratches(g, 14.0, 7.0)
    base = g.mix(g.mul(sc, 0.5), base, "#C4C8CC")
    metal = g.lerp(g.math("MAXIMUM", white, red), 0.85, 0.1)
    rough = g.lerp(spangle, 0.32, 0.46)
    rough = g.lerp(g.math("MAXIMUM", white, red), rough, 0.86)
    rough = g.lerp(g.mul(grime, 0.6), rough, 0.75)
    height = g.add(g.mul(red, g.mul(g.noise(120.0, 5.0, 0.6, offset=8.0), 0.0008)), g.mul(white, 0.00015),
                   g.mul(spangle, 0.00002), g.mul(sc, -0.0001))
    return g.finish(base, rough, metal, height, T)


def grating(g: Graph):
    """The bars of the open steel grating (walkways, bridge decks, stair treads, drains; the bars
    themselves are geometry): galvanised and dulled, worn bright along the tops, rust spots,
    mud and oil in the corners (a tile is 1 m)."""
    T = 1.0
    wear = g.ramp(g.noise(9.0, 6.0, 0.62), 0.55, 0.68)
    rusty = g.ramp(g.noise(7.0, 6.0, 0.66, offset=2.0), 0.66, 0.74)
    mud = g.ramp(g.noise(3.0, 4.0, 0.55, offset=3.0), 0.55, 0.78)
    base = g.mix(g.noise(25.0, 3.0, 0.5, offset=4.0), "#45494D", "#5A5E62")
    base = g.mix(wear, base, "#9A9EA2")
    base = g.mix(rusty, base, "#5E3620")
    base = g.mix(g.mul(mud, 0.7), base, "#2E2822")
    metal = g.lerp(g.math("MAXIMUM", rusty, mud), g.lerp(wear, 0.6, 0.95), 0.05)
    rough = g.lerp(wear, 0.55, 0.32)
    rough = g.lerp(g.math("MAXIMUM", rusty, mud), rough, 0.9)
    height = g.add(g.mul(rusty, g.mul(g.noise(120.0, 4.0, 0.6, offset=5.0), 0.0015)), g.mul(mud, 0.001),
                   g.mul(g.noise(300.0, 3.0, 0.6, offset=6.0), 0.0001))
    return g.finish(base, rough, metal, height, T)


def hazard(g: Graph):
    """Rated-load and pit-rail tape and stair nosings: yellow (#F2C318) on black diagonal stripes,
    the edges ragged, scuffed and chipped to steel, dirty (a tile is 1 m)."""
    T = 1.0
    diag = g.math("FRACT", g.add(g.mul(g.add(g.u, g.v), 4.0), g.mul(g.noise(40.0, 3.0, 0.5, offset=1.0), 0.03)))
    stripe = g.ramp(diag, 0.49, 0.51)
    paint = g.mix(stripe, "#E2B416", "#121212")
    paint = g.mix(g.mul(g.noise(12.0, 4.0, 0.55, offset=2.0), 0.3), paint, "#7A6A3A")
    col, rough, metal, height = painted_over(g, paint, T, chips_at=0.6)
    scuff = g.mul(g.ramp(g.noise(6.0, 3.0, 0.5, repeats=8, rv=1, offset=3.0), 0.6, 0.75), 0.5)
    col = g.mix(scuff, col, "#3A3632")
    return g.finish(col, rough, metal, height, T)


def painted_over(g: Graph, coat, tile: float, chips_at: float):
    """painted() for a coat already built as a colour socket (the hazard stripes)."""
    chip_n = g.noise(14.0 * tile, 8.0, 0.72, offset=1.0)
    chip = g.ramp(chip_n, chips_at, chips_at + 0.015)
    core = g.ramp(chip_n, chips_at + 0.035, chips_at + 0.07)
    col = g.mix(chip, coat, "#6E6A63")
    col = g.mix(core, col, "#4E4A46")
    scr = scratches(g, 9.0 * tile, 4.0)
    col = g.mix(g.mul(scr, 0.6), col, "#8A8C8E")
    dirt = g.ramp(g.noise(4.0, 5.0, 0.6, offset=5.0), 0.45, 0.82)
    col = g.mix(g.mul(dirt, 0.6), col, "#4A4238")
    metal = g.add(g.mul(core, 0.9), g.mul(scr, 0.8))
    rough = g.lerp(core, 0.55, 0.4)
    rough = g.lerp(g.mul(dirt, 0.7), rough, 0.9)
    height = g.add(g.mul(g.inv(chip), 0.00025), g.mul(scr, -0.00012), g.mul(dirt, 0.0001))
    return col, rough, metal, height


def ochre(g: Graph):
    """Guild ochre paint (#C8922A) on handrails and the Guild's kit: chalk-marked, chipped, worn
    to steel where hands go, rust bleeding from the chips (a tile is 1 m)."""
    col, rough, metal, height = painted(g, "#B8842A", "#CC9A34", 1.0, chips_at=0.6, dust="#8A7A60", gloss=0.42)
    return g.finish(col, rough, metal, height, 1.0)


def htr_orange(g: Graph):
    """HTR orange (#E8651A) on Truck B, notices and survey tags: dusty, chipped (a tile is 2 m)."""
    col, rough, metal, height = painted(g, "#D45A18", "#EC6A1E", 2.0, chips_at=0.62, dust="#9A7A60", gloss=0.45)
    return g.finish(col, rough, metal, height, 2.0)


def dark_steel(g: Graph):
    """Machinery black: oiled steel on the press, the Haulers, rails and stringers: mill-scale
    blotches, oil sheen and dry dust, bright scratches, rust in the pits (a tile is 2 m)."""
    T = 2.0
    scale_n = g.noise(6.0, 5.0, 0.6)
    oil = g.ramp(g.noise(3.0, 5.0, 0.58, offset=1.0), 0.5, 0.68)
    dust = g.ramp(g.noise(4.0, 5.0, 0.6, offset=2.0), 0.58, 0.8)
    rusty = g.ramp(g.noise(9.0, 7.0, 0.68, offset=3.0), 0.68, 0.76)
    sc = scratches(g, 18.0, 4.0, 0.01)
    base = g.mix(scale_n, "#1A1C1F", "#2E3135")
    base = g.mix(g.mul(g.ramp(g.noise(14.0, 4.0, 0.6, offset=5.0), 0.55, 0.7), 0.6), base, "#2C2620")
    base = g.mix(g.mul(dust, 0.6), base, "#4E4943")
    base = g.mix(rusty, base, "#5A3018")
    base = g.mix(g.mul(sc, 0.8), base, "#80848A")
    sp = spatter(g, 500.0, 6.0, 0.68)
    base = g.mix(sp, base, "#5A5A58")
    metal = g.lerp(g.math("MAXIMUM", dust, rusty), 0.85, 0.1)
    rough = g.lerp(oil, 0.55, 0.2)
    rough = g.lerp(dust, rough, 0.85)
    rough = g.lerp(rusty, rough, 0.9)
    rough = g.lerp(g.mul(sc, 0.7), rough, 0.3)
    height = g.add(g.mul(rusty, g.mul(g.noise(150.0, 5.0, 0.6, offset=7.0), 0.0008)), g.mul(sc, -0.00012), g.mul(sp, 0.0005),
                   g.mul(scale_n, 0.0001))
    return g.finish(base, rough, metal, height, T)


SURFACES = {
    "Concrete": concrete, "Slag": slag, "HullSteel": hull_steel, "Rust": rust, "Corrugated": corrugated,
    "Grating": grating, "Hazard": hazard, "Ochre": ochre, "HTROrange": htr_orange, "DarkSteel": dark_steel,
}


def bake_surface(name: str, build, res: int, out: str) -> dict:
    ka.reset_scene()
    bpy.ops.mesh.primitive_plane_add(size=1.0)
    plane = bpy.context.active_object
    g = Graph(f"M_{name}")
    mat, tile_m = build(g)
    plane.data.materials.append(mat)
    scene = bpy.context.scene
    scene.cycles.samples = 8
    os.makedirs(out, exist_ok=True)
    paths = {}

    def run(kind: str, bake_type: str, colorspace: str, **kw):
        img = bpy.data.images.new(f"T_{name}_{kind}", res, res, alpha=False)
        img.colorspace_settings.name = colorspace
        tex = mat.node_tree.nodes.new("ShaderNodeTexImage")
        tex.image = img
        mat.node_tree.nodes.active = tex
        bpy.ops.object.bake(type=bake_type, margin=0, **kw)
        path = os.path.join(out, f"T_{name}_{kind}.png")
        img.filepath_raw = path
        img.file_format = "PNG"
        img.save()
        mat.node_tree.nodes.remove(tex)
        paths[kind] = path
        return img

    import numpy as np
    out_node = mat.node_tree.nodes["Material Output"]
    shader = out_node.inputs["Surface"].links[0].from_socket
    # Base colour and metallic through an emission shader (Cycles bakes metal's diffuse as black).
    emit = mat.node_tree.nodes.new("ShaderNodeEmission")
    for kind, socket in (("BaseColor", "Base Color"), ("Metallic", "Metallic")):
        src = g.bsdf.inputs[socket]
        if src.is_linked:
            mat.node_tree.links.new(src.links[0].from_socket, emit.inputs["Color"])
        else:
            v = src.default_value
            emit.inputs["Color"].default_value = tuple(v)[:3] + (1.0,) if hasattr(v, "__len__") else (v, v, v, 1.0)
        mat.node_tree.links.new(emit.outputs["Emission"], out_node.inputs["Surface"])
        run(kind, "EMIT", "sRGB" if kind == "BaseColor" else "Non-Color")
    mat.node_tree.links.new(shader, out_node.inputs["Surface"])
    run("Roughness", "ROUGHNESS", "Non-Color")
    run("Normal", "NORMAL", "Non-Color", normal_space="TANGENT")
    # Pack ORM: no baked occlusion (the kit's geometry and Lumen give it), roughness, metallic.
    rough = np.array(bpy.data.images[f"T_{name}_Roughness"].pixels[:]).reshape(res, res, 4)
    metal = np.array(bpy.data.images[f"T_{name}_Metallic"].pixels[:]).reshape(res, res, 4)
    orm = bpy.data.images.new(f"T_{name}_ORM", res, res, alpha=False)
    orm.colorspace_settings.name = "Non-Color"
    packed = np.ones((res, res, 4), dtype=np.float32)
    packed[..., 1] = rough[..., 0]
    packed[..., 2] = metal[..., 0]
    orm.pixels[:] = packed.ravel()
    path = os.path.join(out, f"T_{name}_ORM.png")
    orm.filepath_raw = path
    orm.file_format = "PNG"
    orm.save()
    for kind in ("Roughness", "Metallic"):
        os.remove(paths.pop(kind))
    paths["ORM"] = path
    info = {"name": name, "tile_m": tile_m, "maps": {k: os.path.basename(v) for k, v in paths.items()}}
    with open(os.path.join(out, "surface.json"), "w") as f:
        json.dump(info, f, indent=2)
    return info


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="art/surfaces/build_surfaces.py")
    parser.add_argument("--res", type=int, default=2048)
    parser.add_argument("--only", default=None)
    args = parser.parse_args(argv)
    for name, build in SURFACES.items():
        if args.only and name.lower() not in [n.strip().lower() for n in args.only.split(",")]:
            continue
        info = bake_surface(name, build, args.res, os.path.join(os.path.abspath(OUT), name))
        print(f"{name}: tiles every {info['tile_m']} m, {sorted(info['maps'])}")
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    raise SystemExit(main(argv))
