"""The surfaces of a harness character as procedural Cycles materials, and the bake that turns
them into the game's textures (art/characters/harness.py uses both).

Every material describes colour, roughness, metal and relief (a height field into a Bump node),
so the bake gets a normal map as well as BaseColor and ORM:

- the suit's cloth: a woven grain, seams with their stitch rows (planes and rectangles in the
  rest pose: side seams, yokes, the placket, armholes, cuffs, pockets, knee patches), grime where
  work puts it (a per-vertex mask from harness_body), dirt in the folds;
- skin (per-vertex colour from the head's model, fine pores), leather (grain, creases, scuffed
  toes and edges, dirt in the creases), sole rubber;
- painted metal worn by the class's wear level: chips at the edges and scratches through to the
  metal beneath, grime in the crevices, dents; bare metal (brushed steel, brass with its patina,
  polished edges), mirror steel with its swing-arc scratches, anodised alloy, polymer;
- webbing that knows its own length and width (a per-vertex attribute, `strap`, written by
  harness_parts.strap_path): the weave along it, the bound edges, box-and-X stitching at the ends.

The bake (`bake`, Cycles, on the GPU when there is one) takes the hard parts from their own
materials and projects the soft parts from their dense meshes, and writes BaseColor, ORM
(occlusion, roughness, metallic) and a tangent-space Normal map, OpenGL convention (green up),
as the props' bakes in art/lib do.
"""
from __future__ import annotations

import math
import os

import bpy
import numpy as np

import kinetic_art as ka


# Helpers (copied from art/lib/kinetic_art.py, which changes under its own owner) --------------------

def _mix(nodes, data_type: str = "RGBA"):
    """A Mix node and its sockets of the right type (Mix has an A, B and Result per type)."""
    m = nodes.new("ShaderNodeMix")
    m.data_type = data_type
    want = "RGBA" if data_type == "RGBA" else "VALUE"
    pick = lambda sockets, name: next(s for s in sockets if s.name == name and s.type == want)
    return m, pick(m.inputs, "A"), pick(m.inputs, "B"), pick(m.outputs, "Result")


def _bake_target(obj, image) -> None:
    for slot in obj.material_slots:
        nt = slot.material.node_tree
        for n in [n for n in nt.nodes if n.name == "BakeTarget"]:
            nt.nodes.remove(n)
        tex = nt.nodes.new("ShaderNodeTexImage")
        tex.name = "BakeTarget"
        tex.image = image
        tex.location = (600, 600)
        nt.nodes.active = tex


def _emit_from(objects, socket_of) -> list:
    """Points every material on `objects` at an emission shader fed by `socket_of(bsdf)` (a socket
    to link, or a value): the trick for passes Cycles cannot bake directly. Returns the undo list."""
    undo = []
    for mat in {slot.material for o in objects for slot in o.material_slots if slot.material}:
        nt = mat.node_tree
        bsdf = nt.nodes.get("Principled BSDF")
        out = nt.nodes.get("Material Output")
        emit = nt.nodes.new("ShaderNodeEmission")
        src = socket_of(bsdf)
        if isinstance(src, bpy.types.NodeSocket) and src.is_linked:
            nt.links.new(src.links[0].from_socket, emit.inputs["Color"])
        else:
            v = src.default_value if isinstance(src, bpy.types.NodeSocket) else src
            emit.inputs["Color"].default_value = tuple(v)[:3] + (1.0,) if hasattr(v, "__len__") else (v, v, v, 1.0)
        old = out.inputs["Surface"].links[0].from_socket
        nt.links.new(emit.outputs["Emission"], out.inputs["Surface"])
        undo.append((nt, old, out, emit))
    return undo


def _restore(undo: list) -> None:
    for nt, old, out, emit in undo:
        nt.links.new(old, out.inputs["Surface"])
        nt.nodes.remove(emit)


def join(name: str, objects: list) -> bpy.types.Object:
    """Applies every part's modifiers and joins them into one mesh with weighted normals (flat
    faces shade flat, bevels round; smooth surfaces stay smooth)."""
    for o in objects:
        bpy.context.view_layer.objects.active = o
        for m in list(o.modifiers):
            bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.ops.object.select_all(action="DESELECT")
    for o in objects:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    result = bpy.context.view_layer.objects.active
    result.name = result.data.name = name
    weighted = result.modifiers.new("WeightedNormals", "WEIGHTED_NORMAL")
    weighted.keep_sharp = True
    bpy.ops.object.modifier_apply(modifier=weighted.name)
    return result


def face_out(obj) -> None:
    """Turns a closed part's faces outward if they face in (by its signed volume): a strap's or a
    shell's winding depends on how it was built, and the bake needs to know which side is out."""
    me = obj.data
    me.calc_loop_triangles()
    if not len(me.loop_triangles):
        return
    tris = np.empty(len(me.loop_triangles) * 3, np.int32)
    me.loop_triangles.foreach_get("vertices", tris)
    co = np.empty(len(me.vertices) * 3)
    me.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    t = co[tris.reshape(-1, 3)]
    if np.einsum("ij,ij->i", t[:, 0], np.cross(t[:, 1], t[:, 2])).sum() < 0:
        me.flip_normals()


def duplicate(obj, name: str | None = None):
    """A copy with its own mesh, sharing materials: a part in both the dense and game meshes."""
    copy = obj.copy()
    copy.data = obj.data.copy()
    copy.name = name or obj.name
    bpy.context.scene.collection.objects.link(copy)
    return copy


# Node graphs ------------------------------------------------------------------------------------

class Graph:
    """A material's node tree, built a socket at a time; numbers are accepted wherever a socket is."""

    def __init__(self, name: str):
        self.mat = bpy.data.materials.new(name)
        self.mat.use_nodes = True
        self.nt = self.mat.node_tree
        self.bsdf = self.nt.nodes["Principled BSDF"]
        self.n = 0
        geo = self.node("ShaderNodeNewGeometry")
        self.P = geo.outputs["Position"]
        self.N = geo.outputs["Normal"]
        self.geo = geo

    def node(self, kind: str, **props):
        n = self.nt.nodes.new(kind)
        n.location = (-2400 + 180 * (self.n // 12), 900 - 150 * (self.n % 12))
        self.n += 1
        for k, v in props.items():
            setattr(n, k, v)
        return n

    def put(self, sock, value) -> None:
        if isinstance(value, bpy.types.NodeSocket):
            self.nt.links.new(value, sock)
        elif value is not None:
            sock.default_value = value

    def math(self, op: str, a, b=None, c=None, clamp: bool = False):
        n = self.node("ShaderNodeMath", operation=op, use_clamp=clamp)
        self.put(n.inputs[0], a)
        if b is not None:
            self.put(n.inputs[1], b)
        if c is not None:
            self.put(n.inputs[2], c)
        return n.outputs[0]

    def add(self, *terms):
        out = terms[0]
        for t in terms[1:]:
            out = self.math("ADD", out, t)
        return out

    def mul(self, a, b, clamp: bool = False):
        return self.math("MULTIPLY", a, b, clamp=clamp)

    def vmath(self, op: str, a, b=None, scale=None, out: str | None = None):
        n = self.node("ShaderNodeVectorMath", operation=op)
        self.put(n.inputs[0], a)
        if b is not None:
            self.put(n.inputs[1], b)
        if scale is not None:
            self.put(n.inputs["Scale"], scale)
        return n.outputs[out or ("Value" if op in ("DOT_PRODUCT", "LENGTH", "DISTANCE") else "Vector")]

    def dot(self, a, b):
        return self.vmath("DOT_PRODUCT", a, b)

    def xyz(self, v):
        n = self.node("ShaderNodeSeparateXYZ")
        self.put(n.inputs[0], v)
        return n.outputs["X"], n.outputs["Y"], n.outputs["Z"]

    def vec(self, x, y, z):
        n = self.node("ShaderNodeCombineXYZ")
        for i, v in enumerate((x, y, z)):
            self.put(n.inputs[i], v)
        return n.outputs[0]

    def smooth(self, value, lo: float, hi: float, to=(0.0, 1.0)):
        """smoothstep(lo, hi, value) mapped to `to` (lo > hi inverts it)."""
        n = self.node("ShaderNodeMapRange", interpolation_type="SMOOTHSTEP", clamp=True)
        self.put(n.inputs["Value"], value)
        n.inputs["From Min"].default_value = lo
        n.inputs["From Max"].default_value = hi
        n.inputs["To Min"].default_value, n.inputs["To Max"].default_value = to
        return n.outputs["Result"]

    def lerp(self, value, lo: float, hi: float, to=(0.0, 1.0)):
        n = self.node("ShaderNodeMapRange", clamp=True)
        self.put(n.inputs["Value"], value)
        n.inputs["From Min"].default_value = lo
        n.inputs["From Max"].default_value = hi
        n.inputs["To Min"].default_value, n.inputs["To Max"].default_value = to
        return n.outputs["Result"]

    def noise(self, scale: float, detail: float = 4.0, roughness: float = 0.55, vector=None, distortion: float = 0.0,
              out: str = "Fac"):
        n = self.node("ShaderNodeTexNoise")
        self.put(n.inputs["Vector"], vector if vector is not None else self.P)
        n.inputs["Scale"].default_value = scale
        n.inputs["Detail"].default_value = detail
        n.inputs["Roughness"].default_value = roughness
        n.inputs["Distortion"].default_value = distortion
        return n.outputs[out]

    def voronoi(self, scale: float, vector=None, feature: str = "F1", randomness: float = 1.0, out: str = "Distance"):
        n = self.node("ShaderNodeTexVoronoi", feature=feature)
        self.put(n.inputs["Vector"], vector if vector is not None else self.P)
        n.inputs["Scale"].default_value = scale
        n.inputs["Randomness"].default_value = randomness
        return n.outputs[out]

    def stretch(self, scale) -> bpy.types.NodeSocket:
        """The position scaled per axis (stretched noise: brushing, scratches, streaks)."""
        n = self.node("ShaderNodeMapping")
        self.put(n.inputs["Vector"], self.P)
        n.inputs["Scale"].default_value = scale
        return n.outputs["Vector"]

    def rotated(self, degrees: float, axis: str = "Y", scale=(1, 1, 1)):
        n = self.node("ShaderNodeMapping")
        self.put(n.inputs["Vector"], self.P)
        rot = [0.0, 0.0, 0.0]
        rot["XYZ".index(axis)] = math.radians(degrees)
        n.inputs["Rotation"].default_value = rot
        n.inputs["Scale"].default_value = scale
        return n.outputs["Vector"]

    def mix(self, fac, a, b):
        m, sa, sb, out = _mix(self.nt.nodes, "RGBA")
        m.location = (-2400 + 180 * (self.n // 12), 900 - 150 * (self.n % 12))
        self.n += 1
        self.put(m.inputs["Factor"], fac)
        self.put(sa, a if isinstance(a, bpy.types.NodeSocket) else colour(a))
        self.put(sb, b if isinstance(b, bpy.types.NodeSocket) else colour(b))
        return out

    def mixf(self, fac, a, b):
        m, sa, sb, out = _mix(self.nt.nodes, "FLOAT")
        self.n += 1
        self.put(m.inputs["Factor"], fac)
        self.put(sa, a)
        self.put(sb, b)
        return out

    def attr(self, name: str, out: str = "Fac"):
        n = self.node("ShaderNodeAttribute", attribute_name=name)
        return n.outputs[out]

    def edges(self, radius: float = 0.002, lo: float = 0.02, hi: float = 0.25):
        """1 on convex edges and corners (the Bevel shader's rounded normal against the true one)."""
        bev = self.node("ShaderNodeBevel", samples=6)
        bev.inputs["Radius"].default_value = radius
        d = self.dot(bev.outputs["Normal"], self.N)
        return self.smooth(self.math("SUBTRACT", 1.0, d), lo, hi)

    def cavity(self, distance: float = 0.02, lo: float = 0.4, hi: float = 1.0):
        """1 deep in crevices and folds, 0 in the open (ambient occlusion)."""
        ao = self.node("ShaderNodeAmbientOcclusion", samples=8)
        ao.inputs["Distance"].default_value = distance
        return self.smooth(ao.outputs["AO"], hi, lo)

    def finish(self, base, rough, metal, height=None, bump_strength: float = 1.0):
        self.put(self.bsdf.inputs["Base Color"], base)
        self.put(self.bsdf.inputs["Roughness"], rough)
        self.put(self.bsdf.inputs["Metallic"], metal)
        if height is not None:
            b = self.node("ShaderNodeBump")
            b.inputs["Strength"].default_value = bump_strength
            b.inputs["Distance"].default_value = 1.0
            self.put(b.inputs["Height"], height)
            self.nt.links.new(b.outputs["Normal"], self.bsdf.inputs["Normal"])
        return self.mat


def colour(hex_or_rgb):
    if isinstance(hex_or_rgb, str):
        return ka.hex_rgb(hex_or_rgb)
    c = tuple(hex_or_rgb)
    return c if len(c) == 4 else c + (1.0,)


def shade(hex_color: str, factor: float):
    r, g, b, _ = ka.hex_rgb(hex_color)
    return (r * factor, g * factor, b * factor, 1.0)


# Seams on the suit ------------------------------------------------------------------------------

def seam_height(g: Graph, d, stitches: bool = True, groove: float = 0.0007, spacing: float = 0.0032, stitch: float = 0.0004):
    """The relief across a seam at signed distance d (m): a groove where the panels meet, and
    stitch rows either side (dots of thread on a 3.5 mm pitch)."""
    a = g.math("ABSOLUTE", d)
    h = g.mul(g.smooth(a, 0.0012, 0.0), -groove)
    if stitches:
        row = g.smooth(g.math("ABSOLUTE", g.math("SUBTRACT", a, spacing)), 0.0007, 0.0002)
        dots = g.smooth(g.voronoi(1.0 / 0.0035, randomness=0.0), 0.42, 0.18)
        h = g.add(h, g.mul(g.mul(row, dots), stitch))
        # The seam's edges stand a little proud (the folded allowance underneath).
        h = g.add(h, g.mul(g.smooth(a, 0.009, 0.0015), 0.00025))
    return h


def region(g: Graph, value, lo: float, hi: float, soft: float = 0.006):
    """1 where lo < value < hi."""
    return g.mul(g.smooth(value, lo - soft, lo + soft), g.smooth(value, hi + soft, hi - soft))


def plane_seam(g: Graph, normal, offset: float, mask=None, stitches: bool = True):
    d = g.math("SUBTRACT", g.dot(g.P, tuple(normal)), offset)
    h = seam_height(g, d, stitches)
    return g.mul(h, mask) if mask is not None else h


def suit_fabric(name: str, spec: dict, layout: dict):
    """The coverall's cloth. `layout` (harness_body.Suit.layout) gives the seams' places in the
    rest pose."""
    g = Graph(name)
    body = spec.get("workwear", spec["colours"]["Body"][0])
    thread = spec.get("thread", "#8C7A5A")
    wear = spec.get("wear", 0.5)
    x, y, z = g.xyz(g.P)
    ay = g.math("ABSOLUTE", y)
    torso = g.smooth(ay, 0.215, 0.195)
    # Seams: the side and shoulder line, the waist, the back yoke, the placket, armholes, cuffs, hems.
    h = plane_seam(g, (1, 0, 0), layout.get("side_x", -0.004), g.smooth(z, 1.58, 1.54))
    h = g.add(h, plane_seam(g, (0, 0, 1), 1.035, torso))
    h = g.add(h, plane_seam(g, (0, 0, 1), 1.385, g.mul(g.smooth(x, -0.01, -0.03), g.smooth(ay, 0.16, 0.14))))
    front = g.mul(g.smooth(x, 0.03, 0.05), region(g, z, 0.86, 1.53))
    for s in (1.0, -1.0):
        h = g.add(h, plane_seam(g, (0, s, 0), 0.024, front))
    h = g.add(h, g.mul(seam_height(g, y, stitches=False, groove=0.0011), front))  # The zip
    for joint in layout["shoulders"]:
        dist = g.math("SUBTRACT", g.vmath("DISTANCE", g.P, tuple(joint)), 0.098)
        h = g.add(h, g.mul(seam_height(g, dist), g.smooth(ay, 0.12, 0.15)))
    for elbow, axis, at in layout["cuffs"]:
        rel = g.vmath("SUBTRACT", g.P, tuple(elbow))
        along = g.dot(rel, tuple(axis))
        radial = g.vmath("LENGTH", g.vmath("CROSS_PRODUCT", rel, tuple(axis)))
        near = g.smooth(radial, 0.08, 0.06)
        for c in at:
            h = g.add(h, g.mul(seam_height(g, g.math("SUBTRACT", along, c)), near))
    for c in layout["hems"]:
        h = g.add(h, g.mul(seam_height(g, g.math("SUBTRACT", z, c)), g.smooth(z, 0.45, 0.4)))
    for rect in layout["patches"]:  # Pockets and patches: (frame origin, u, v, w axes, centre, half sizes)
        h = g.add(h, patch_outline(g, *rect))
    seams = h
    # The cloth's grain: a fine woven texture, slubs and wrinkles too small to model.
    grain = g.mul(g.noise(2600.0, 2.0, 0.6), 0.00009)
    twill = g.mul(g.math("SINE", g.mul(g.add(g.mul(x, 0.7), g.mul(z, 0.7), g.mul(y, 0.3)), 2 * math.pi / 0.0016)), 0.000035)
    slub = g.mul(g.noise(380.0, 3.0, 0.6, vector=g.stretch((1.0, 1.0, 0.25))), 0.00018)
    crinkle = g.mul(g.noise(70.0, 4.0, 0.65), 0.0004)
    h = g.add(h, grain, twill, slub, crinkle)
    # Colour: dyed cloth, faded on the crests and the top surfaces, grimy where work rubs and in
    # the folds, darker with sweat under the arms and at the collar.
    mottle = g.noise(45.0, 4.0, 0.6)
    base = g.mix(g.lerp(mottle, 0.3, 0.7), shade(body, 0.86), shade(body, 1.1))
    up = g.smooth(g.xyz(g.N)[2], 0.4, 0.95)
    base = g.mix(g.mul(up, 0.18 * (0.4 + wear)), base, shade(body, 1.35))
    bands = None  # Retro-reflective tape sewn on (the Breaker's hi-vis)
    for z0, z1 in spec.get("reflective", ()):
        b = region(g, z, z0, z1, soft=0.0015)
        if z0 > 0.9:
            b = g.mul(b, torso)
        bands = b if bands is None else g.math("MAXIMUM", bands, b)
    if bands is not None:
        beads = g.noise(3000.0, 1.0, 0.5)
        base = g.mix(bands, base, g.mix(beads, "#B9BDC0", "#D4D8DA"))
        h = g.add(h, g.mul(bands, 0.0003), g.mul(g.mul(bands, beads), 0.00002))
    grime = g.attr("grime")
    dirt = spec.get("dirt", "#3A3026")
    base = g.mix(g.mul(grime, 0.55 + 0.3 * wear), base, dirt)
    base = g.mix(g.mul(g.cavity(0.03, 0.55, 0.95), 0.35 + 0.25 * wear), base, shade(dirt, 0.5))
    sweat = g.attr("sweat")
    base = g.mix(g.mul(sweat, 0.35), base, shade(body, 0.62))
    stitch_col = g.smooth(seams, 0.0003, 0.00042)
    base = g.mix(g.mul(stitch_col, 0.4), base, thread)
    rough = g.add(0.84, g.mul(mottle, 0.1), g.mul(grime, -0.12))
    if bands is not None:
        rough = g.mixf(bands, rough, 0.35)
    return g.finish(base, rough, 0.0, h)


def patch_outline(g: Graph, origin, u, v, w, centre, half, kind: str = "box"):
    """The relief of a sewn patch's edge (a pocket, a knee patch), in a limb's frame: u forward,
    v out, w along the limb; `centre` and `half` are (arc, along). For kind "plane", the frame's
    first two coordinates are used directly (the seat pockets)."""
    rel = g.vmath("SUBTRACT", g.P, tuple(origin))
    qu, qv, qw = g.dot(rel, tuple(u)), g.dot(rel, tuple(v)), g.dot(rel, tuple(w))
    if kind == "plane":
        a, b, depth = qv, qw, g.smooth(qu, 0.03, 0.05)
    else:
        r = g.math("SQRT", g.add(g.mul(qu, qu), g.mul(qv, qv)))
        a = g.mul(g.math("ARCTAN2", qv, qu), g.math("MAXIMUM", r, 0.05))
        b, depth = qw, g.smooth(r, 0.14, 0.11)
    da = g.math("SUBTRACT", g.math("ABSOLUTE", g.math("SUBTRACT", a, centre[0])), half[0])
    db = g.math("SUBTRACT", g.math("ABSOLUTE", g.math("SUBTRACT", b, centre[1])), half[1])
    d = g.math("MAXIMUM", da, db)
    edge = seam_height(g, g.math("ADD", d, 0.003), spacing=0.0025)
    return g.mul(edge, depth)


# Skin, leather, rubber ----------------------------------------------------------------------------

def skin(name: str):
    g = Graph(name)
    base = g.attr("skin", "Color")
    pores = g.mul(g.smooth(g.voronoi(2200.0), 0.0, 0.35), -0.00003)
    fine = g.mul(g.noise(900.0, 3.0, 0.6), 0.00004)
    hair = g.attr("hair")
    stubble = g.mul(g.mul(g.noise(5200.0, 1.0, 0.5), hair), 0.0002)
    rough = g.add(0.48, g.mul(g.noise(60.0, 2.0, 0.5), 0.1), g.mul(hair, 0.3))
    return g.finish(base, rough, 0.0, g.add(pores, fine, stubble))


def eye(name: str, iris: str = "#4A3524"):
    """The eyeball: sclera, iris and pupil by angle from its forward axis (the `eye` attribute:
    each vertex's direction from the eyeball's centre)."""
    g = Graph(name)
    loc = g.attr("eye", "Vector")
    fx = g.xyz(loc)[0]
    iris_m = g.smooth(fx, 0.84, 0.88)
    pupil = g.smooth(fx, 0.965, 0.975)
    ring = g.smooth(g.math("ABSOLUTE", g.math("SUBTRACT", fx, 0.86)), 0.02, 0.0)
    stroma = g.noise(900.0, 3.0, 0.6, vector=loc)
    base = g.mix(iris_m, "#D9D2C8", g.mix(stroma, shade(iris, 0.7), shade(iris, 1.3)))
    base = g.mix(g.mul(ring, 0.8), base, "#1A1410")
    base = g.mix(pupil, base, "#050505")
    veins = g.mul(g.smooth(g.noise(300.0, 6.0, 0.7, vector=loc), 0.62, 0.7), g.math("SUBTRACT", 1.0, iris_m))
    base = g.mix(g.mul(veins, 0.4), base, "#B4524A")
    return g.finish(base, 0.08, 0.0)


def leather(name: str, color: str, wear: float = 0.5, toe_steel: bool = False):
    g = Graph(name)
    x, y, z = g.xyz(g.P)
    grain = g.mul(g.smooth(g.voronoi(900.0), 0.0, 0.5), 0.00004)
    pebble = g.mul(g.noise(320.0, 4.0, 0.6), 0.00006)
    creases = g.mul(g.smooth(g.noise(70.0, 3.0, 0.6, vector=g.stretch((3.0, 0.5, 1.0)), distortion=0.4), 0.55, 0.62), -0.00035)
    h = g.add(grain, pebble, creases)
    e = g.edges(0.003)
    toe = g.mul(g.smooth(x, 0.15, 0.21), g.smooth(z, 0.09, 0.05))
    scuff = g.mul(g.add(g.mul(e, 0.7), toe), g.smooth(g.noise(55.0, 4.0, 0.6), 0.42, 0.62))
    scuff = g.math("MINIMUM", g.mul(scuff, 0.4 + 0.8 * wear), 1.0)
    mottle = g.noise(25.0, 4.0, 0.6)
    base = g.mix(g.lerp(mottle, 0.3, 0.7), shade(color, 0.8), shade(color, 1.12))
    base = g.mix(scuff, base, shade(color, 1.55))
    base = g.mix(g.mul(g.cavity(0.02, 0.5, 0.95), 0.5), base, shade(color, 0.45))
    dust = g.mul(g.smooth(z, 0.12, 0.02), 0.25 + 0.35 * wear)
    base = g.mix(dust, base, "#6E6252")
    metal = 0.0
    if toe_steel:  # The steel cap showing through the worn leather at the toe
        bare = g.mul(g.mul(toe, g.smooth(g.noise(40.0, 5.0, 0.65), 0.6 - 0.12 * wear, 0.66 - 0.12 * wear)), g.smooth(x, 0.19, 0.215))
        base = g.mix(bare, base, "#9A9C9C")
        metal = bare
    rough = g.add(0.6, g.mul(scuff, 0.2), g.mul(mottle, -0.12))
    rough = g.mixf(metal if not isinstance(metal, float) else 0.0, rough, 0.35)
    return g.finish(base, rough, metal, h)


def rubber(name: str, color: str = "#1A1A1A", wear: float = 0.5):
    g = Graph(name)
    e = g.edges(0.002)
    base = g.mix(g.lerp(g.noise(60.0, 3.0, 0.6), 0.3, 0.7), shade(color, 0.85), shade(color, 1.2))
    base = g.mix(g.mul(e, 0.3 + 0.4 * wear), base, "#5A564E")
    base = g.mix(g.mul(g.cavity(0.012, 0.4, 0.95), 0.6), base, "#4A4034")
    h = g.mul(g.noise(800.0, 2.0, 0.5), 0.00003)
    return g.finish(base, g.add(0.82, g.mul(e, 0.1)), 0.0, h)


# Metals and polymers -------------------------------------------------------------------------------

def scratches(g: Graph, density: float, angle: float = 0.0, scale: float = 60.0):
    """Fine straight scratches in three directions (or mostly one, along `angle`)."""
    out = None
    for i, a in enumerate((angle, angle + 63.0, angle - 41.0)):
        vec = g.rotated(a, "Y", (scale, scale, scale * 0.03))
        n = g.noise(1.0, 2.0, 0.5, vector=vec, distortion=0.1)
        line = g.smooth(g.math("ABSOLUTE", g.math("SUBTRACT", n, 0.5)), 0.006 + 0.004 * density, 0.0)
        line = g.mul(line, g.smooth(g.noise(8.0, 2.0, 0.5), 0.62 - 0.25 * density, 0.75 - 0.25 * density))
        line = g.mul(line, 1.0 if i == 0 else 0.6)
        out = line if out is None else g.math("MAXIMUM", out, line)
    return out


def worn_paint(name: str, paint: str, under: str = "#B8B4AC", wear: float = 0.5, paint_rough=(0.45, 0.68),
               under_rough: float = 0.32, scratch: float = 0.5, grime: str = "#2A2420", metal_under: bool = True):
    """Paint over metal: mottled, chipped at the edges and scratched through, dented, grimy in the
    crevices and dusty on top. wear 0 is fresh; 1 is a lifetime of use."""
    g = Graph(name)
    mottle = g.noise(18.0, 4.0, 0.6)
    e = g.edges(0.0025, 0.03, 0.3)
    chips_n = g.noise(70.0, 6.0, 0.7)
    chip = g.mul(e, g.smooth(chips_n, 0.62 - 0.3 * wear, 0.66 - 0.3 * wear))
    sc = g.mul(scratches(g, wear * scratch), min(1.0, 0.35 + wear))
    bare = g.math("MINIMUM", g.math("MAXIMUM", chip, sc), 1.0)
    base = g.mix(g.lerp(mottle, 0.25, 0.75), shade(paint, 0.82), shade(paint, 1.1))
    fade = g.smooth(g.xyz(g.N)[2], 0.5, 1.0)
    base = g.mix(g.mul(fade, 0.12 * wear), base, shade(paint, 1.3))
    base = g.mix(bare, base, under)
    cav = g.cavity(0.025, 0.45, 0.95)
    base = g.mix(g.mul(cav, 0.45 + 0.35 * wear), base, grime)
    base = g.mix(g.mul(g.smooth(g.noise(6.0, 3.0, 0.6), 0.5, 0.8), 0.18 * wear), base, grime)
    rough = g.lerp(mottle, 0.0, 1.0, paint_rough)
    rough = g.mixf(bare, rough, under_rough)
    rough = g.mixf(g.mul(cav, 0.5), rough, 0.85)
    metal = g.mul(bare, 1.0 if metal_under else 0.0)
    dents = g.mul(g.noise(9.0, 2.0, 0.5), 0.0005 * wear)
    h = g.add(g.mul(bare, -0.00008), dents, g.mul(mottle, 0.00002))
    return g.finish(base, rough, metal, h)


def bare_metal(name: str, color: str, rough: float = 0.35, wear: float = 0.5, patina: str = "#2E2A22",
               brushed: bool = True, polish: float = 0.3):
    """Machined or cast metal: brushed or not, polished bright at the edges, patina in the crevices."""
    g = Graph(name)
    if brushed:
        streak = g.noise(1.0, 3.0, 0.5, vector=g.stretch((900.0, 900.0, 12.0)))
    else:
        streak = g.noise(140.0, 4.0, 0.6)
    e = g.edges(0.0018, 0.03, 0.3)
    sc = scratches(g, wear)
    base = g.mix(g.lerp(streak, 0.3, 0.7), shade(color, 0.86), shade(color, 1.08))
    base = g.mix(g.mul(e, polish), base, shade(color, 1.35))
    cav = g.cavity(0.02, 0.45, 0.95)
    base = g.mix(g.mul(cav, 0.55 + 0.3 * wear), base, patina)
    base = g.mix(g.mul(sc, 0.4), base, shade(color, 1.25))
    r = g.add(rough, g.mul(streak, 0.08), g.mul(e, -0.12 * polish), g.mul(cav, 0.3), g.mul(sc, 0.1))
    metal = g.mixf(g.mul(cav, 0.8 * wear), 1.0, 0.4)
    h = g.add(g.mul(streak, 0.000015), g.mul(sc, -0.00004))
    return g.finish(base, r, metal, h)


def mirror(name: str, color: str = "#D8DADC", wear: float = 0.3):
    """Polished stainless: fine scratches along the swing arc, never dull."""
    g = Graph(name)
    sc = scratches(g, 0.6 + wear * 0.5, angle=12.0, scale=90.0)
    haze = g.noise(30.0, 3.0, 0.6)
    base = g.mix(g.mul(sc, 0.5), color, shade(color, 0.82))
    base = g.mix(g.mul(g.cavity(0.015, 0.5, 0.95), 0.5), base, "#3C3A36")
    rough = g.add(0.06, g.mul(sc, 0.14), g.mul(haze, 0.03))
    return g.finish(base, rough, 1.0, g.mul(sc, -0.00002))


def anodised(name: str, color: str, wear: float = 0.15):
    """Anodised alloy: satin, machined, its colour worn to bright alloy only on the sharpest edges."""
    g = Graph(name)
    e = g.edges(0.0015, 0.08, 0.35)
    worn = g.mul(e, g.smooth(g.noise(60.0, 4.0, 0.6), 0.7 - 0.4 * wear, 0.74 - 0.4 * wear))
    base = g.mix(g.lerp(g.noise(30.0, 3.0, 0.5), 0.3, 0.7), shade(color, 0.92), shade(color, 1.05))
    base = g.mix(worn, base, "#C9CCCE")
    base = g.mix(g.mul(g.cavity(0.02, 0.5, 0.95), 0.3 + 0.4 * wear), base, "#2A2826")
    rough = g.add(0.42, g.mul(worn, -0.15), g.mul(g.noise(200.0, 2.0, 0.5), 0.05))
    return g.finish(base, rough, 0.85, g.mul(g.noise(1.0, 2.0, 0.5, vector=g.stretch((1200.0, 1200.0, 30.0))), 0.00001))


def polymer(name: str, color: str, wear: float = 0.5, rough=(0.62, 0.78)):
    """Moulded ABS or composite: a fine stipple, pale scuffs on the edges, grime in the seams."""
    g = Graph(name)
    stipple = g.noise(1400.0, 2.0, 0.5)
    e = g.edges(0.002)
    scuff = g.mul(e, g.smooth(g.noise(50.0, 4.0, 0.6), 0.5 - 0.2 * wear, 0.6 - 0.2 * wear))
    base = g.mix(g.lerp(g.noise(20.0, 3.0, 0.5), 0.3, 0.7), shade(color, 0.9), shade(color, 1.08))
    base = g.mix(g.mul(scuff, 0.6), base, shade(color, 1.8) if ka.hex_rgb(color)[0] < 0.2 else shade(color, 0.7))
    base = g.mix(g.mul(g.cavity(0.02, 0.5, 0.95), 0.5), base, "#1E1C1A")
    r = g.add(rough[0], g.mul(stipple, rough[1] - rough[0]), g.mul(scuff, 0.1))
    return g.finish(base, r, 0.0, g.mul(stipple, 0.00002))


def webbing(name: str, color: str, wear: float = 0.5, dirt: str = "#3A3026"):
    """Woven webbing. The `strap` attribute (x along the strap, y across, z its width, metres)
    lays the weave along it, raises the bound edges and puts box-and-X stitching at its ends."""
    g = Graph(name)
    uvw = g.attr("strap", "Vector")
    along, across, width = g.xyz(uvw)
    length = g.attr("strap_length")
    half = g.mul(width, 0.5)
    weave = g.mul(g.math("SINE", g.mul(along, 2 * math.pi / 0.0018)), 0.00003)
    ribs = g.mul(g.math("SINE", g.mul(across, 2 * math.pi / 0.0011)), 0.00002)
    edge_d = g.math("SUBTRACT", half, g.math("ABSOLUTE", across))
    binding = g.mul(g.smooth(edge_d, 0.0035, 0.0012), 0.00012)
    # The stitched ends: a box and its diagonals within 3 cm of either end.
    end_d = g.math("MINIMUM", along, g.math("SUBTRACT", length, along))
    near_end = g.mul(g.smooth(end_d, 0.045, 0.035), g.smooth(length, 0.08, 0.1))
    inner = g.math("SUBTRACT", half, 0.004)
    box_u = g.math("ABSOLUTE", g.math("SUBTRACT", end_d, 0.022))
    box = g.math("MINIMUM", g.math("ABSOLUTE", g.math("SUBTRACT", g.math("ABSOLUTE", across), inner)),
                 g.math("SUBTRACT", box_u, 0.0))
    box = g.math("MAXIMUM", box, g.math("SUBTRACT", g.math("ABSOLUTE", across), inner))
    box = g.math("MINIMUM", box, g.math("MAXIMUM", g.math("ABSOLUTE", g.math("SUBTRACT", box_u, 0.014)),
                                         g.math("SUBTRACT", g.math("ABSOLUTE", across), inner)))
    diag = g.math("ABSOLUTE", g.math("SUBTRACT", g.math("ABSOLUTE", across),
                                     g.mul(g.math("SUBTRACT", end_d, 0.008), g.math("DIVIDE", inner, 0.028))))
    diag = g.math("MAXIMUM", diag, g.math("SUBTRACT", box_u, 0.014))
    lines = g.math("MINIMUM", box, diag)
    dots = g.smooth(g.math("FRACT", g.mul(g.add(along, across), 1.0 / 0.003)), 0.8, 0.4)
    stitch = g.mul(g.mul(g.smooth(lines, 0.0009, 0.0002), dots), near_end)
    h = g.add(weave, ribs, binding, g.mul(stitch, 0.00025))
    mottle = g.noise(30.0, 4.0, 0.6)
    base = g.mix(g.lerp(mottle, 0.3, 0.7), shade(color, 0.85), shade(color, 1.08))
    base = g.mix(g.mul(g.smooth(edge_d, 0.004, 0.0), 0.25 + 0.3 * wear), base, shade(color, 1.25 if ka.hex_rgb(color)[0] < 0.1 else 0.8))
    base = g.mix(g.mul(g.cavity(0.02, 0.5, 0.95), 0.4 + 0.35 * wear), base, dirt)
    base = g.mix(g.mul(g.smooth(g.noise(4.0, 3.0, 0.6), 0.45, 0.75), 0.35 * wear), base, dirt)
    base = g.mix(g.mul(stitch, 0.8), base, shade(color, 0.6) if ka.hex_rgb(color)[0] > 0.3 else "#8A8A82")
    rough = g.add(0.82, g.mul(mottle, 0.1))
    return g.finish(base, rough, 0.0, h)


def tape(name: str, color: str, wear: float = 0.5):
    g = Graph(name)
    mottle = g.noise(60.0, 3.0, 0.6)
    weave = g.mul(g.noise(1800.0, 2.0, 0.5), 0.00002)
    base = g.mix(g.lerp(mottle, 0.3, 0.7), shade(color, 0.85), shade(color, 1.05))
    base = g.mix(g.mul(g.cavity(0.015, 0.5, 0.95), 0.3 + 0.4 * wear), base, "#2A2420")
    base = g.mix(g.mul(g.smooth(g.noise(8.0, 3.0, 0.6), 0.5, 0.8), 0.4 * wear), base, "#4A4036")
    return g.finish(base, g.add(0.55, g.mul(mottle, 0.15)), 0.0, weave)


def glazed(name: str, color: str, rough: float = 0.25, chips: float = 1.0):
    """Glaze, glass or a moulded shield: glossy, chipped on its edges by `chips`, grimy in the crevices."""
    g = Graph(name)
    e = g.edges(0.0015, 0.05, 0.3)
    chip = g.mul(g.mul(e, g.smooth(g.noise(90.0, 4.0, 0.6), 0.66, 0.7)), chips)
    base = g.mix(chip, color, "#8C8478")
    base = g.mix(g.mul(g.cavity(0.01, 0.5, 0.95), 0.4), base, "#4A443C")
    return g.finish(base, g.add(rough, g.mul(chip, 0.5)), 0.0, g.mul(chip, -0.00005))


# The bake --------------------------------------------------------------------------------------

def use_gpu() -> str:
    """Cycles on the GPU (Metal, OptiX, CUDA or HIP) when there is one; the CPU otherwise."""
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    if os.environ.get("KINETIC_CPU"):
        scene.cycles.device = "CPU"
        return "CPU"
    prefs = bpy.context.preferences.addons["cycles"].preferences
    for kind in ("METAL", "OPTIX", "CUDA", "HIP"):
        try:
            prefs.compute_device_type = kind
        except TypeError:
            continue
        prefs.get_devices()
        gpus = [d for d in prefs.devices if d.type == kind]
        if gpus:
            for d in prefs.devices:
                d.use = d.type == kind
            if hasattr(prefs, "kernel_optimization_level"):
                # Metal's specialised kernels compile on a background thread that can crash a
                # headless Blender; the generic kernel is barely slower for a bake.
                prefs.kernel_optimization_level = "OFF"
            scene.cycles.device = "GPU"
            return kind
    scene.cycles.device = "CPU"
    return "CPU"


def bake(low, soft, out_dir: str, prefix: str, resolution: int = 4096, samples: int = 16,
         cage: float = 0.008) -> dict[str, str]:
    """Bakes the game mesh's textures: BaseColor, Roughness, AO, Metallic and Normal (tangent
    space, OpenGL), written as BaseColor, ORM (occlusion, roughness, metallic) and Normal PNGs
    (Roughness, AO and Metallic are kept too).

    The hard parts are their own detail: they bake from their own materials. The soft parts (the
    game mesh's `soft` attribute) are projected from `soft`, their dense meshes (rays start
    `cage` outside the game mesh and reach 2 x cage in), so a strap or buckle lying on the cloth
    leaves no print on it where the cloth shows when the body bends."""
    scene = bpy.context.scene
    scene.cycles.samples = samples
    scene.cycles.seed = 7
    margin = max(4, resolution // 512)
    own = [slot.material for slot in low.material_slots]

    def image(kind: str, colorspace: str, float_buffer: bool = False):
        img = bpy.data.images.new(f"{prefix}_{kind}", resolution, resolution, alpha=False, float_buffer=float_buffer)
        img.colorspace_settings.name = colorspace
        _bake_target(low, img)
        return img

    def passes(source, projected: dict) -> dict:
        bpy.ops.object.select_all(action="DESELECT")
        low.hide_render = False
        low.select_set(True)
        if source is not low:
            source.hide_render = False
            source.select_set(True)
        bpy.context.view_layer.objects.active = low
        out = {}

        def run(kind, bake_type, colorspace, **kwargs):
            img = image(kind, colorspace, float_buffer=kind == "Normal")
            bpy.ops.object.bake(type=bake_type, margin=margin, **projected, **kwargs)
            out[kind] = np.array(img.pixels[:], dtype=np.float32).reshape(resolution, resolution, 4)
            bpy.data.images.remove(img)

        undo = _emit_from([source], lambda bsdf: bsdf.inputs["Base Color"])
        run("BaseColor", "EMIT", "sRGB")
        _restore(undo)
        run("Roughness", "ROUGHNESS", "Non-Color")
        run("AO", "AO", "Non-Color")
        undo = _emit_from([source], lambda bsdf: bsdf.inputs["Metallic"])
        run("Metallic", "EMIT", "Non-Color")
        _restore(undo)
        run("Normal", "NORMAL", "Non-Color", normal_space="TANGENT")
        source.select_set(False)
        return out

    # The hard parts from their own materials.
    pixels = passes(low, {"use_selected_to_active": False})
    # Which texels belong to the soft parts: the game mesh's own `soft` attribute, baked flat.
    mask_mat = bpy.data.materials.new(f"{prefix}_SoftMask")
    mask_mat.use_nodes = True
    nt = mask_mat.node_tree
    attr = nt.nodes.new("ShaderNodeAttribute")
    attr.attribute_name = "soft"
    emit = nt.nodes.new("ShaderNodeEmission")
    nt.links.new(attr.outputs["Fac"], emit.inputs["Color"])
    nt.links.new(emit.outputs[0], nt.nodes["Material Output"].inputs["Surface"])
    for slot in low.material_slots:
        slot.material = mask_mat
    img = image("SoftMask", "Non-Color")
    bpy.ops.object.select_all(action="DESELECT")
    low.select_set(True)
    bpy.context.view_layer.objects.active = low
    bpy.ops.object.bake(type="EMIT", margin=margin, use_selected_to_active=False)
    m = (np.array(img.pixels[:], dtype=np.float32).reshape(resolution, resolution, 4)[..., :1] > 0.5).astype(np.float32)
    bpy.data.images.remove(img)
    # The soft parts projected from their dense meshes.
    target = ka.flat_material(f"{prefix}_BakeTarget", "#000000", 0.5)
    for slot in low.material_slots:
        slot.material = target
    cloth = passes(soft, {"use_selected_to_active": True, "cage_extrusion": cage, "max_ray_distance": 2 * cage})
    for slot, mat in zip(low.material_slots, own):
        slot.material = mat
    for kind in pixels:
        pixels[kind] = m * cloth[kind] + (1.0 - m) * pixels[kind]

    paths = {}

    def save(kind: str, px, colorspace: str):
        img = bpy.data.images.new(f"{prefix}_{kind}_out", resolution, resolution, alpha=False)
        img.colorspace_settings.name = colorspace
        px = px.copy()
        px[..., 3] = 1.0
        img.pixels[:] = px.ravel()
        path = os.path.join(out_dir, f"{prefix}_{kind}.png")
        img.filepath_raw = path
        img.file_format = "PNG"
        img.save()
        paths[kind] = path

    save("BaseColor", pixels["BaseColor"], "sRGB")
    for kind in ("Roughness", "AO", "Metallic"):
        save(kind, pixels[kind], "Non-Color")
    nrm = pixels["Normal"].copy()
    empty = nrm[..., :3].max(-1) < 1e-4  # Texels no island covers: a flat normal, so mips stay clean
    nrm[empty, 0:3] = (0.5, 0.5, 1.0)
    save("Normal", nrm, "Non-Color")
    packed = np.ones((resolution, resolution, 4), dtype=np.float32)
    packed[..., 0] = pixels["AO"][..., 0]
    packed[..., 1] = pixels["Roughness"][..., 0]
    packed[..., 2] = pixels["Metallic"][..., 0]
    save("ORM", packed, "Non-Color")
    scene.render.bake.use_selected_to_active = False
    return paths

def heat_scored(name: str, color: str, spots, reach: float = 0.06, wear: float = 0.5):
    """Bare steel blued by heat round its hot spots (the Doubler's line exits): straw, then blue,
    then brown toward each spot over `reach` metres, roughness rising with the scale."""
    g = Graph(name)
    streak = g.noise(1.0, 3.0, 0.5, vector=g.stretch((900.0, 900.0, 12.0)))
    near = None
    for p in spots:
        d = g.vmath("DISTANCE", g.P, tuple(p))
        near = d if near is None else g.math("MINIMUM", near, d)
    t = g.math("DIVIDE", g.add(near, g.mul(g.noise(40.0, 3.0, 0.6), 0.012)), reach)
    ramp = g.node("ShaderNodeValToRGB")
    g.put(ramp.inputs["Fac"], t)
    els = ramp.color_ramp.elements
    els[0].position, els[0].color = 0.0, colour("#4A2E1E")
    els[1].position, els[1].color = 1.0, colour(color)
    for pos, c in ((0.33, "#3B4C7A"), (0.66, "#C9A24A"), (0.85, color)):
        e = els.new(pos)
        e.color = colour(c)
    e_ = g.edges(0.0018, 0.03, 0.3)
    base = g.mix(g.lerp(streak, 0.3, 0.7), shade(color, 0.88), shade(color, 1.06))
    scored = g.smooth(t, 1.0, 0.85)
    base = g.mix(scored, base, ramp.outputs["Color"])
    base = g.mix(g.mul(e_, 0.3), base, shade(color, 1.3))
    cav = g.cavity(0.02, 0.45, 0.95)
    base = g.mix(g.mul(cav, 0.6 + 0.3 * wear), base, "#24201C")
    rough = g.add(0.3, g.mul(g.mul(scored, g.math("SUBTRACT", 1.0, g.math("MINIMUM", t, 1.0))), 0.3),
                  g.mul(streak, 0.08), g.mul(cav, 0.3))
    return g.finish(base, rough, g.mixf(g.mul(cav, 0.5), 1.0, 0.4), g.mul(streak, 0.000015))
