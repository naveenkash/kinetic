"""Geometry for an arena's visual kit (art/kit/yard_kit.py): shapes added straight into one bmesh per
output mesh, each face carrying a surface (a material slot named after an art/surfaces surface),
then UV'd in metres by box projection, so the tiling surfaces land at their true size whatever the
shape. Everything is in the layout's frame: metres, X east, Y north, Z up (the FBX import turns
that into Unreal's, as for the props).

Nothing real has a knife edge: every box is chamfered (CHAMFER, two segments, less on thin stock)
and every cylinder end eased, so the edges catch the light. Shading is hardened the way a weighted
normal would: the faces are all smooth, the edge where a chamfer meets a flat face is marked hard,
the chamfer itself and a cylinder's sides stay soft, and any other edge is hard past HARD_ANGLE.
The hard edges are split open and the meshes go out with smoothing groups as well, so Unreal's
own normals agree with Blender's however it computes them.

Besides boxes, cylinders and the rolled sections there are the shapes real scrap and real plate
need: `sweep` (a profile along a bending path: rebar, cable, bent beams, crushed pipe), `tube`
(hollow pipe), `sheet` (a plate on a grid with any bend, dish, dent or torn edge, given a
thickness if it shows one), `prism` (a side outline extruded: stringers, gussets) and `grating`
(open bar grating, bar by bar).
"""
from __future__ import annotations

import math
import random

import bmesh
import bpy
from mathutils import Matrix, Vector

SURFACES = ["Concrete", "Slag", "HullSteel", "Rust", "Corrugated", "Grating", "Hazard", "Ochre", "HTROrange", "DarkSteel",
            "Glass", "Rubber", "Timber", "Lamp"]

CHAMFER = 0.015                   # The default edge on a box: 1.5 cm, less on anything thinner than 6 cm
HARD_ANGLE = math.radians(50.0)   # An unmarked edge sharper than this shades hard
AUTO, HARD, SOFT = 0, 1, 2        # The edge marks
CHUNK = 6000                      # Vertices built in one bmesh before it is finished and set aside


def auto_bevel(size) -> float:
    """The chamfer a box of this size gets by default: CHAMFER, or a quarter of its thinnest side
    for thin stock, and none under 6 mm (bars, plate edges)."""
    thin = min(size)
    return 0.0 if thin < 0.006 else min(CHAMFER, thin * 0.25)


def frames(path, up=(0, 0, 1)):
    """Parallel-transport frames (tangent, normal, binormal) along a polyline, the first normal
    as near `up` as the path allows."""
    n = len(path)
    ts = [(path[min(i + 1, n - 1)] - path[max(i - 1, 0)]).normalized() for i in range(n)]
    upv = Vector(up)
    if abs(ts[0].dot(upv)) > 0.95:
        upv = Vector((1, 0, 0)) if abs(ts[0].x) < 0.9 else Vector((0, 1, 0))
    nrm = (upv - ts[0] * upv.dot(ts[0])).normalized()
    out = []
    for i, t in enumerate(ts):
        if i:
            nrm = ts[i - 1].rotation_difference(t) @ nrm
            nrm = (nrm - t * nrm.dot(t)).normalized()
        out.append((t, nrm, t.cross(nrm)))
    return out


class Group:
    """One output mesh (SM_Yard_<name>). It is built in chunks: a bmesh operator walks the whole
    bmesh it works in, so shapes go into a small one that is finished (hardened, UV'd) and set
    aside every CHUNK vertices, and the chunks are joined at the end."""

    def __init__(self, name: str):
        self.name = name
        self.slots: list[str] = []
        self.done: list = []
        self._pieces: list[list] = []
        self._new_bmesh()

    def _new_bmesh(self):
        self.bm = bmesh.new()
        self.marks = self.bm.edges.layers.int.new("hard")

    def _flush(self):
        if not len(self.bm.verts):
            return
        self.bm.normal_update()
        self.harden()
        self.uv_metres()
        mesh = bpy.data.meshes.new(f"{self.name}_part")
        self.bm.to_mesh(mesh)
        self.bm.free()
        self.done.append(mesh)
        self._new_bmesh()

    def _maybe_flush(self):
        if not self._pieces and len(self.bm.verts) > CHUNK:
            self._flush()

    def hold(self):
        """Start a piece built from several shapes: they stay in one chunk, and `release` hands
        back every vertex they made (to move the piece as one)."""
        self._maybe_flush()
        self._pieces.append([])

    def release(self) -> list:
        verts = [v for v in self._pieces.pop() if v.is_valid]
        self._made(verts)
        return verts

    def _made(self, verts):
        """A shape's finished vertices, for the piece being held (if any). Tracked by hand: bmesh
        reuses the slots of deleted vertices, so "everything after index n" is not "new"."""
        if self._pieces:
            self._pieces[-1].extend(verts)

    def slot(self, surface: str) -> int:
        if surface not in self.slots:
            self.slots.append(surface)
        return self.slots.index(surface)

    def _take(self, geom_faces, surface: str):
        index = self.slot(surface)
        for f in geom_faces:
            f.material_index = index

    def _bevel(self, edges, offset: float, segments: int = 2):
        """Chamfer these edges; the chamfer's own edges soft, its border with the flat faces hard."""
        b = bmesh.ops.bevel(self.bm, geom=edges, offset=offset, segments=segments, affect="EDGES", profile=0.5,
                            clamp_overlap=True)
        bevel_faces = set(b["faces"])
        for f in bevel_faces:
            for e in f.edges:
                e[self.marks] = SOFT if all(x in bevel_faces for x in e.link_faces) else HARD
        return b

    @staticmethod
    def faces_of(verts) -> list:
        return list({f for v in verts for f in v.link_faces})

    def _finish(self, verts, surface: str):
        """Give a shape's faces their surface and hand its vertices to the piece being held."""
        self._take(self.faces_of(verts), surface)
        self._made(verts)

    # Shapes --------------------------------------------------------------------------------------

    def box(self, mn, mx, surface: str, bevel: float | None = None, matrix: Matrix | None = None, segments: int = 2):
        """An axis-aligned box from min to max (then transformed by matrix), chamfered: `bevel` None
        takes auto_bevel's, 0 leaves it sharp."""
        self._maybe_flush()
        mn, mx = Vector(mn), Vector(mx)
        size = mx - mn
        if min(size) <= 1e-5:
            return
        if bevel is None:
            bevel = auto_bevel(size)
        res = bmesh.ops.create_cube(self.bm, size=1.0)
        verts = res["verts"]
        m = Matrix.Translation((mn + mx) / 2) @ Matrix.Diagonal((size.x, size.y, size.z, 1.0))
        if matrix is not None:
            m = matrix @ m
        bmesh.ops.transform(self.bm, matrix=m, verts=verts)
        if bevel > 0.0:
            edges = list({e for v in verts for e in v.link_edges})
            b = self._bevel(edges, min(bevel, min(size) * 0.45), segments if bevel < 0.05 else max(segments, 4))
            verts = [v for v in verts if v.is_valid] + b["verts"]
        self._finish(verts, surface)

    def cylinder(self, a, b, radius: float, surface: str, segments: int = 12, cap: bool = True, bevel: float | None = None):
        """A round bar or pipe from a to b, its sides soft (hard-cornered at six sides or fewer: a hex
        nut); capped ends are eased by `bevel` (None: a fifth of the radius up to 8 mm, nothing
        under 2 cm radius)."""
        self._maybe_flush()
        a, b = Vector(a), Vector(b)
        axis = b - a
        length = axis.length
        if length < 1e-5:
            return
        res = bmesh.ops.create_cone(self.bm, cap_ends=cap, cap_tris=False, segments=segments, radius1=radius,
                                    radius2=radius, depth=length)
        m = Matrix.Translation((a + b) / 2) @ axis.to_track_quat("Z", "Y").to_matrix().to_4x4()
        bmesh.ops.transform(self.bm, matrix=m, verts=res["verts"])
        faces = self.faces_of(res["verts"])
        ax = axis / length
        caps = []
        for f in faces:
            f.normal_update()
            if abs(f.normal.dot(ax)) > 0.9:
                caps.append(f)
        ring = set()
        for f in faces:
            for e in f.edges:
                if f in caps:
                    ring.add(e)
                    e[self.marks] = HARD
                elif e[self.marks] != HARD:
                    e[self.marks] = SOFT if segments > 6 else AUTO
        verts = res["verts"]
        if cap:
            if bevel is None:
                bevel = 0.0 if radius < 0.02 else min(0.008, radius * 0.2)
            if bevel > 0.0:
                b = self._bevel(list(ring), min(bevel, length * 0.3))
                verts = [v for v in verts if v.is_valid] + b["verts"]
        self._finish(verts, surface)

    def sweep(self, path, profile, surface: str, up=(0, 0, 1), soft: bool = False, cap: bool = True, scale=None,
              cut0=(0.0, 0.0), cut1=(0.0, 0.0)):
        """A closed profile [(side, up), ...] carried along a polyline: bent beams and bars, cable,
        crushed pipe. `soft` shades round the profile (round stock); `scale(i)` -> (sx, su) squeezes
        ring i; cut0/cut1 slope the end cuts (metres along the path per metre of side and up)."""
        self._maybe_flush()
        path = [Vector(p) for p in path]
        if len(path) < 2:
            return
        fr = frames(path, up)
        rings = []
        last = len(path) - 1
        for i, (p, (t, n, bn)) in enumerate(zip(path, fr)):
            sx, su = scale(i) if scale else (1.0, 1.0)
            cut = cut0 if i == 0 else cut1 if i == last else None
            ring = []
            for (px, py) in profile:
                q = p + bn * (px * sx) + n * (py * su)
                if cut is not None:
                    q += t * (px * cut[0] + py * cut[1])
                ring.append(self.bm.verts.new(q))
            rings.append(ring)
        k = len(profile)
        sides = []
        for r0, r1 in zip(rings, rings[1:]):
            for j in range(k):
                sides.append(self.bm.faces.new((r0[j], r0[(j + 1) % k], r1[(j + 1) % k], r1[j])))
        caps = []
        if cap:
            for ring in (rings[0], rings[-1]):
                caps.append(self.bm.faces.new(ring))
        faces = sides + caps
        bmesh.ops.recalc_face_normals(self.bm, faces=faces)
        # The end rings are hard; round stock is soft all over; a flat-sided profile is left to the
        # angle test (its corners hard, its bends soft).
        first, final = set(rings[0]), set(rings[-1])
        for f in sides:
            for e in f.edges:
                v0, v1 = e.verts
                if (v0 in first and v1 in first) or (v0 in final and v1 in final):
                    e[self.marks] = HARD if cap else AUTO
                elif soft:
                    e[self.marks] = SOFT
        if cap:
            concave = [c for c in caps if len(c.verts) > 4]
            if concave:
                bmesh.ops.triangulate(self.bm, faces=concave)
        self._finish([v for ring in rings for v in ring], surface)

    def tube(self, path, radius: float, wall: float, surface: str, segments: int = 16, up=(0, 0, 1), scale=None,
             cut0=(0.0, 0.0), cut1=(0.0, 0.0), phase: float = 0.0):
        """A hollow pipe along a polyline: outer and bore walls and the cut annulus at each end (four
        segments at a phase of 45 degrees make a square hollow section)."""
        self._maybe_flush()
        path = [Vector(p) for p in path]
        fr = frames(path, up)
        last = len(path) - 1
        outer, inner = [], []
        for i, (p, (t, n, bn)) in enumerate(zip(path, fr)):
            sx, su = scale(i) if scale else (1.0, 1.0)
            cut = cut0 if i == 0 else cut1 if i == last else None
            ro, ri = [], []
            for j in range(segments):
                a = 2 * math.pi * j / segments + phase
                for r, ring in ((radius, ro), (radius - wall, ri)):
                    px, py = math.cos(a) * r * sx, math.sin(a) * r * su
                    q = p + bn * px + n * py
                    if cut is not None:
                        q += t * (px * cut[0] + py * cut[1])
                    ring.append(self.bm.verts.new(q))
            outer.append(ro)
            inner.append(ri)
        k = segments
        walls = []
        for rings, flip in ((outer, False), (inner, True)):
            for r0, r1 in zip(rings, rings[1:]):
                for j in range(k):
                    quad = (r0[j], r0[(j + 1) % k], r1[(j + 1) % k], r1[j])
                    walls.append(self.bm.faces.new(quad[::-1] if flip else quad))
        ends = []
        for i in (0, last):
            for j in range(k):
                quad = (outer[i][j], inner[i][j], inner[i][(j + 1) % k], outer[i][(j + 1) % k])
                ends.append(self.bm.faces.new(quad if i == 0 else quad[::-1]))
        bmesh.ops.recalc_face_normals(self.bm, faces=walls + ends)
        for f in walls:
            for e in f.edges:
                e[self.marks] = SOFT if segments > 6 else AUTO
        for f in ends:
            for e in f.edges:
                e[self.marks] = HARD
        self._finish([v for rings in (outer, inner) for ring in rings for v in ring], surface)

    def sheet(self, matrix: Matrix, us, vs, surface: str, shape=None, keep=None, thickness: float = 0.0, cell_surface=None):
        """A plate over the grid us x vs (metres in the matrix's XY plane, its Z out of the face).
        shape(u, v) -> (du, dv, dz) moves each vertex (a bend, dishing, a dent, a torn edge);
        keep(i, j) False drops that cell; thickness > 0 gives the plate a back and cut edges;
        cell_surface(i, j) may give a cell a surface of its own (rust at a sheet's foot)."""
        self._maybe_flush()
        grid = {}
        for i, u in enumerate(us):
            for j, v in enumerate(vs):
                d = shape(u, v) if shape else (0.0, 0.0, 0.0)
                grid[i, j] = Vector((u + d[0], v + d[1], d[2]))
        used = {}

        def vert(i, j):
            if (i, j) not in used:
                used[i, j] = self.bm.verts.new(matrix @ grid[i, j])
            return used[i, j]

        faces, cells = [], {}
        for i in range(len(us) - 1):
            for j in range(len(vs) - 1):
                if keep is not None and not keep(i, j):
                    continue
                f = self.bm.faces.new((vert(i, j), vert(i + 1, j), vert(i + 1, j + 1), vert(i, j + 1)))
                faces.append(f)
                cells[f] = (i, j)
        if not faces:
            return
        verts = list(used.values())
        if thickness > 0.0:
            res = bmesh.ops.solidify(self.bm, geom=faces, thickness=thickness)
            verts += [v for v in res["geom"] if isinstance(v, bmesh.types.BMVert)]
        self._finish(verts, surface)
        if cell_surface is not None:
            for f, (i, j) in cells.items():
                s = cell_surface(i, j)
                if s:
                    f.material_index = self.slot(s)

    def prism(self, outline, matrix: Matrix, depth: float, surface: str):
        """A flat outline [(x, z), ...] in the matrix's XZ plane extruded `depth` along its Y: a web,
        a flange cut to a stair's slope, a gusset."""
        self.hold()
        self.sweep([Vector((0, 0, 0)), Vector((0, depth, 0))], outline, surface, up=(0, 0, 1))
        bmesh.ops.transform(self.bm, matrix=matrix, verts=self.release())

    # Rolled sections -------------------------------------------------------------------------------

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
        self.box((0, -0.035, 0.12), (L, 0.035, 0.16), surface, bevel=0.008, matrix=m)  # Head

    def railing(self, a, b, height: float, surface: str = "Ochre", posts_every: float = 1.5, mid: bool = True, toe: bool = False):
        """A handrail: posts, a top rail and a mid rail, from a to b at floor level (the top rail
        follows a slope if a and b differ in height); `toe` adds a 10 cm kick plate."""
        a, b = Vector(a), Vector(b)
        L = (b - a).length
        n = max(1, round(L / posts_every))
        for i in range(n + 1):
            p = a.lerp(b, i / n)
            self.cylinder(p, p + Vector((0, 0, height + 0.02)), 0.024, surface, segments=12)
            self.cylinder(p, p + Vector((0, 0, 0.008)), 0.06, "DarkSteel", segments=12)  # The base plate
        up = Vector((0, 0, height))
        self.cylinder(a + up, b + up, 0.0265, surface, segments=14)
        if mid:
            self.cylinder(a + up * 0.5, b + up * 0.5, 0.021, surface, segments=12)
        if toe:
            d = (b - a).normalized()
            side = Vector((-d.y, d.x, 0)).normalized() * 0.003
            m = Matrix((d, side.normalized(), Vector((0, 0, 1)))).transposed().to_4x4()
            m.translation = a
            self.box((0, -0.003, 0.005), (L, 0.003, 0.105), surface, matrix=m)

    def ring_cage(self, centre, radius: float, z0: float, z1: float, side, surface: str = "Ochre"):
        """A ladder cage: flat-bar hoops every 0.75 m and five vertical straps, open on the wall side."""
        c = Vector(centre)
        sx, sy = side
        start = math.atan2(-sy, -sx) + math.radians(40)  # Leave the wall side open
        z = z0 + 2.2
        while z < z1:
            pts = [c + Vector((math.cos(start + t * math.radians(280) / 20) * radius,
                               math.sin(start + t * math.radians(280) / 20) * radius, z)) for t in range(21)]
            self.sweep(pts, [(-0.003, -0.025), (0.003, -0.025), (0.003, 0.025), (-0.003, 0.025)], surface, up=(0, 0, 1))
            z += 0.75
        for k in range(5):
            t = start + math.radians(280) * k / 4
            p = c + Vector((math.cos(t) * radius, math.sin(t) * radius, 0))
            self.cylinder(p + Vector((0, 0, z0 + 2.2)), p + Vector((0, 0, z1)), 0.012, surface, segments=8)

    def grating(self, mn, mx, along: int = 0, surface: str = "Grating", pitch: float = 0.03, bar=(0.003, 0.03),
                rod_pitch: float = 0.1, panel: float = 1.0, band: bool = True):
        """Open bar grating filling mn..mx, its top at mx's z: bearing bars along axis `along` (0 X,
        1 Y) every `pitch`, square cross rods pressed in flush every `rod_pitch`, and a banding bar
        round each `panel` length with a 5 mm gap between panels."""
        mn, mx = Vector(mn), Vector(mx)
        a, c = along, 1 - along
        top = mx.z
        t, d = bar
        length = mx[a] - mn[a]
        n_panels = max(1, round(length / panel))
        pl = length / n_panels

        def put(lo_a, hi_a, lo_c, hi_c, z0, z1, bevel=0.0):
            lo, hi = [0.0, 0.0, z0], [0.0, 0.0, z1]
            lo[a], hi[a], lo[c], hi[c] = lo_a, hi_a, lo_c, hi_c
            self.box(lo, hi, surface, bevel=bevel)

        for p in range(n_panels):
            a0, a1 = mn[a] + p * pl + 0.0025, mn[a] + (p + 1) * pl - 0.0025
            if band:
                for lo_c, hi_c in ((mn[c], mn[c] + t), (mx[c] - t, mx[c])):
                    put(a0, a1, lo_c, hi_c, top - d, top)
                for lo_a, hi_a in ((a0, a0 + t), (a1 - t, a1)):
                    put(lo_a, hi_a, mn[c], mx[c], top - d, top)
            span = mx[c] - mn[c]
            nb = max(1, int(span / pitch))
            for i in range(1, nb):
                cc = mn[c] + span * i / nb
                put(a0 + t, a1 - t, cc - t / 2, cc + t / 2, top - d, top - 0.001)
            nr = max(1, int((a1 - a0) / rod_pitch))
            for i in range(1, nr):
                aa = a0 + (a1 - a0) * i / nr
                put(aa - 0.0025, aa + 0.0025, mn[c] + t, mx[c] - t, top - 0.006, top)

    # Finishing -----------------------------------------------------------------------------------

    def harden(self):
        """Shade every face smooth and set each edge hard or soft: as marked, else hard where the
        surface changes or the faces meet sharper than HARD_ANGLE. The hard edges are then split
        open, so the shading survives whatever normals the importer computes (smoothing groups,
        per-face smoothing, or none)."""
        marks = self.marks
        for f in self.bm.faces:
            f.smooth = True
        split = []
        for e in self.bm.edges:
            m = e[marks]
            if m == AUTO:
                lf = e.link_faces
                hard = len(lf) != 2 or lf[0].material_index != lf[1].material_index or e.calc_face_angle(math.pi) > HARD_ANGLE
            else:
                hard = m == HARD
            e.smooth = not hard
            if hard and len(e.link_faces) == 2:
                split.append(e)
        if split:
            bmesh.ops.split_edges(self.bm, edges=split)

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
        self._flush()
        self.bm.free()
        whole = bmesh.new()
        for part in self.done:
            whole.from_mesh(part)
            bpy.data.meshes.remove(part)
        self.done = []
        mesh = bpy.data.meshes.new(self.name)
        whole.to_mesh(mesh)
        whole.free()
        for s in self.slots:
            mesh.materials.append(materials[s])
        obj = bpy.data.objects.new(self.name, mesh)
        bpy.context.scene.collection.objects.link(obj)
        return obj


def seeded(label: str, index: int) -> random.Random:
    """A repeatable random for one piece."""
    import zlib
    return random.Random(zlib.crc32(repr((label, index)).encode()))
