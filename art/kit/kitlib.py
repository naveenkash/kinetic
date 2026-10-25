"""Geometry for an arena's visual kit (art/kit/yard_kit.py): shapes added straight into one bmesh per
output mesh, each face carrying a surface (a material slot named after an art/surfaces surface),
then UV'd in metres by box projection, so the tiling surfaces land at their true size whatever the
shape. Everything is in the layout's frame: metres, X east, Y north, Z up (the FBX import turns
that into Unreal's, as for the props).
"""
from __future__ import annotations

import math
import random

import bmesh
import bpy
from mathutils import Matrix, Vector

SURFACES = ["Concrete", "Slag", "HullSteel", "Rust", "Corrugated", "Grating", "Hazard", "Ochre", "HTROrange", "DarkSteel",
            "Glass", "Rubber", "Timber", "Lamp"]


class Group:
    """One output mesh (SM_Yard_<name>)."""

    def __init__(self, name: str):
        self.name = name
        self.bm = bmesh.new()
        self.slots: list[str] = []

    def slot(self, surface: str) -> int:
        if surface not in self.slots:
            self.slots.append(surface)
        return self.slots.index(surface)

    def _take(self, geom_faces, surface: str):
        index = self.slot(surface)
        for f in geom_faces:
            f.material_index = index

    # Shapes --------------------------------------------------------------------------------------

    def box(self, mn, mx, surface: str, bevel: float = 0.0, matrix: Matrix | None = None):
        """An axis-aligned box from min to max (then transformed by matrix), optionally bevelled."""
        mn, mx = Vector(mn), Vector(mx)
        size = mx - mn
        if min(size) <= 1e-5:
            return
        res = bmesh.ops.create_cube(self.bm, size=1.0)
        verts = res["verts"]
        m = Matrix.Translation((mn + mx) / 2) @ Matrix.Diagonal((size.x, size.y, size.z, 1.0))
        if matrix is not None:
            m = matrix @ m
        bmesh.ops.transform(self.bm, matrix=m, verts=verts)
        if bevel > 0.0:
            edges = list({e for v in verts for e in v.link_edges})
            b = bmesh.ops.bevel(self.bm, geom=edges, offset=min(bevel, min(size) * 0.45), segments=1, affect="EDGES",
                                profile=0.5)
            # The bevel replaces the box's faces: take what is there now.
            new_verts = [v for v in b.get("verts", []) if v.is_valid] + [v for v in verts if v.is_valid]
            faces = list({f for v in new_verts for f in v.link_faces} | {f for f in b.get("faces", []) if f.is_valid})
        else:
            faces = list({f for v in verts for f in v.link_faces})
        self._take(faces, surface)

    def cylinder(self, a, b, radius: float, surface: str, segments: int = 12, cap: bool = True):
        a, b = Vector(a), Vector(b)
        axis = b - a
        length = axis.length
        if length < 1e-5:
            return
        res = bmesh.ops.create_cone(self.bm, cap_ends=cap, cap_tris=False, segments=segments, radius1=radius,
                                    radius2=radius, depth=length)
        m = Matrix.Translation((a + b) / 2) @ axis.to_track_quat("Z", "Y").to_matrix().to_4x4()
        bmesh.ops.transform(self.bm, matrix=m, verts=res["verts"])
        self._take(list({f for v in res["verts"] for f in v.link_faces}), surface)

    def beam(self, a, b, depth: float, width: float, surface: str, flange: float = 0.02, web: float = 0.012, up=(0, 0, 1)):
        """An I-beam from a to b: two flanges and a web, `depth` tall along up."""
        a, b = Vector(a), Vector(b)
        x = (b - a).normalized()
        upv = Vector(up)
        if abs(x.dot(upv)) > 0.95:
            upv = Vector((1, 0, 0)) if abs(x.x) < 0.9 else Vector((0, 1, 0))
        y = upv.cross(x).normalized()
        z = x.cross(y)
        m = Matrix((x, y, z)).transposed().to_4x4()
        m.translation = a
        L = (b - a).length
        for zc in (depth / 2 - flange / 2, -depth / 2 + flange / 2):
            self.box((0, -width / 2, zc - flange / 2), (L, width / 2, zc + flange / 2), surface, matrix=m)
        self.box((0, -web / 2, -depth / 2 + flange), (L, web / 2, depth / 2 - flange), surface, matrix=m)

    def rail(self, a, b, surface: str = "DarkSteel"):
        """A rail: head, web and foot (a 54 kg/m flat-bottom section), along a to b at its foot."""
        a, b = Vector(a), Vector(b)
        x = (b - a).normalized()
        y = Vector((0, 0, 1)).cross(x).normalized()
        m = Matrix((x, y, Vector((0, 0, 1)))).transposed().to_4x4()
        m.translation = a
        L = (b - a).length
        self.box((0, -0.075, 0.0), (L, 0.075, 0.012), surface, matrix=m)       # Foot
        self.box((0, -0.008, 0.012), (L, 0.008, 0.12), surface, matrix=m)      # Web
        self.box((0, -0.035, 0.12), (L, 0.035, 0.16), surface, bevel=0.006, matrix=m)  # Head

    def railing(self, a, b, height: float, surface: str = "Ochre", posts_every: float = 1.5, mid: bool = True):
        """A handrail: posts, a top rail and a mid rail, from a to b at floor level."""
        a, b = Vector(a), Vector(b)
        L = (b - a).length
        n = max(1, round(L / posts_every))
        for i in range(n + 1):
            p = a.lerp(b, i / n)
            self.cylinder(p, p + Vector((0, 0, height)), 0.022, surface, segments=8)
        up = Vector((0, 0, height))
        self.cylinder(a + up, b + up, 0.024, surface, segments=8)
        if mid:
            self.cylinder(a + up * 0.5, b + up * 0.5, 0.018, surface, segments=8)

    def ring_cage(self, centre, radius: float, z0: float, z1: float, side, surface: str = "Ochre"):
        """A ladder cage: hoops every 0.75 m and five vertical straps, open on the wall side."""
        c = Vector(centre)
        sx, sy = side
        start = math.atan2(-sy, -sx) + math.radians(40)  # Leave the wall side open
        z = z0 + 2.2
        while z < z1:
            pts = [c + Vector((math.cos(start + t * math.radians(280) / 10) * radius,
                               math.sin(start + t * math.radians(280) / 10) * radius, 0)) for t in range(11)]
            for p, q in zip(pts, pts[1:]):
                self.cylinder(p + Vector((0, 0, z)), q + Vector((0, 0, z)), 0.012, surface, segments=6)
            z += 0.75
        for k in range(5):
            t = start + math.radians(280) * k / 4
            p = c + Vector((math.cos(t) * radius, math.sin(t) * radius, 0))
            self.cylinder(p + Vector((0, 0, z0 + 2.2)), p + Vector((0, 0, z1)), 0.012, surface, segments=6)

    # Finishing -----------------------------------------------------------------------------------

    def uv_metres(self):
        """Box projection in metres: each face takes the two axes across its normal."""
        uv = self.bm.loops.layers.uv.verify()
        for f in self.bm.faces:
            n = f.normal
            ax = max(range(3), key=lambda i: abs(n[i]))
            u_axis, v_axis = {0: (1, 2), 1: (0, 2), 2: (0, 1)}[ax]
            flip = -1.0 if n[ax] < 0 else 1.0
            for loop in f.loops:
                co = loop.vert.co
                loop[uv].uv = (co[u_axis] * (flip if ax != 2 else 1.0), co[v_axis])

    def object(self, materials: dict) -> bpy.types.Object:
        self.bm.normal_update()
        self.uv_metres()
        mesh = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(mesh)
        self.bm.free()
        for s in self.slots:
            mesh.materials.append(materials[s])
        obj = bpy.data.objects.new(self.name, mesh)
        bpy.context.scene.collection.objects.link(obj)
        return obj


def seeded(label: str, index: int) -> random.Random:
    """A repeatable random for one piece."""
    import zlib
    return random.Random(zlib.crc32(repr((label, index)).encode()))
