"""Each class's gear fitted to its MetaHuman (art/out/metahumans/<Class>/, exported from Unreal),
and skinned to the MetaHuman body skeleton so Unreal can drive it with the body's pose (leader
pose). Run through harness.py:

    blender -b --factory-startup -P art/characters/harness.py -- --metahuman [--class Rigger] [--res 4096]

For each class it reads SKM_MH_<Class>_BodyMesh.fbx and SKM_MH_<Class>_FaceMesh.fbx and writes,
beside them, SK_MHGear_<Class>.fbx (one skeletal mesh on the MetaHuman skeleton: root and the
body's 341 bones, same names, hierarchy and bind pose), T_<Class>_MHGear_BaseColor/ORM/Normal,
a preview, and report.json.

What it fits:
- the work coverall: the body's own surface from the neck to the wrists and ankles, lifted off
  the skin by the cloth's ease and drawn taut over the hollows (the armpits, the small of the
  back, between the legs) as cloth is, then given the suit's folds, pockets, patches and seams
  (harness_body.Suit, harness_look.suit_fabric); a stand collar round the neck; cut clean at the
  wrists, ankles and neck with the hems turned in. Its weights are the body's, taken from the
  nearest point of the skin, so it moves with the skin;
- gloves over the hands (finger bones and all), cuffed over the sleeves; the Climber's taped palms;
- boots shaped as the class's boots (harness_body.Boot) and scaled round each MetaHuman foot,
  closing over the trouser; weighted from the foot, toes and calf under them;
- the headgear sized to the MetaHuman's head (from the face mesh);
- the harness (harness_parts) placed from the MetaHuman's bones and body, rigid on its bones;
  the straps take their weights from the coverall under them.

Everything is built in the gear's own frame (X forward, Y left: the MetaHuman frame turned a
quarter about Z) and turned back for export.
"""
from __future__ import annotations

import json
import math
import os

import bmesh
import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

import harness_sdf as sdf
import kinetic_art as ka

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "out", "metahumans")
TO_GEAR = Matrix.Rotation(math.pi / 2, 4, "Z")   # MetaHuman (forward -Y, left +X) to the gear's frame
TO_MH = TO_GEAR.inverted()
# The gear names three spine bones; the MetaHuman has five.
BONE_MAP = {"spine_01": "spine_02", "spine_02": "spine_04", "spine_03": "spine_05"}
FAMILIES = {
    "hand": ("hand", "thumb", "index", "middle", "ring", "pinky", "wrist"),
    "foot": ("foot", "ball", "bigtoe", "indextoe", "middletoe", "ringtoe", "littletoe", "ankle"),
    "head": ("head", "neck_02", "FACIAL"),
}


# Loading ---------------------------------------------------------------------------------------

def load(cls: str) -> dict:
    """The class's MetaHuman body and face meshes (world space, metres, the MetaHuman frame) and
    the body skeleton's bones (name, parent, world rest matrix, length) in hierarchy order."""
    folder = os.path.join(ROOT, cls)

    def imported(path):
        before = set(bpy.data.objects)
        bpy.ops.import_scene.fbx(filepath=path)
        new = [o for o in bpy.data.objects if o not in before]
        return next(o for o in new if o.type == "MESH"), next(o for o in new if o.type == "ARMATURE"), new

    body, arm, junk = imported(os.path.join(folder, f"SKM_MH_{cls}_BodyMesh.fbx"))
    bones = []
    for b in arm.data.bones:
        m = arm.matrix_world @ b.matrix_local
        loc, rot, _scale = m.decompose()
        head = arm.matrix_world @ b.head_local
        tail = arm.matrix_world @ b.tail_local
        bones.append((b.name, b.parent.name if b.parent else "root", Matrix.LocRotScale(loc, rot, None), (tail - head).length,
                      Vector(head), Vector(tail)))
    face, face_arm, junk2 = imported(os.path.join(folder, f"SKM_MH_{cls}_FaceMesh.fbx"))
    for o in (body, face):
        mw = o.matrix_world.copy()
        o.parent = None
        o.data.transform(mw)
        o.matrix_world = Matrix.Identity(4)
        for m in list(o.modifiers):
            o.modifiers.remove(m)
    for o in junk + junk2:
        if o not in (body, face):
            bpy.data.objects.remove(o)
    body.name, face.name = f"MH_Body_{cls}", f"MH_Face_{cls}"
    meta = json.load(open(os.path.join(folder, "metahuman.json")))
    return {"body": body, "face": face, "bones": bones, "meta": meta}


def build_rig(bones, name: str = "Armature"):
    """The MetaHuman skeleton as a Blender armature: a root at the origin and every body bone at
    its rest matrix (Unreal drops the node named Armature, so root is the skeleton's root)."""
    data = bpy.data.armatures.new(name)
    rig = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode="EDIT")
    root = data.edit_bones.new("root")
    root.head, root.tail, root.roll = (0, 0, 0), (0, 0.1, 0), 0.0  # Identity rest, as the body FBX's root
    for bname, parent, m, length, _h, _t in bones:
        eb = data.edit_bones.new(bname)
        eb.head, eb.tail = (0, 0, 0), (0, max(length, 0.002), 0)
        eb.matrix = m
        eb.length = max(length, 0.002)
    for bname, parent, *_ in bones:
        eb = data.edit_bones[bname]
        eb.parent = data.edit_bones[parent]
        eb.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    data.bones["root"].use_deform = False
    return rig


def gear_bones(bones) -> list:
    """The bone table the gear code is written for (harness.BONES' names and layout) from the
    MetaHuman's joints, in the gear's frame."""
    P = {b[0]: TO_GEAR @ b[4] for b in bones}
    T = {b[0]: TO_GEAR @ b[5] for b in bones}
    up = Vector((0, 0, 0.18))
    t = lambda v: tuple(v)
    out = [("root", None, (0, 0, 0), (0, 0, 0.1)),
           ("pelvis", "root", t(P["pelvis"]), t(P["spine_02"])),
           ("spine_01", "pelvis", t(P["spine_02"]), t(P["spine_04"])),
           ("spine_02", "spine_01", t(P["spine_04"]), t(P["spine_05"])),
           ("spine_03", "spine_02", t(P["spine_05"]), t(P["neck_01"])),
           ("neck_01", "spine_03", t(P["neck_01"]), t(P["head"])),
           ("head", "neck_01", t(P["head"]), t(P["head"] + up))]
    for s in "lr":
        ball = P[f"ball_{s}"]
        fwd = (ball - P[f"foot_{s}"])
        fwd.z = 0
        out += [(f"clavicle_{s}", "spine_03", t(P[f"clavicle_{s}"]), t(P[f"upperarm_{s}"])),
                (f"upperarm_{s}", f"clavicle_{s}", t(P[f"upperarm_{s}"]), t(P[f"lowerarm_{s}"])),
                (f"lowerarm_{s}", f"upperarm_{s}", t(P[f"lowerarm_{s}"]), t(P[f"hand_{s}"])),
                (f"hand_{s}", f"lowerarm_{s}", t(P[f"hand_{s}"]), t(P[f"middle_01_{s}"])),
                (f"thigh_{s}", "pelvis", t(P[f"thigh_{s}"]), t(P[f"calf_{s}"])),
                (f"calf_{s}", f"thigh_{s}", t(P[f"calf_{s}"]), t(P[f"foot_{s}"])),
                (f"foot_{s}", f"calf_{s}", t(P[f"foot_{s}"]), t(ball)),
                (f"ball_{s}", f"foot_{s}", t(ball), t(ball + fwd.normalized() * 0.07))]
    del T
    return out


# Weights on the MetaHuman skeleton ------------------------------------------------------------

def weights(obj, names: list) -> np.ndarray:
    index = {g.index: names.index(g.name) for g in obj.vertex_groups if g.name in names}
    W = np.zeros((len(obj.data.vertices), len(names)))
    for v in obj.data.vertices:
        for g in v.groups:
            if g.group in index:
                W[v.index, index[g.group]] += g.weight
    s = W.sum(1)
    W[s < 1e-9, names.index("pelvis")] = 1.0
    return W / W.sum(1)[:, None]


def set_weights(obj, W: np.ndarray, names: list, top: int = 8) -> None:
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)
    W = W.copy()
    if top < W.shape[1]:
        cut = np.sort(W, 1)[:, -top][:, None]
        W[W < cut] = 0.0
    W[W < 1e-3] = 0.0
    W /= np.maximum(W.sum(1), 1e-9)[:, None]
    for b, name in enumerate(names):
        col = W[:, b]
        idx = np.nonzero(col > 0)[0]
        if not len(idx):
            continue
        group = obj.vertex_groups.new(name=name)
        q = np.round(col[idx], 3)
        for level in np.unique(q):
            group.add(idx[q == level].tolist(), float(level), "REPLACE")


class Surface:
    """A mesh's surface for nearest-point queries: the point, its normal and the weights there
    (barycentric in the nearest triangle)."""

    def __init__(self, obj, W: np.ndarray | None = None):
        from mathutils.bvhtree import BVHTree
        me = obj.data
        me.calc_loop_triangles()
        tris = np.empty(len(me.loop_triangles) * 3, np.int32)
        me.loop_triangles.foreach_get("vertices", tris)
        self.tris = tris.reshape(-1, 3)
        self.co = sdf.verts_of(obj)
        a, b, c = (self.co[self.tris[:, k]] for k in range(3))
        n = np.cross(b - a, c - a)
        self.fn = n / np.maximum(np.linalg.norm(n, axis=1), 1e-12)[:, None]
        vn = np.zeros_like(self.co)
        for k in range(3):
            np.add.at(vn, self.tris[:, k], n)
        self.vn = vn / np.maximum(np.linalg.norm(vn, axis=1), 1e-12)[:, None]
        self.tree = BVHTree.FromPolygons(self.co.tolist(), self.tris.tolist())
        self.W = W

    def nearest(self, points):
        """For each point: the nearest surface point, the smooth normal there, the signed
        distance (positive outside) and the barycentric weights' triangle and coordinates."""
        n = len(points)
        loc = np.zeros((n, 3))
        tri = np.zeros(n, np.int64)
        for i, p in enumerate(points):
            hit = self.tree.find_nearest(Vector(p))
            if hit[0] is None:
                loc[i] = p
                continue
            loc[i] = hit[0]
            tri[i] = hit[2]
        a, b, c = (self.co[self.tris[tri, k]] for k in range(3))
        v0, v1, v2 = b - a, c - a, loc - a
        d00, d01, d11 = sdf.dot(v0, v0), sdf.dot(v0, v1), sdf.dot(v1, v1)
        d20, d21 = sdf.dot(v2, v0), sdf.dot(v2, v1)
        den = np.maximum(d00 * d11 - d01 * d01, 1e-18)
        wv = (d11 * d20 - d01 * d21) / den
        ww = (d00 * d21 - d01 * d20) / den
        bary = np.clip(np.stack([1 - wv - ww, wv, ww], 1), 0, 1)
        bary /= bary.sum(1)[:, None]
        normal = sum(bary[:, k, None] * self.vn[self.tris[tri, k]] for k in range(3))
        normal /= np.maximum(np.linalg.norm(normal, axis=1), 1e-12)[:, None]
        dist = sdf.dot(points - loc, self.fn[tri])
        return loc, normal, dist, tri, bary

    def weights_at(self, points):
        _loc, _n, _d, tri, bary = self.nearest(points)
        out = sum(bary[:, k, None] * self.W[self.tris[tri, k]] for k in range(3))
        return out / out.sum(1)[:, None]


# The soft pieces ----------------------------------------------------------------------------------

def family_weight(W, names, family: str):
    keys = FAMILIES[family]
    cols = [i for i, n in enumerate(names) if any(n == k or n.startswith(k + "_") for k in keys)]
    return W[:, cols].sum(1)


def weld(obj, distance: float = 1e-4) -> None:
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=distance)
    bm.to_mesh(obj.data)
    bm.free()


def copy_mesh(obj, name: str):
    c = obj.copy()
    c.data = obj.data.copy()
    c.name = c.data.name = name
    bpy.context.scene.collection.objects.link(c)
    return c


ARM = ("upperarm", "lowerarm", "hand", "thumb", "index", "middle", "ring", "pinky", "wrist")
LEG = ("thigh", "calf", "foot", "ball", "bigtoe", "indextoe", "middletoe", "ringtoe", "littletoe", "ankle")


def chain_mask(obj, names, chain, side: str):
    """The vertices a limb (arm or leg, one side) mostly moves."""
    W = weights(obj, names)
    cols = [i for i, n in enumerate(names) if n.endswith("_" + side) and any(n == f"{c}_{side}" or n.startswith(c + "_") for c in chain)]
    hit = W[:, cols].sum(1) > 0.5
    return lambda co: hit


def bisect(obj, select, plane_co, plane_no) -> None:
    """Cuts away what lies past a plane, among the faces whose vertices all pass `select`."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    co = np.array([v.co for v in bm.verts])
    keep = select(co)
    faces = [f for f in bm.faces if all(keep[v.index] for v in f.verts)]
    edges = list({e for f in faces for e in f.edges})
    verts = list({v for f in faces for v in f.verts})
    bmesh.ops.bisect_plane(bm, geom=faces + edges + verts, dist=1e-6, plane_co=plane_co, plane_no=plane_no,
                           clear_outer=True)
    # Anything left hanging off the cut (a sliver of one face) goes too.
    loose = [v for v in bm.verts if not v.link_faces]
    bmesh.ops.delete(bm, geom=loose, context="VERTS")
    bm.to_mesh(obj.data)
    bm.free()


def subdivide(obj, levels: int) -> None:
    mod = obj.modifiers.new("Subdivision", "SUBSURF")
    mod.levels = mod.render_levels = levels
    mod.boundary_smooth = "PRESERVE_CORNERS"
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=mod.name)


def neighbours(obj):
    me = obj.data
    e = np.empty(len(me.edges) * 2, np.int64)
    me.edges.foreach_get("vertices", e)
    e = e.reshape(-1, 2)
    return np.concatenate([e[:, 0], e[:, 1]]), np.concatenate([e[:, 1], e[:, 0]])


def drape(obj, skin: Surface, ease, iterations: int = 14, tension: float = 0.45, fixed=None):
    """Lifts a copy of the skin off it by `ease(points)` metres and draws it taut, as cloth is:
    Laplacian steps pull it across the hollows, and it is never let closer to the skin than its
    ease. `fixed` vertices (the open edges) only take the push."""
    v = sdf.verts_of(obj)
    src, dst = neighbours(obj)
    count = np.bincount(src, minlength=len(v)).astype(float)
    loc, nrm, dist, _t, _b = skin.nearest(v)
    c = ease(loc)
    v = loc + nrm * c[:, None]
    free = np.ones(len(v)) if fixed is None else (~fixed).astype(float)
    for _ in range(iterations):
        mean = np.stack([np.bincount(src, weights=v[dst, k], minlength=len(v)) for k in range(3)], 1)
        mean /= np.maximum(count, 1)[:, None]
        v = v + (tension * free)[:, None] * (mean - v)
        loc, nrm, dist, _t, _b = skin.nearest(v)
        c = ease(loc)
        along = sdf.dot(v - loc, nrm)
        short = (along < c) | (dist < 0)
        v[short] = loc[short] + nrm[short] * c[short, None]
    sdf.set_verts(obj, v)
    return v


def vertex_normals(obj):
    obj.data.update()
    n = np.empty(len(obj.data.vertices) * 3, np.float32)
    obj.data.vertices.foreach_get("normal", n)
    return n.reshape(-1, 3).astype(float)


def boundary_verts(obj) -> np.ndarray:
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    out = np.zeros(len(bm.verts), bool)
    for e in bm.edges:
        if e.is_boundary:
            for v in e.verts:
                out[v.index] = True
    bm.free()
    return out


def smooth_edges(obj, iterations: int = 4) -> None:
    """Evens out an open edge's loop (a bisected cut leaves it stepped)."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    ring = [v for v in bm.verts if any(e.is_boundary for e in v.link_edges)]
    for _ in range(iterations):
        new = {}
        for v in ring:
            nb = [e.other_vert(v) for e in v.link_edges if e.is_boundary]
            if len(nb) == 2:
                new[v] = v.co * 0.5 + (nb[0].co + nb[1].co) * 0.25
        for v, c in new.items():
            v.co = c
    bm.to_mesh(obj.data)
    bm.free()


def hem(obj, depth: float = 0.006) -> None:
    """Turns the open edges in: a band from each edge back under the cloth, so a cuff or a collar
    shows the fabric's thickness instead of a paper edge."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bm.normal_update()
    edges = [e for e in bm.edges if e.is_boundary]
    if not edges:
        bm.free()
        return
    normals = {v: v.normal.copy() for e in edges for v in e.verts}
    ret = bmesh.ops.extrude_edge_only(bm, edges=edges)
    new = [g for g in ret["geom"] if isinstance(g, bmesh.types.BMVert)]
    for v in new:
        src = next((e.other_vert(v) for e in v.link_edges if e.other_vert(v) in normals), None)
        if src is not None:
            v.co = src.co - normals[src] * depth
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(obj.data)
    bm.free()


def coverall(spec, suit, skin: Surface, names, body_W, gb: dict, mats, top_neck: float, collide: Surface | None = None):
    """The coverall: the skin from the neck to the wrists and ankles, draped, cut and hemmed;
    the dense mesh with its folds and pockets, and the game mesh with the skin's weights. With
    `collide` (the body joined to the face mesh's neck and shoulder flap), it is cut from and
    draped over that, so it closes up to the neck where the face mesh takes over from the body."""
    body = collide.obj if collide is not None else skin.obj
    cloth = copy_mesh(body, "HIGH_Suit")
    W = skin.weights_at(sdf.verts_of(cloth)) if collide is not None else body_W

    def probe(stage):
        if os.environ.get("KINETIC_DEBUG"):
            c = sdf.verts_of(cloth)
            r = (np.abs(c[:, 1]) > 0.1) & (np.abs(c[:, 1]) < 0.2) & (c[:, 2] > 1.42) & (c[:, 2] < 1.52)
            print("probe", stage, len(c), r.sum(), "ztop", round(float(c[:, 2].max()), 3), "top_neck", round(top_neck, 3))
    probe("source")
    hand = family_weight(W, names, "hand")
    foot = family_weight(W, names, "foot")
    headw = family_weight(W, names, "head")
    # Drop what the coverall never covers (well past the cuts), then cut clean.
    bm = bmesh.new()
    bm.from_mesh(cloth.data)
    gone = [v for v in bm.verts if hand[v.index] > 0.85 or foot[v.index] > 0.85 or headw[v.index] > 0.6 or v.co.z > top_neck + 0.06]
    bmesh.ops.delete(bm, geom=gone, context="VERTS")
    bm.to_mesh(cloth.data)
    bm.free()
    probe("dropped")
    for s, side in ((1.0, "l"), (-1.0, "r")):
        e, w = Vector(gb[f"lowerarm_{side}"][0]), Vector(gb[f"lowerarm_{side}"][1])
        axis = (w - e).normalized()
        at = e + (w - e) * 0.955
        bisect(cloth, chain_mask(cloth, names, ARM, side), at, axis)
        a, f = Vector(gb[f"calf_{side}"][0]), Vector(gb[f"calf_{side}"][1])
        cut = f + Vector((0, 0, 0.045))
        bisect(cloth, chain_mask(cloth, names, LEG, side), cut, (f - a).normalized())
    probe("limbs cut")
    bisect(cloth, lambda co: (co[:, 2] > top_neck - 0.08) & (np.abs(co[:, 1]) < 0.11), Vector((0, 0, top_neck)), Vector((0, 0, 1)))
    probe("neck cut")
    subdivide(cloth, 2)
    ease_k = spec.get("suit", {}).get("ease", 1.0)

    def ease(p):
        z = p[:, 2]
        base = 0.0065 + 0.0035 * ease_k
        torso = sdf.smoothstep(1.4, 1.2, z) * sdf.smoothstep(0.8, 0.95, z) * sdf.smoothstep(0.2, 0.14, np.abs(p[:, 1]))
        legs = sdf.smoothstep(0.75, 0.3, z) * sdf.smoothstep(0.08, 0.13, z)
        return base + 0.006 * ease_k * torso + 0.008 * ease_k * legs

    smooth_edges(cloth, 6)
    edge = boundary_verts(cloth)
    drape(cloth, collide or skin, ease, iterations=14, fixed=edge)
    if os.environ.get("KINETIC_DEBUG"):
        t = skin.co[skin.tris]
        vol = np.einsum("ij,ij->i", t[:, 0], np.cross(t[:, 1], t[:, 2])).sum() / 6
        _l, _n, dd, _t, _b = skin.nearest(sdf.verts_of(cloth))
        print("skin volume", round(vol, 4), "cloth verts under ease", (dd < ease(_l) - 1e-4).sum(), "inside skin", (dd < 0).sum(), len(dd))
    v = sdf.verts_of(cloth)
    n = vertex_normals(cloth)
    v = v + n * suit.surface(v)[:, None]
    sdf.set_verts(cloth, v)
    smooth_edges(cloth, 3)
    hem(cloth, 0.005)
    for p in cloth.data.polygons:
        p.use_smooth = True
    v = sdf.verts_of(cloth)
    grime, sweat = suit.grime(v, gb["table"])
    sdf.set_attribute(cloth, "grime", grime)
    sdf.set_attribute(cloth, "sweat", sweat)
    for o in [cloth]:
        o.data.materials.clear()
        o.data.materials.append(mats["Suit"])
    low = sdf.decimated(cloth, 32000, "Suit")
    set_weights(low, skin.weights_at(sdf.verts_of(low)), names)
    if os.environ.get("KINETIC_BLEND"):
        bpy.ops.wm.save_as_mainfile(filepath=os.environ["KINETIC_BLEND"], copy=True)
    if os.environ.get("KINETIC_DEBUG"):
        cover = Surface(low)
        bh, bf, bhd = (family_weight(body_W, names, k) for k in ("hand", "foot", "head"))
        sk = skin.co[(bh < 0.3) & (bf < 0.3) & (bhd < 0.3) & (skin.co[:, 2] < top_neck - 0.03) & (skin.co[:, 2] > 0.2)]
        _l, _n, d, _t, _b = cover.nearest(sk)
        out_ = sk[d > -0.002]
        print("skin outside the coverall:", len(out_), "of", len(sk))
        for z0 in np.arange(0.2, 1.5, 0.1):
            band = out_[(out_[:, 2] >= z0) & (out_[:, 2] < z0 + 0.1)]
            if len(band):
                print(f"  z {z0:.1f}: {len(band)}", np.round(band[:6], 3).tolist(), "depth", np.round(d[d > -0.002][(out_[:, 2] >= z0) & (out_[:, 2] < z0 + 0.1)][:6], 4).tolist())
    return low, cloth


def collar(skin: Surface, names, top_neck: float, mats, neck_pts=None):
    """The stand collar: a short band round the neck at the coverall's top edge."""
    co = skin.co if neck_pts is None else np.concatenate([skin.co, neck_pts])
    ring = co[(np.abs(co[:, 2] - (top_neck - 0.01)) < 0.012) & (np.abs(co[:, 1]) < 0.1)]
    c = ring[:, :2].mean(0)
    rx = np.abs(ring[:, 0] - c[0]).max() + 0.011
    ry = np.abs(ring[:, 1] - c[1]).max() + 0.011
    centre = np.array((c[0], c[1], top_neck + 0.006))

    def field(p):
        q = p - centre
        radial = np.sqrt((q[:, 0] / rx) ** 2 + (q[:, 1] / ry) ** 2) * min(rx, ry)
        wall = np.abs(radial - min(rx, ry)) - 0.0035
        height = np.abs(q[:, 2]) - 0.024 + 0.014 * sdf.smoothstep(0.0, rx, q[:, 0])
        band = sdf.smax(wall, height, 0.003)
        gap = 0.011 - np.abs(q[:, 1]) + 1.0 * (q[:, 0] < 0.0)
        return sdf.smax(band, gap, 0.0025)

    lo = centre - np.array((rx + 0.02, ry + 0.02, 0.04))
    hi = centre + np.array((rx + 0.02, ry + 0.02, 0.04))
    v, q = sdf.mesh(field, lo, hi, 0.0015)
    v = sdf.refine(field, v, 2)
    high = sdf.to_object("HIGH_Collar", v, q)
    high.data.materials.append(mats["Suit"])
    low = sdf.decimated(high, 1600, "Collar")
    set_weights(low, skin.weights_at(sdf.verts_of(low)), names)
    return low, high


def gloves(spec, skin: Surface, names, body_W, gb: dict, mats):
    """Gloves over the hands, cuffed over the sleeves (the Climber: tape round the palm)."""
    out = []
    taped = spec["glove"] is None
    for s, side in ((1.0, "l"), (-1.0, "r")):
        glove = copy_mesh(skin.obj, f"HIGH_Glove_{side}")
        e, w = Vector(gb[f"lowerarm_{side}"][0]), Vector(gb[f"lowerarm_{side}"][1])
        axis = (w - e).normalized()
        length = (w - e).length
        hand = family_weight(body_W, names, "hand")
        co = skin.co
        t = (co - np.array(e)) @ np.array(axis) / length
        fingers = sum(family_weight_cols(body_W, names, k) for k in ("thumb_02", "thumb_03", "index_0", "middle_0", "ring_0", "pinky_0"))
        arm = chain_mask(skin.obj, names, ARM, side)(co)
        keep = arm & ((hand > 0.3) | (t > 0.8))
        if taped:
            keep &= fingers < 0.5
        bm = bmesh.new()
        bm.from_mesh(glove.data)
        bm.verts.ensure_lookup_table()
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if not keep[v.index]], context="VERTS")
        bm.to_mesh(glove.data)
        bm.free()
        cuff_at = e + (w - e) * (0.86 if not taped else 0.9)
        bisect(glove, lambda c: np.ones(len(c), bool), cuff_at, -axis)
        subdivide(glove, 1)

        def ease(p, e=np.array(e), a=np.array(axis), L=length):
            tt = (p - e) @ a / L
            over_sleeve = sdf.smoothstep(0.985, 0.94, tt)
            return 0.0022 + (0.0007 if taped else 0.0012) + 0.0115 * over_sleeve

        smooth_edges(glove, 6)
        edge = boundary_verts(glove)
        drape(glove, skin, ease, iterations=3, tension=0.25, fixed=edge)
        v = sdf.verts_of(glove)
        n = vertex_normals(glove)
        grain = 0.0003 * (sdf.fbm(v, 300.0, 2, seed=7) - 0.5)
        sdf.set_verts(glove, v + n * grain[:, None])
        hem(glove, 0.004)
        for p in glove.data.polygons:
            p.use_smooth = True
        glove.data.materials.clear()
        glove.data.materials.append(mats["Strap" if taped else "Glove"])
        low = sdf.decimated(glove, 2400 if taped else 5200, f"Glove_{side}")
        set_weights(low, skin.weights_at(sdf.verts_of(low)), names)
        out.append((low, glove))
    return out


def family_weight_cols(W, names, prefix: str):
    cols = [i for i, n in enumerate(names) if n.startswith(prefix)]
    return W[:, cols].sum(1) if cols else np.zeros(len(W))


def boots(spec, suit, skin: Surface, trouser: Surface, names, body_W, gb: dict, mats):
    """Each class's boots shaped round each MetaHuman foot: the canonical boot (harness_body.Boot)
    turned to the foot, scaled to its length and breadth and grown until the whole foot is inside
    it, the shaft closing round the trouser; weighted from the skin under it."""
    import harness_body as hb
    out, made = [], {}
    foot_w = family_weight(body_W, names, "foot")
    for (s, side), style in zip(((1.0, "l"), (-1.0, "r")), spec["boots"]):
        ankle = np.array(gb[f"foot_{side}"][0])
        ball = np.array(gb[f"ball_{side}"][0])
        fwd = ball - ankle
        fwd[2] = 0
        fwd /= np.linalg.norm(fwd)
        left = np.cross((0, 0, 1.0), fwd)
        R = np.stack([fwd, left, (0, 0, 1.0)], 1)
        pts = skin.co[(foot_w > 0.3) & (skin.co[:, 1] * s > 0.0) & (skin.co[:, 2] < 0.16)]
        along = (pts - ankle) @ fwd
        across = (pts - ankle) @ left
        heel, toe = along.min(), along.max()
        width = across.max() - across.min()
        mid = (across.max() + across.min()) / 2
        kx = (toe - heel + 0.03) / 0.33
        ky = (width + 0.018) / 0.105
        kz = 1.0
        y0 = 0.1 * s
        k = np.array((kx, ky, kz))
        # Canonical (0, y0, 0) goes under the ankle at the foot's middle; the heel's back (-0.09)
        # to 1.2 cm behind the MetaHuman's heel.
        base = ankle + left * mid
        base[2] = -0.004
        origin = base + fwd * (heel - 0.012 + 0.09 * kx) - R @ (np.array((0.0, y0, 0.0)) * k)
        calf_a, calf_b = np.array(gb[f"calf_{side}"][0]), np.array(gb[f"calf_{side}"][1])

        leg_pts = skin.co[(foot_w < 0.3) & (skin.co[:, 1] * s > 0.02)]

        def fit(z, side=side, s=s, calf_a=calf_a, calf_b=calf_b, ky=ky):
            # The leg and trouser's half-width round the calf's axis at height z, in boot units
            # (none below the ankle: the foot is the last's to hold).
            if z < ankle[2] + 0.01:
                return 0.0
            src = trouser.co[trouser.co[:, 1] * s > 0.02] if z > ankle[2] + 0.03 else leg_pts
            band = src[np.abs(src[:, 2] - z) < 0.012]
            if not len(band):
                return 0.0
            t = (z - calf_a[2]) / (calf_b[2] - calf_a[2])
            c = calf_a + (calf_b - calf_a) * t
            r = np.hypot(band[:, 0] - c[0], band[:, 1] - c[1]).max()
            return (r + 0.006) / ky

        for _ in range(0 if os.environ.get("KINETIC_DEBUG") else 10):
            boot = hb.Boot(side, spec, suit, style, place=(origin, R, k), fit=fit, low_widest=True)
            inside = contained(boot, pts)
            if inside.max() < -0.0012:
                break
            k = k * np.array((1.01, 1.04, 1.02))
            origin = base + fwd * (heel - 0.012 + 0.09 * k[0]) - R @ (np.array((0.0, y0, 0.0)) * k)
        boot = hb.Boot(side, spec, suit, style, place=(origin, R, k), fit=fit, low_widest=True)
        inside = contained(boot, pts)
        print(f"boot {side}: scale {np.round(k, 3)}, foot {toe - heel:.3f} x {width:.3f} m, worst {inside.max():.4f}")
        if os.environ.get("KINETIC_DEBUG"):
            bad = inside > -0.003
            print("  poking", bad.sum(), "of", len(pts), "local", np.round(boot.local(pts[bad])[:12], 3).tolist())
            print("  ankle", np.round(ankle, 3), "heel/toe", round(heel, 3), round(toe, 3))
        made[side] = boot
        lo, hi = boot.bounds()
        v, q = sdf.mesh(boot.upper, lo, hi, 0.0022)
        v = sdf.refine(boot.upper, v, 2)
        high = sdf.to_object(f"HIGH_Boot_{side}", v, q)
        slo, shi = boot.bounds(top=boot.sole_h + 0.004)
        sv, sq = sdf.mesh(boot.sole, slo, shi, 0.0013)
        sv = sdf.refine(boot.sole, sv, 2)
        shigh = sdf.to_object(f"HIGH_Sole_{side}", sv, sq)
        low = sdf.decimated(high, 4200 if style == "tall" else 3400, f"Boot_{side}")
        slow = sdf.decimated(shigh, 1800, f"Sole_{side}")
        for o in (high, low):
            if style in ("climb", "hitop"):
                o.data.materials.append(mats["Upper" if style == "climb" else "Canvas"])
                o.data.materials.append(mats["Rand" if style == "climb" else "Foxing"])
                cen = np.array([boot.local(sdf.verts_of(o)[list(p.vertices)]).mean(0) for p in o.data.polygons])
                band = cen[:, 2] < boot.sole_h + (0.012 if style == "climb" else 0.016)
                toe_ = (cen[:, 0] > 0.165) & (cen[:, 2] < boot.sole_h + 0.05)
                o.data.polygons.foreach_set("material_index", (band | toe_).astype(np.int32))
            else:
                o.data.materials.append(mats["Upper"])
        for o in (shigh, slow):
            o.data.materials.append(mats["Sole"])
        foot_only = Surface(skin.obj, body_W)
        set_weights(low, foot_only.weights_at(sdf.verts_of(low)), names)
        set_weights(slow, foot_only.weights_at(sdf.verts_of(slow)), names)
        out += [(low, high), (slow, shigh)]
    return out, made


def contained(boot, pts):
    """How far each foot point is outside its boot (negative: inside). The sole of the foot rests
    on the boot's sole, where the upper meets it, so the bottom 6 mm are not tested."""
    d = np.minimum(boot.upper(pts), boot.sole(pts))
    low = boot.local(pts)[:, 2] < boot.sole_h + 0.006
    return np.where(low, -1.0, d)


def head_fit(face, gb: dict) -> dict:
    """The MetaHuman's cranium from its face mesh: centre, radii, the eye's height and the brow's
    reach (the gear's frame)."""
    co = sdf.verts_of(face)
    top = co[:, 2].max()
    eye_z = top - 0.115
    cap = co[co[:, 2] > eye_z + 0.005]
    lo, hi = cap.min(0), cap.max(0)
    centre = (lo + hi) / 2
    centre[2] = eye_z + 0.005
    radii = (hi - lo) / 2
    radii[2] = top - centre[2]
    band = co[np.abs(co[:, 2] - eye_z) < 0.01]
    return {"centre": centre, "radii": radii, "eye_z": eye_z, "front": band[:, 0].max(), "top": top}


# Posing (the checks) --------------------------------------------------------------------------

def pose(rig, angles: dict, pelvis_drop: float = 0.0) -> None:
    """Rotations about the gear frame's axes (degrees: X forward, Y left, Z up), as harness.pose
    takes them, applied to the MetaHuman skeleton (whose armature space is the MetaHuman frame)."""
    for pb in rig.pose.bones:
        pb.rotation_mode = "QUATERNION"
        pb.rotation_quaternion = Quaternion()
        pb.location = Vector()
    to_mh = TO_MH.to_3x3()
    for bone, (ax, ay, az) in angles.items():
        if bone not in rig.pose.bones:
            continue
        pb = rig.pose.bones[bone]
        rest = pb.bone.matrix_local.to_3x3()
        world = (Matrix.Rotation(math.radians(az), 3, to_mh @ Vector((0, 0, 1))) @
                 Matrix.Rotation(math.radians(ay), 3, to_mh @ Vector((0, 1, 0))) @
                 Matrix.Rotation(math.radians(ax), 3, to_mh @ Vector((1, 0, 0))))
        pb.rotation_quaternion = (rest.inverted() @ world @ rest).to_quaternion()
    if pelvis_drop:
        pb = rig.pose.bones["pelvis"]
        pb.location = pb.bone.matrix_local.to_3x3().inverted() @ Vector((0, 0, -pelvis_drop))


POSES = {  # The six takes' kind of motion, on the MetaHuman's A-pose rest
    "rest": ({}, 0.0),
    "run": ({"spine_02": (0, 4, 0), "spine_04": (0, 5, 0), "spine_05": (0, 0, 8),
             "thigh_l": (0, -38, 0), "thigh_r": (0, 38, 0), "calf_l": (0, 15, 0), "calf_r": (0, 60, 0),
             "upperarm_l": (-38, 30, 0), "upperarm_r": (38, -30, 0), "lowerarm_l": (0, -60, 0), "lowerarm_r": (0, -60, 0)}, 0.02),
    "brace": ({"thigh_l": (-12, -45, 0), "thigh_r": (12, -45, 0), "calf_l": (0, 80, 0), "calf_r": (0, 80, 0),
               "foot_l": (0, -35, 0), "foot_r": (0, -35, 0), "spine_02": (0, 12, 0), "spine_04": (0, 10, 0),
               "upperarm_r": (30, -40, 0), "lowerarm_r": (0, -70, 0), "upperarm_l": (-25, -30, 0), "lowerarm_l": (0, -50, 0)}, 0.24),
    "climb": ({"upperarm_l": (95, -15, 0), "upperarm_r": (-95, -15, 0), "lowerarm_l": (0, -25, 0), "lowerarm_r": (0, -25, 0),
               "thigh_l": (0, -60, 0), "thigh_r": (0, -25, 0), "calf_l": (0, 75, 0), "calf_r": (0, 50, 0), "spine_02": (0, 5, 0)}, 0.0),
    "stagger": ({"spine_02": (0, -12, 0), "spine_04": (0, -10, 4), "spine_05": (0, -6, 4), "upperarm_l": (30, 10, 0),
                 "upperarm_r": (-30, 10, 0), "thigh_l": (0, -15, 0), "calf_l": (0, 25, 0)}, 0.03),
}


# The build ----------------------------------------------------------------------------------

def build(cls: str, res: int = 4096, preview: bool = True) -> dict:
    import harness as H
    import harness_body as hb
    import harness_look as hl
    import harness_parts as hp

    spec = H.CLASSES[cls]
    ka.reset_scene()
    device = hl.use_gpu()
    mh = load(cls)
    body, face, bones = mh["body"], mh["face"], mh["bones"]
    names = [b[0] for b in bones]
    rig = build_rig(bones)
    # Work in the gear's frame: copies of the body and face turned a quarter about Z.
    work = copy_mesh(body, "Skin")
    weld(work)
    work.data.transform(TO_GEAR)
    head_mesh = copy_mesh(face, "Head")
    head_mesh.data.transform(TO_GEAR)
    body_W = weights(work, names)
    skin = Surface(work, body_W)
    skin.obj = work
    table = gear_bones(bones)
    gb = {b[0]: (b[2], b[3]) for b in table}
    gb["table"] = table
    bone_frame = lambda name: (Vector(gb[name][0]), Vector(gb[name][1]))
    suit = hb.Suit(spec, table, apose=False)
    suit.boot_top = gb["foot_l"][0][2] + 0.04  # The trousers stop above the ankle, inside the boots
    # The body mesh stops at the shoulder line: the face mesh carries the neck and the tops of the
    # shoulders, so the coverall is cut from both and closes at the neck.
    top_neck = gb["neck_01"][0][2] + 0.018
    layout = suit.layout(table)
    co = skin.co
    chest = co[(np.abs(co[:, 2] - 1.25) < 0.02) & (np.abs(co[:, 1]) < 0.12)]
    layout["side_x"] = float((chest[:, 0].max() + chest[:, 0].min()) / 2)
    layout["hems"] = []
    mats = H.materials(cls, spec)
    mats["Suit"] = hl.suit_fabric(f"M_{cls}_Suit", spec, layout)
    pieces_low, pieces_high = [], []
    # The face mesh's neck and the flap of it that lies over the shoulders: the coverall goes over
    # those too (its weights still come from the body alone).
    flap = copy_mesh(head_mesh, "Flap")
    bm = bmesh.new()
    bm.from_mesh(flap.data)
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if v.co.z > top_neck + 0.03], context="VERTS")
    bm.to_mesh(flap.data)
    bm.free()
    both = copy_mesh(work, "Collide")
    both = hl.join("Collide", [both, flap]) if len(flap.data.vertices) else both
    weld(both, 0.0015)
    # The cloth is pushed off a smooth version of the skin: the MetaHuman's game mesh is faceted,
    # and pushing off facets quilts the cloth.
    smooth_skin = copy_mesh(both, "CollideSmooth")
    subdivide(smooth_skin, 2)
    collide = Surface(smooth_skin)
    collide.obj = both
    low, high = coverall(spec, suit, skin, names, body_W, gb, mats, top_neck, collide)
    pieces_low.append(low)
    pieces_high.append(high)
    trouser = Surface(low)
    cl, ch = collar(skin, names, top_neck, mats, sdf.verts_of(head_mesh))
    pieces_low.append(cl)
    pieces_high.append(ch)
    for gl, gh in gloves(spec, skin, names, body_W, gb, mats):
        pieces_low.append(gl)
        pieces_high.append(gh)
    made, boots_ = boots(spec, suit, skin, trouser, names, body_W, gb, mats)
    for bl, bh in made:
        pieces_low.append(bl)
        pieces_high.append(bh)
    # The harness, placed from the MetaHuman's bones and fitted to the coverall.
    parts = hp.Parts(mats)
    parts.bone_frame = bone_frame
    parts.bone_map = BONE_MAP
    parts.set_body(low)
    head = head_fit(head_mesh, gb)
    gear = H.harness_parts_on(parts, spec, boots_, head)
    # Soft gear (straps, laces) takes its weights from the cloth or boot under it.
    sources = {"Suit": low, **{b.name: b for b, _h in made if b.name.startswith("Boot_")}}
    cache = {}
    for obj in parts.skinned:
        src = sources.get(obj.get("skin_from", "Suit"), low)
        if src.name not in cache:
            cache[src.name] = Surface(src, weights(src, names))
        W = cache[src.name].weights_at(sdf.verts_of(obj))
        if obj.get("skin_bones"):
            keep = np.zeros(len(names), bool)
            for b in obj["skin_bones"]:
                b = BONE_MAP.get(b, b)
                keep |= np.array([n == b or n.startswith(b + "_twist") or n.startswith(b + "_correct") for n in names])
            W2 = W * keep
            W = np.where(W2.sum(1)[:, None] > 1e-6, W2, W)
        set_weights(obj, W, names)
    del gear
    for piece in pieces_low:
        sdf.set_attribute(piece, "soft", np.ones(len(piece.data.vertices)))
    soft = hl.join(f"HIGH_Soft_{cls}", pieces_high)
    name = f"SK_MHGear_{cls}"
    mesh = hl.join(name, pieces_low + parts.rigid)
    for o in (mesh, soft):
        ka.triangulate(o)
    # Bake in the gear's frame (its materials' wear is laid out in it), then turn it into the
    # MetaHuman's frame onto its skeleton.
    out = os.path.join(ROOT, cls)
    team = [p.index for p in mesh.data.polygons if mesh.data.materials[p.material_index].name.endswith(H.TEAM_SLOT)]
    while mesh.data.uv_layers:
        mesh.data.uv_layers.remove(mesh.data.uv_layers[0])
    mesh.data.uv_layers.new(name="UVMap")
    ka.smart_uv(mesh, margin=0.002)
    tint = mats[H.TEAM_SLOT]
    stand_in = ka.flat_material(f"M_{cls}_TeamBake", "#E69F00", 0.3)
    for slot in mesh.material_slots:
        if slot.material == tint:
            slot.material = stand_in
    for o in (work, head_mesh, both, smooth_skin):
        bpy.data.objects.remove(o)
    for o in (body, face):  # Not in the bake: the gear's own occlusion only
        o.hide_render = True
    maps = hl.bake(mesh, soft, out, f"T_{cls}_MHGear", resolution=res, samples=16)
    for o in (body, face):
        o.hide_render = False
    bpy.data.objects.remove(soft)
    mesh.data.transform(TO_MH)
    baked = ka.baked_material(f"M_{cls}_Harness", maps)
    mesh.data.materials.clear()
    mesh.data.materials.append(baked)
    mesh.data.materials.append(tint)
    team = set(team)
    for p in mesh.data.polygons:
        p.material_index = 1 if p.index in team else 0
    mesh.parent = rig
    mod = mesh.modifiers.new("Armature", "ARMATURE")
    mod.object = rig
    export(os.path.join(out, f"{name}.fbx"), rig, mesh)
    before = match_bind(os.path.join(out, f"{name}.fbx"), os.path.join(out, f"SKM_MH_{cls}_BodyMesh.fbx"))
    print(f"{name}: bind pose set to the body FBX's (it differed by up to {before['pose']:.2e} before)")
    report = {
        "name": name,
        "class": cls,
        "triangles": len(mesh.data.polygons),
        "skeleton": mh["meta"]["body"]["skeleton"],
        "bones": ["root"] + names,
        "textures": {k: os.path.basename(v) for k, v in maps.items() if k in ("BaseColor", "ORM", "Normal")},
        "texture_size": res,
        "normal_map": "tangent space, OpenGL convention (green up): flip green in Unreal",
        "slots": [f"M_{cls}_Harness", f"M_{cls}_{H.TEAM_SLOT}"],
        "follows": f"SKM_MH_{cls}_BodyMesh (leader pose)",
        "hide_under_gear": ["torso", "arms to the wrists", "legs", "hands", "feet"],
        "bake_device": device,
    }
    with open(os.path.join(out, "report.json"), "w") as f:
        json.dump(report, f, indent=2)
    if preview:
        prepare_preview(body, face, rig, names, spec["skin"])
        render(os.path.join(out, f"{name}_preview.png"), [("rest", ("front", "three"))])
        if os.environ.get("KINETIC_POSES"):  # The deformation checks: every pose, four views
            render(os.environ["KINETIC_POSES"], [(p, ("front", "three", "back")) for p in ("rest", "run", "brace", "climb", "stagger")],
                   size=(380, 560))
            render(os.environ["KINETIC_POSES"] + ".close.png", [(p, ("front", "three", "back")) for p in ("rest", "brace")],
                   size=(460, 560), target_z=0.75, dist=2.2)
            render(os.environ["KINETIC_POSES"] + ".head.png", [(p, ("front", "three", "side")) for p in ("rest", "climb")],
                   size=(420, 420), target_z=1.62, dist=1.0, lens=60)
    return report


def export(path: str, rig, mesh) -> None:
    """The gear on the MetaHuman skeleton, no takes (the body's animation drives it), written in
    the body FBX's own conventions so every bone's bind matrix is the body's, number for number:
    Z up and -Y front (Blender's own axes, so no conversion lands on the root objects),
    centimetres (unit scale 1: the scene unit is set to the centimetre and the data scaled by 100
    for the export, then put back), the Armature node and the root at identity, and the bones'
    local frames as Blender's importer read them (primary Y, secondary X: the inverse of the
    import). harness_mh's checks compare the result with the body FBX's raw bind poses."""
    import harness as H

    scene = bpy.context.scene
    unit = scene.unit_settings.scale_length
    H._scale_scene(rig, mesh, H.CM)
    scene.unit_settings.scale_length = 1.0 / H.CM
    try:
        bpy.ops.object.select_all(action="DESELECT")
        rig.select_set(True)
        mesh.select_set(True)
        bpy.context.view_layer.objects.active = rig
        bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS", global_scale=1.0,
                                 axis_forward="Y", axis_up="Z", bake_space_transform=False,
                                 object_types={"ARMATURE", "MESH"}, mesh_smooth_type="FACE", add_leaf_bones=False,
                                 primary_bone_axis="Y", secondary_bone_axis="X", armature_nodetype="NULL",
                                 bake_anim=False, use_armature_deform_only=False, path_mode="AUTO")
    finally:
        scene.unit_settings.scale_length = unit
        H._scale_scene(rig, mesh, 1.0 / H.CM)


# Exact bind pose ----------------------------------------------------------------------------------

def _model_name(el) -> str:
    return el.props[1].split(b"\x00")[0].decode()


def _rebuilt(pe):
    """A parsed FBX element (io_scene_fbx.parse_fbx) as an element the binary encoder writes."""
    from io_scene_fbx import encode_bin
    e = encode_bin.FBXElem(pe.id)
    adders = {"B": e.add_bool, "C": e.add_char, "Z": e.add_int8, "Y": e.add_int16, "I": e.add_int32, "L": e.add_int64,
              "F": e.add_float32, "D": e.add_float64, "R": e.add_bytes, "S": e.add_string, "i": e.add_int32_array,
              "l": e.add_int64_array, "f": e.add_float32_array, "d": e.add_float64_array, "b": e.add_bool_array,
              "c": e.add_byte_array}
    for data, kind in zip(pe.props, pe.props_type):
        adders[chr(kind)](data)
    for child in pe.elems:
        e.elems.append(_rebuilt(child))
    return e


def _matrix(values) -> np.ndarray:
    return np.array(values, float).reshape(4, 4).T


def _flat(M: np.ndarray):
    import array
    return array.array("d", np.asarray(M, float).T.ravel().tolist())


def match_bind(gear_path: str, body_path: str) -> dict:
    """Rewrites the gear FBX's skeleton with the body FBX's own numbers: every bone's Model
    properties (Lcl Translation, Rotation, Scaling, and whatever else the body sets), its bind
    Pose matrix and its skin cluster's TransformLink (with the cluster's Transform re-derived so
    the mesh's bind stays where it is). Blender keeps bones as head, tail and roll, which loses a
    little near some directions (the thigh and calf twist bones point almost straight down -Y),
    and leader pose needs the bind to be exactly the body's. Returns the largest differences
    found before the rewrite."""
    from io_scene_fbx import encode_bin, parse_fbx

    body, _bv = parse_fbx.parse(body_path)
    gear, gv = parse_fbx.parse(gear_path)

    def objects(root):
        return next(e for e in root.elems if e.id == b"Objects")

    bmodels = {_model_name(o): o for o in objects(body).elems if o.id == b"Model"}
    b_id = {o.props[0]: _model_name(o) for o in bmodels.values()}
    bpose = {}
    for o in objects(body).elems:
        if o.id == b"Pose":
            for pn in o.elems:
                if pn.id == b"PoseNode":
                    nid = next(e.props[0] for e in pn.elems if e.id == b"Node")
                    if nid in b_id:
                        bpose[b_id[nid]] = next(e.props[0] for e in pn.elems if e.id == b"Matrix")
    out = gear
    gobj = next(e for e in out.elems if e.id == b"Objects")
    g_id = {o.props[0]: _model_name(o) for o in gobj.elems if o.id == b"Model"}
    cluster_bone = {}
    conns = next(e for e in out.elems if e.id == b"Connections")
    for c in conns.elems:
        child, parent = c.props[1], c.props[2]
        if child in g_id:
            cluster_bone[parent] = g_id[child]
    worst = {"pose": 0.0, "cluster": 0.0}
    for o in gobj.elems:
        if o.id == b"Model" and _model_name(o) in bmodels and o.props[2] in (b"LimbNode", b"Root"):
            src = next((e for e in bmodels[_model_name(o)].elems if e.id == b"Properties70"), None)
            if src is not None:
                o.elems[:] = [e for e in o.elems if e.id != b"Properties70"] + [src]
        if o.id == b"Pose":
            for pn in o.elems:
                if pn.id != b"PoseNode":
                    continue
                node = next(e for e in pn.elems if e.id == b"Node")
                name = g_id.get(node.props[0])
                if name in bpose:
                    mat = next(e for e in pn.elems if e.id == b"Matrix")
                    worst["pose"] = max(worst["pose"], float(np.abs(_matrix(mat.props[0]) - _matrix(bpose[name])).max()))
                    mat.props[0] = _flat(_matrix(bpose[name]))
        if o.id == b"Deformer" and o.props[2] == b"Cluster" and cluster_bone.get(o.props[0]) in bpose:
            name = cluster_bone[o.props[0]]
            tl = next(e for e in o.elems if e.id == b"TransformLink")
            t = next(e for e in o.elems if e.id == b"Transform")
            old_tl, old_t = _matrix(tl.props[0]), _matrix(t.props[0])
            mesh_bind = old_tl @ old_t
            new_tl = _matrix(bpose[name])
            worst["cluster"] = max(worst["cluster"], float(np.abs(old_tl - new_tl).max()))
            tl.props[0] = _flat(new_tl)
            t.props[0] = _flat(np.linalg.inv(new_tl) @ mesh_bind)
    encode_bin.write(gear_path, _rebuilt(out), gv)
    return worst


# Previews -------------------------------------------------------------------------------------

def prepare_preview(body, face, rig, names, skin: str = "#A8785A") -> None:
    """The body on the skeleton, and the face following its head and neck (its own face rig is
    not here: its weights are the body's below the head and the head bone's above). Both get a
    plain skin colour (the MetaHuman's own materials stay in Unreal)."""
    tone = ka.flat_material("PreviewSkin", skin, 0.5)
    body.hide_render = bool(os.environ.get("KINETIC_NOBODY"))
    face.hide_render = os.environ.get("KINETIC_NOBODY") == "all"
    for o in (body, face):
        mod = o.modifiers.new("Armature", "ARMATURE")
        mod.object = rig
        for i in range(len(o.data.materials)):
            o.data.materials[i] = tone
    co = sdf.verts_of(face)
    bw = Surface(body, weights(body, names))
    W = bw.weights_at(co)
    head_z = rig.data.bones["head"].head_local.z
    top = sdf.smoothstep(head_z - 0.02, head_z + 0.03, co[:, 2])
    H = np.zeros_like(W)
    H[:, names.index("head")] = 1.0
    set_weights(face, W * (1 - top[:, None]) + H * top[:, None], names)
    world = bpy.data.worlds.new("Preview")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.18, 0.18, 0.19, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.35
    bpy.context.scene.world = world
    floor = ka.box("PreviewFloor", (20, 20, 0.02), (0, 0, -0.01), ka.flat_material("Floor", "#3A3836", 0.85))
    del floor
    for name, e, loc, size in (("Key", 520, (2.6, -3.0, 3.2), 2.0), ("Fill", 140, (-3.0, -2.5, 1.6), 3.0),
                               ("Rim", 380, (-0.5, 3.0, 2.8), 1.5)):
        d = bpy.data.lights.new(name, "AREA")
        d.energy, d.size = e, size
        o = bpy.data.objects.new(name, d)
        o.location = loc
        o.rotation_euler = (Vector((0, 0, 1.0)) - Vector(loc)).to_track_quat("-Z", "Y").to_euler()
        bpy.context.scene.collection.objects.link(o)
    cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam"))
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam


VIEWS = {"front": (0, -1, 0.06), "three": (0.7, -0.75, 0.14), "back": (0.1, 1, 0.1), "threel": (-0.7, -0.75, 0.14),
         "side": (1, 0, 0.05)}


def render(path: str, shots, size=(420, 640), samples: int = 32, target_z: float = 0.92, dist: float = 3.4,
           lens: float = 50) -> None:
    """A sheet: one row per pose, one column per view."""
    scene = bpy.context.scene
    scene.cycles.samples = samples
    scene.render.resolution_x, scene.render.resolution_y = size
    rig = bpy.data.objects["Armature"]
    cam = scene.camera
    cam.data.lens = lens
    rows = []
    for pose_name, views in shots:
        angles, drop = POSES[pose_name]
        pose(rig, angles, drop)
        bpy.context.view_layer.update()
        tiles = []
        for v in views:
            d = Vector(VIEWS[v]).normalized()
            t = Vector((0, 0, target_z - drop * 0.5))
            cam.location = t + d * dist
            cam.rotation_euler = (t - cam.location).to_track_quat("-Z", "Y").to_euler()
            scene.render.filepath = path + f".{pose_name}.{v}.png"
            bpy.ops.render.render(write_still=True)
            img = bpy.data.images.load(scene.render.filepath)
            tiles.append(np.array(img.pixels[:]).reshape(img.size[1], img.size[0], 4))
            bpy.data.images.remove(img)
            os.remove(scene.render.filepath)
        rows.append(np.concatenate(tiles, 1))
    pose(rig, {})
    sheet = np.concatenate(rows[::-1], 0)
    img = bpy.data.images.new("Sheet", sheet.shape[1], sheet.shape[0])
    img.pixels[:] = sheet.ravel()
    img.filepath_raw = path
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)
