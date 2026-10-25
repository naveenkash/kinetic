"""Seamless tiling surfaces for the arenas (docs/arenas/the-yard/lighting-and-mood.md, Palette), from
code:

    blender -b --factory-startup -P art/surfaces/build_surfaces.py -- [--res 1024] [--only concrete]

Each surface is a procedural shader baked from a 1 m (or larger) square to BaseColor, ORM (R
occlusion, G roughness, B metallic) and a tangent-space Normal map (OpenGL convention, as the props
are), written to art/out/surfaces/<name>/. Every noise is sampled on a 4D torus (the square's U and
V each go once round a circle), so each map tiles with no seam. The arena kit (art/kit) UVs its
meshes in metres and repeats these at their stated size.
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

    def torus(self, repeats: int = 1):
        """(vector, w) on a 4D torus: U and V each once round a circle, `repeats` times per tile."""
        if repeats in self._torus:
            return self._torus[repeats]
        tau = 2 * math.pi * repeats
        r = repeats / (2 * math.pi)  # Noise detail per tile scales with the repeats
        cu, su = self.math("COSINE", self.math("MULTIPLY", self.u, tau)), self.math("SINE", self.math("MULTIPLY", self.u, tau))
        cv, sv = self.math("COSINE", self.math("MULTIPLY", self.v, tau)), self.math("SINE", self.math("MULTIPLY", self.v, tau))
        comb = self.node("ShaderNodeCombineXYZ")
        self.link(self.math("MULTIPLY", cu, r), comb.inputs["X"])
        self.link(self.math("MULTIPLY", su, r), comb.inputs["Y"])
        self.link(self.math("MULTIPLY", cv, r), comb.inputs["Z"])
        self._torus[repeats] = (comb.outputs["Vector"], self.math("MULTIPLY", sv, r))
        return self._torus[repeats]

    def noise(self, scale: float, detail: float = 4.0, roughness: float = 0.55, repeats: int = 1, offset: float = 0.0):
        vec, w = self.torus(repeats)
        n = self.node("ShaderNodeTexNoise", Scale=scale, Detail=detail, Roughness=roughness)
        n.noise_dimensions = "4D"
        self.link(vec, n.inputs["Vector"])
        self.link(self.math("ADD", w, offset) if offset else w, n.inputs["W"])
        return n.outputs["Fac"]

    def cells(self, scale: float, repeats: int = 1, feature: str = "F1"):
        vec, w = self.torus(repeats)
        n = self.node("ShaderNodeTexVoronoi", Scale=scale)
        n.voronoi_dimensions = "4D"
        n.feature = feature
        self.link(vec, n.inputs["Vector"])
        self.link(w, n.inputs["W"])
        return n.outputs["Distance"]

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
        self.link(fac, n.inputs["Factor"]) if not isinstance(fac, float) else None
        if isinstance(fac, float):
            n.inputs["Factor"].default_value = fac
        for sock, val in ((n.inputs[6], a), (n.inputs[7], b)):
            if isinstance(val, str):
                sock.default_value = ka.hex_rgb(val)
            else:
                self.link(val, sock)
        return n.outputs[2]

    def grid_line(self, coord, count: float, width: float):
        """1 on lines at every 1/count of the tile (seams, joints), 0 between, soft-edged."""
        f = self.math("FRACT", self.math("MULTIPLY", coord, count))
        edge = self.math("MINIMUM", f, self.math("SUBTRACT", 1.0, f))
        return self.math("SUBTRACT", 1.0, self.ramp(edge, 0.0, width * count))

    def finish(self, colour, roughness, metallic, height=None, bump: float = 0.3):
        self.link(colour, self.bsdf.inputs["Base Color"])
        if isinstance(roughness, float):
            self.bsdf.inputs["Roughness"].default_value = roughness
        else:
            self.link(roughness, self.bsdf.inputs["Roughness"])
        if isinstance(metallic, float):
            self.bsdf.inputs["Metallic"].default_value = metallic
        else:
            self.link(metallic, self.bsdf.inputs["Metallic"])
        if height is not None:
            b = self.node("ShaderNodeBump", Strength=1.0, Distance=bump)
            self.link(height, b.inputs["Height"])
            self.link(b.outputs["Normal"], self.bsdf.inputs["Normal"])
        return self.mat


# The surfaces ------------------------------------------------------------------------------------

def concrete(g: Graph):
    """The docks' concrete apron: mottled grey, aggregate, tyre-darkened patches, oil stains, and
    a joint every 2 m (a tile is 4 m)."""
    mottle = g.noise(6.0, 6.0, 0.6, repeats=2)
    aggregate = g.ramp(g.cells(40.0, repeats=2), 0.0, 0.15)
    stain = g.ramp(g.noise(2.5, 3.0, 0.5, repeats=1, offset=3.0), 0.55, 0.7)
    base = g.mix(mottle, "#4C4945", "#67625B")
    base = g.mix(g.math("MULTIPLY", aggregate, 0.25), base, "#7A746B")
    base = g.mix(stain, base, "#1E1A17")
    joint = g.math("MAXIMUM", g.grid_line(g.u, 2.0, 0.004), g.grid_line(g.v, 2.0, 0.004))
    base = g.mix(joint, base, "#1A1816")
    rough = g.ramp(stain, 0.0, 1.0)
    rough = g.math("SUBTRACT", 0.92, g.math("MULTIPLY", rough, 0.45))
    height = g.math("SUBTRACT", g.math("MULTIPLY", mottle, 0.3), g.math("MULTIPLY", joint, 1.0))
    return g.finish(base, rough, 0.0, height, bump=0.04), 4.0


def slag(g: Graph):
    """The trench beds: oiled slag over the rail bed, dark and gritty with wet patches."""
    grit = g.cells(60.0, repeats=2)
    lumps = g.noise(10.0, 8.0, 0.7, repeats=2)
    wet = g.ramp(g.noise(3.0, 2.0, 0.5, repeats=1, offset=7.0), 0.5, 0.65)
    base = g.mix(lumps, "#1F1B18", "#3A332D")
    base = g.mix(g.math("MULTIPLY", wet, 0.7), base, "#100E0C")
    rough = g.math("SUBTRACT", 0.95, g.math("MULTIPLY", wet, 0.6))
    height = g.math("ADD", g.math("MULTIPLY", lumps, 0.6), g.math("MULTIPLY", g.ramp(grit, 0.0, 0.3), 0.4))
    return g.finish(base, rough, 0.0, height, bump=0.06), 2.0


def hull_steel(g: Graph):
    """The Stacks' cut hull plate: structure steel paint (#4A5560) worn to bare steel and rust,
    plate seams with weld beads every 2 m by 1.35 m and rivet rows along them (a tile is 4 m)."""
    seam_u, seam_v = g.grid_line(g.u, 2.0, 0.003), g.grid_line(g.v, 3.0, 0.003)
    seam = g.math("MAXIMUM", seam_u, seam_v)
    # Rivets: dots every 12.5 cm along the seams.
    dots = g.ramp(g.cells(64.0, repeats=1), 0.0, 0.12)
    rivets = g.math("MULTIPLY", g.math("SUBTRACT", 1.0, dots), g.ramp(g.math("MAXIMUM", g.grid_line(g.u, 2.0, 0.012),
                                                                             g.grid_line(g.v, 3.0, 0.012)), 0.5, 1.0))
    wear = g.ramp(g.noise(5.0, 6.0, 0.65, repeats=2), 0.55, 0.75)
    rust = g.ramp(g.noise(3.0, 5.0, 0.6, repeats=1, offset=11.0), 0.58, 0.74)
    streaks = g.ramp(g.noise(1.0, 2.0, 0.4, repeats=1, offset=5.0), 0.4, 0.8)
    paint = g.mix(g.noise(20.0, 3.0, 0.5, repeats=2), "#465160", "#525C68")
    base = g.mix(wear, paint, "#6D6A66")
    base = g.mix(rust, base, "#6B3B1F")
    base = g.mix(g.math("MULTIPLY", streaks, 0.25), base, "#3E2A1E")
    base = g.mix(seam, base, "#2A2A2C")
    metal = g.math("MULTIPLY", wear, 0.9)
    rough = g.math("ADD", 0.55, g.math("MULTIPLY", rust, 0.35))
    height = g.math("ADD", g.math("MULTIPLY", seam, 0.6), g.math("MULTIPLY", rivets, 1.0))
    height = g.math("SUBTRACT", height, g.math("MULTIPLY", rust, 0.15))
    return g.finish(base, rough, metal, height, bump=0.03), 4.0


def rust(g: Graph):
    """The Well's walls and old steel: flaking rust over pitted plate."""
    flake = g.cells(18.0, repeats=2, feature="F2")
    pits = g.noise(30.0, 8.0, 0.7, repeats=2)
    base = g.mix(g.noise(4.0, 4.0, 0.6, repeats=1), "#5A2E17", "#8A4A22")
    base = g.mix(g.ramp(flake, 0.2, 0.5), "#3A2116", base)
    height = g.math("ADD", g.math("MULTIPLY", pits, 0.5), g.math("MULTIPLY", g.ramp(flake, 0.2, 0.5), 0.5))
    return g.finish(base, 0.85, 0.15, height, bump=0.05), 2.0


def corrugated(g: Graph):
    """The perimeter walls: galvanised corrugated sheet, 7.6 cm pitch, grimed at the foot."""
    wave = g.math("SINE", g.math("MULTIPLY", g.u, 2 * math.pi * 26.0))  # 26 corrugations per 2 m tile
    grime = g.ramp(g.math("SUBTRACT", 1.0, g.v), 0.7, 1.0)
    spots = g.ramp(g.noise(8.0, 4.0, 0.6, repeats=2), 0.6, 0.75)
    base = g.mix(g.noise(12.0, 3.0, 0.5, repeats=2), "#7C8085", "#959A9E")
    base = g.mix(spots, base, "#6B4A2E")
    base = g.mix(g.math("MULTIPLY", grime, 0.8), base, "#2B2622")
    height = g.math("MULTIPLY", g.math("ADD", wave, 1.0), 0.5)
    return g.finish(base, 0.55, 0.8, height, bump=0.03), 2.0


def grating(g: Graph):
    """Open steel grating for walkways and bridge decks: 3 cm by 10 cm bars, painted, worn
    bright on top (a tile is 1 m)."""
    bars = g.math("MAXIMUM", g.grid_line(g.u, 33.0, 0.004), g.grid_line(g.v, 10.0, 0.003))
    wear = g.ramp(g.noise(6.0, 4.0, 0.6, repeats=1), 0.5, 0.7)
    base = g.mix(wear, "#3B4046", "#8E9296")
    base = g.mix(g.math("SUBTRACT", 1.0, bars), base, "#050505")  # The holes read as dark
    height = bars
    return g.finish(base, 0.5, 0.7, height, bump=0.02), 1.0


def hazard(g: Graph):
    """Rated-load and pit-rail tape: yellow (#F2C318) on black diagonal stripes, scuffed."""
    diag = g.math("FRACT", g.math("MULTIPLY", g.math("ADD", g.u, g.v), 4.0))
    stripe = g.ramp(diag, 0.48, 0.52)
    scuff = g.ramp(g.noise(10.0, 6.0, 0.6, repeats=1), 0.6, 0.72)
    base = g.mix(stripe, "#F2C318", "#111111")
    base = g.mix(g.math("MULTIPLY", scuff, 0.6), base, "#4A4540")
    return g.finish(base, 0.6, 0.0, g.math("MULTIPLY", scuff, 0.2), bump=0.01), 1.0


def ochre(g: Graph):
    """Guild ochre paint (#C8922A) on handrails, chalk-marked and chipped."""
    chips = g.ramp(g.noise(12.0, 6.0, 0.65, repeats=2), 0.62, 0.72)
    base = g.mix(g.noise(15.0, 3.0, 0.5, repeats=2), "#BD8826", "#CC982E")
    base = g.mix(chips, base, "#5C5F63")
    return g.finish(base, g.math("ADD", 0.5, g.math("MULTIPLY", chips, -0.2)), g.math("MULTIPLY", chips, 0.9),
                    g.math("SUBTRACT", 0.0, chips), bump=0.004), 1.0


def htr_orange(g: Graph):
    """HTR orange (#E8651A) on Truck B, notices and survey tags; dusty."""
    dust = g.ramp(g.noise(5.0, 4.0, 0.6, repeats=2), 0.55, 0.8)
    chips = g.ramp(g.noise(14.0, 6.0, 0.65, repeats=2, offset=4.0), 0.66, 0.74)
    base = g.mix(dust, "#E8651A", "#9A6A4A")
    base = g.mix(chips, base, "#5C5F63")
    return g.finish(base, 0.55, g.math("MULTIPLY", chips, 0.9), g.math("SUBTRACT", 0.0, chips), bump=0.004), 2.0


def dark_steel(g: Graph):
    """Machinery black: oiled steel on the press, the Haulers and the rails."""
    oil = g.ramp(g.noise(4.0, 4.0, 0.55, repeats=1), 0.5, 0.75)
    scratches = g.ramp(g.noise(60.0, 2.0, 0.3, repeats=2), 0.7, 0.75)
    base = g.mix(g.noise(10.0, 3.0, 0.5, repeats=2), "#1C1E21", "#2A2D31")
    base = g.mix(scratches, base, "#6A6E72")
    return g.finish(base, g.math("SUBTRACT", 0.6, g.math("MULTIPLY", oil, 0.35)), 0.85, scratches, bump=0.005), 2.0


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
    parser.add_argument("--res", type=int, default=1024)
    parser.add_argument("--only", default=None)
    args = parser.parse_args(argv)
    for name, build in SURFACES.items():
        if args.only and name.lower() != args.only.lower():
            continue
        info = bake_surface(name, build, args.res, os.path.join(os.path.abspath(OUT), name))
        print(f"{name}: tiles every {info['tile_m']} m, {sorted(info['maps'])}")
    return 0


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    raise SystemExit(main(argv))
