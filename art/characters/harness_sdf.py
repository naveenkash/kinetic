"""Signed distance fields and their meshing, for the soft parts of a harness character
(harness_body.py models the suit, head, hands and boots with them).

A field is a function from points (an (N, 3) numpy array, metres) to signed distances (negative
inside). The primitives are approximate distances (ellipsoids, round cones, rounded boxes, and
lofts: tubes of superellipse sections with their own extents front and back, out and in), and
`smin`/`smax` blend them with a rounded fillet of size k, which is what makes a figure read as
one surface instead of parts. `mesh` samples a field in narrow-band blocks into an OpenVDB grid
(bundled with Blender) and meshes it to quads; `refine` pulls the vertices onto the exact
surface; `project_sphere` wraps a sphere onto a star-shaped field (a head).

Everything is numpy and deterministic: the same field gives the same mesh every run.
"""
from __future__ import annotations

import math

import bpy
import numpy as np


# Fields ----------------------------------------------------------------------------------------

def dot(a, b):
    return np.einsum("ij,ij->i", a, b)


def length(a):
    return np.sqrt(np.einsum("ij,ij->i", a, a))


def smin(a, b, k: float):
    """Polynomial smooth minimum: the union of two fields with a fillet of about k."""
    if k <= 0:
        return np.minimum(a, b)
    h = np.clip(0.5 + 0.5 * (b - a) / k, 0.0, 1.0)
    return b + (a - b) * h - k * h * (1.0 - h)


def smax(a, b, k: float):
    """Smooth maximum: the intersection (or, with -b, the subtraction) with a fillet of about k."""
    return -smin(-a, -b, k)


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


def frame(w, forward=(1.0, 0.0, 0.0), out_sign: float = 1.0) -> np.ndarray:
    """Columns (u, v, w): w along `w`, u as near `forward` as it can be, v = out_sign * (u x w)
    (for a left limb with out_sign 1, v points away from the body)."""
    w = np.asarray(w, float)
    w = w / np.linalg.norm(w)
    f = np.asarray(forward, float)
    u = f - f.dot(w) * w
    if np.linalg.norm(u) < 1e-6:
        u = np.array([0.0, 0.0, 1.0]) - w[2] * w
    u /= np.linalg.norm(u)
    v = out_sign * np.cross(u, w)
    return np.stack([u, v, w], 1)


def rotation(axis: str, degrees: float) -> np.ndarray:
    a = math.radians(degrees)
    c, s = math.cos(a), math.sin(a)
    return {"X": np.array([[1, 0, 0], [0, c, -s], [0, s, c]]),
            "Y": np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]]),
            "Z": np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])}[axis]


def ellipsoid(p, c, r, rot=None):
    """An ellipsoid of radii r about c; rot's columns are its local axes."""
    q = p - np.asarray(c, float)
    if rot is not None:
        q = q @ rot
    r = np.asarray(r, float)
    k0 = length(q / r)
    k1 = np.maximum(length(q / (r * r)), 1e-9)
    return k0 * (k0 - 1.0) / k1


def round_cone(p, a, b, r1: float, r2: float):
    """A capsule from a (radius r1) to b (radius r2): a limb segment, a finger bone."""
    a, b = np.asarray(a, float), np.asarray(b, float)
    ba = b - a
    l2 = ba.dot(ba)
    rr = r1 - r2
    a2 = l2 - rr * rr
    il2 = 1.0 / l2
    pa = p - a
    y = pa @ ba
    z = y - l2
    xv = pa * l2 - np.outer(y, ba)
    x2 = dot(xv, xv)
    y2 = y * y * l2
    z2 = z * z * l2
    k = math.copysign(1.0, rr) * rr * rr * x2
    d_mid = (np.sqrt(np.maximum(x2 * a2 * il2, 0.0)) + y * rr) * il2 - r1
    d_b = np.sqrt(x2 + z2) * il2 - r2
    d_a = np.sqrt(x2 + y2) * il2 - r1
    return np.where(np.sign(z) * a2 * z2 > k, d_b, np.where(np.sign(y) * a2 * y2 < k, d_a, d_mid))


def rbox(p, c, half, rot=None, radius: float = 0.0):
    """A box of half-extents `half` about c with its edges rounded by `radius`."""
    q = p - np.asarray(c, float)
    if rot is not None:
        q = q @ rot
    q = np.abs(q) - (np.asarray(half, float) - radius)
    return length(np.maximum(q, 0.0)) + np.minimum(q.max(1), 0.0) - radius


class Loft:
    """A tube along an axis: rows (s, cu, cv, u+, u-, v+, v-, n) give, at s along the axis, the
    section's centre offset (cu, cv), its extents forward and back (u+, u-), out and in (v+, v-)
    and its superellipse exponent n (2 round, higher boxier). Sections in between are Hermite
    interpolated (Catmull-Rom tangents), so the profile has no kinks; the ends are rounded."""

    def __init__(self, origin, axes: np.ndarray, rows, round_ends: float = 0.02):
        self.o = np.asarray(origin, float)
        self.R = np.asarray(axes, float)
        table = np.array(rows, float)
        self.s = table[:, 0]
        self.y = table[:, 1:]
        self.k = round_ends

    def params(self, s):
        S, Y = self.s, self.y
        n = len(S)
        i = np.clip(np.searchsorted(S, s) - 1, 0, n - 2)
        s0, s1 = S[i], S[i + 1]
        h = s1 - s0
        t = np.clip((s - s0) / h, 0.0, 1.0)

        def tangent(j):
            jm, jp = np.maximum(j - 1, 0), np.minimum(j + 1, n - 1)
            return (Y[jp] - Y[jm]) / (S[jp] - S[jm])[:, None]

        t2, t3 = t * t, t * t * t
        h00, h10, h01, h11 = 2 * t3 - 3 * t2 + 1, t3 - 2 * t2 + t, -2 * t3 + 3 * t2, t3 - t2
        return (h00[:, None] * Y[i] + (h10 * h)[:, None] * tangent(i) + h01[:, None] * Y[i + 1]
                + (h11 * h)[:, None] * tangent(i + 1))

    def local(self, p):
        return (p - self.o) @ self.R

    def __call__(self, p):
        q = self.local(p)
        s = q[:, 2]
        P = self.params(np.clip(s, self.s[0], self.s[-1]))
        du, dv = q[:, 0] - P[:, 0], q[:, 1] - P[:, 1]
        a = np.maximum(np.where(du > 0, P[:, 2], P[:, 3]), 1e-4)
        b = np.maximum(np.where(dv > 0, P[:, 4], P[:, 5]), 1e-4)
        n = P[:, 6]
        rho = ((np.abs(du) / a) ** n + (np.abs(dv) / b) ** n) ** (1.0 / n)
        r = np.sqrt(du * du + dv * dv)
        # The distance along the ray from the section's centre: exact on that ray, and smooth.
        boundary = np.where(rho > 1e-7, r / np.maximum(rho, 1e-7), np.minimum(a, b))
        d = r - boundary
        ends = np.maximum(self.s[0] - s, s - self.s[-1])
        return smax(d, ends, self.k)


# Noise -----------------------------------------------------------------------------------------

def _hash(ix, iy, iz, seed: int):
    h = (ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791) ^ (seed * 2654435761)
    h = (h ^ (h >> 13)) * 1274126177
    return ((h ^ (h >> 16)) & 0xFFFF).astype(np.float64) / 65535.0


def value_noise(p, scale: float, seed: int = 0):
    """Smooth value noise in [0, 1] at `scale` cells per metre (deterministic)."""
    q = p * scale
    i = np.floor(q).astype(np.int64)
    f = q - i
    f = f * f * (3.0 - 2.0 * f)
    out = 0.0
    for dx in (0, 1):
        for dy in (0, 1):
            for dz in (0, 1):
                w = (f[:, 0] if dx else 1 - f[:, 0]) * (f[:, 1] if dy else 1 - f[:, 1]) * (f[:, 2] if dz else 1 - f[:, 2])
                out = out + w * _hash(i[:, 0] + dx, i[:, 1] + dy, i[:, 2] + dz, seed)
    return out


def fbm(p, scale: float, octaves: int = 3, seed: int = 0):
    total, amp, norm = 0.0, 1.0, 0.0
    for o in range(octaves):
        total = total + amp * value_noise(p, scale * 2 ** o, seed + o)
        norm += amp
        amp *= 0.5
    return total / norm


# Meshing ---------------------------------------------------------------------------------------

def mesh(field, lo, hi, h: float, block: int = 16, batch: int = 96):
    """Meshes the field's zero surface inside the box lo..hi at voxel size h: returns (verts,
    quads). Only the blocks the surface can pass through are sampled; blocks wholly inside are
    filled as interior tiles."""
    import openvdb as vdb

    lo = np.asarray(lo, float)
    dims = np.ceil((np.asarray(hi, float) - lo) / h).astype(int) + 1
    nb = -(-dims // block)
    idx = np.stack(np.meshgrid(*[np.arange(k) for k in nb], indexing="ij"), -1).reshape(-1, 3)
    centres = lo + (idx * block + (block - 1) / 2.0) * h
    dc = field(centres)
    reach = block * h * 0.87 * 1.7 + 2 * h
    background = block * h * 2.0
    grid = vdb.FloatGrid(background=background)
    for b in idx[dc <= -reach]:
        start = tuple(int(x) for x in b * block)
        grid.fill(start, tuple(int(x) + block - 1 for x in start), -background, False)
    near = idx[np.abs(dc) < reach]
    local = np.stack(np.meshgrid(*[np.arange(block)] * 3, indexing="ij"), -1).reshape(-1, 3)
    for k in range(0, len(near), batch):
        chunk = near[k:k + batch]
        pts = lo + ((chunk[:, None, :] * block + local[None]) * h).reshape(-1, 3)
        vals = field(pts).astype(np.float32).reshape(len(chunk), block, block, block)
        for b, v in zip(chunk, vals):
            grid.copyFromArray(np.ascontiguousarray(v), ijk=tuple(int(x) for x in b * block), tolerance=0)
    points, quads = grid.convertToQuads(isovalue=0.0)
    return lo + np.asarray(points, float) * h, np.asarray(quads, np.int64)


def gradient(field, p, eps: float = 2e-4):
    d = field(p)
    g = np.stack([(field(p + e) - d) / eps for e in np.eye(3) * eps], 1)
    return d, g


def refine(field, verts, iterations: int = 2):
    """Newton steps onto the zero surface (the meshed vertices sit within a voxel of it)."""
    for _ in range(iterations):
        d, g = gradient(field, verts)
        verts = verts - (d / np.maximum(dot(g, g), 1e-12))[:, None] * g
    return verts


def relax(verts, faces, field, iterations: int = 6, amount: float = 0.5):
    """Evens the vertex spacing (Laplacian steps) and puts the vertices back on the surface."""
    n = len(verts)
    k = faces.shape[1]
    a = faces.ravel()
    b = np.roll(faces, -1, axis=1).ravel()
    src = np.concatenate([a, b])
    dst = np.concatenate([b, a])
    count = np.bincount(src, minlength=n).astype(float)
    for _ in range(iterations):
        mean = np.stack([np.bincount(src, weights=verts[dst, c], minlength=n) for c in range(3)], 1)
        mean /= np.maximum(count, 1)[:, None]
        verts = verts + amount * (mean - verts)
        verts = refine(field, verts, 1)
    del k
    return verts


def project_sphere(field, centre, subdivisions: int = 64, reach: float = 0.25, steps: int = 96):
    """A cube-sphere wrapped onto a field that is star-shaped about `centre` (each ray from it
    leaves the surface once): the outermost crossing along each ray. Returns (verts, quads)."""
    c = np.asarray(centre, float)
    n = subdivisions
    t = np.linspace(-1.0, 1.0, n + 1)
    faces, verts, offset = [], [], 0
    for axis in range(3):
        for sign in (1.0, -1.0):
            u, v = np.meshgrid(t, t, indexing="ij")
            pts = np.zeros((n + 1, n + 1, 3))
            pts[..., axis] = sign
            pts[..., (axis + 1) % 3] = u
            pts[..., (axis + 2) % 3] = v
            verts.append(pts.reshape(-1, 3))
            i = np.arange(n)
            ii, jj = np.meshgrid(i, i, indexing="ij")
            q = np.stack([ii * (n + 1) + jj, (ii + 1) * (n + 1) + jj, (ii + 1) * (n + 1) + jj + 1, ii * (n + 1) + jj + 1], -1)
            q = q.reshape(-1, 4) + offset
            faces.append(q if sign > 0 else q[:, ::-1])
            offset += (n + 1) ** 2
    verts = np.concatenate(verts)
    faces = np.concatenate(faces)
    # Weld the cube's shared edges.
    key = np.round(verts * 1e5).astype(np.int64)
    _, first, inverse = np.unique(key, axis=0, return_index=True, return_inverse=True)
    verts = verts[first]
    faces = inverse.reshape(-1)[faces]
    dirs = verts / np.linalg.norm(verts, axis=1)[:, None]
    # March in from outside: the last outside sample before the first inside one, then bisect.
    lo_t = np.full(len(dirs), reach)
    hi_t = np.full(len(dirs), reach)
    found = np.zeros(len(dirs), bool)
    for s in range(steps + 1):
        tt = reach * (1.0 - s / steps)
        d = field(c + dirs * tt)
        newly = (~found) & (d < 0)
        hi_t[newly] = tt
        lo_t[newly] = tt + reach / steps
        found |= newly
    for _ in range(24):
        mid = 0.5 * (lo_t + hi_t)
        inside = field(c + dirs * mid[:, None]) < 0
        hi_t = np.where(inside, mid, hi_t)
        lo_t = np.where(inside, lo_t, mid)
    return c + dirs * hi_t[:, None], faces


# Blender meshes --------------------------------------------------------------------------------

def to_object(name: str, verts, faces, smooth: bool = True) -> bpy.types.Object:
    """A mesh object from numpy arrays, faces turned outward (by signed volume)."""
    faces = np.asarray(faces, np.int64)
    tri = np.concatenate([faces[:, [0, 1, 2]], faces[:, [0, 2, 3]]])
    vol = np.einsum("ij,ij->i", verts[tri[:, 0]], np.cross(verts[tri[:, 1]], verts[tri[:, 2]])).sum()
    if vol < 0:
        faces = faces[:, ::-1]
    me = bpy.data.meshes.new(name)
    me.vertices.add(len(verts))
    me.vertices.foreach_set("co", np.asarray(verts, np.float32).ravel())
    me.loops.add(faces.size)
    me.loops.foreach_set("vertex_index", faces.astype(np.int32).ravel())
    me.polygons.add(len(faces))
    me.polygons.foreach_set("loop_start", (np.arange(len(faces)) * faces.shape[1]).astype(np.int32))
    me.update()
    me.validate()
    me.polygons.foreach_set("use_smooth", np.full(len(me.polygons), smooth))
    obj = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def verts_of(obj) -> np.ndarray:
    co = np.empty(len(obj.data.vertices) * 3, np.float32)
    obj.data.vertices.foreach_get("co", co)
    return co.reshape(-1, 3).astype(float)


def set_verts(obj, verts) -> None:
    obj.data.vertices.foreach_set("co", np.asarray(verts, np.float32).ravel())
    obj.data.update()


def decimated(obj, triangles: int, name: str, symmetric: bool = False) -> bpy.types.Object:
    """A decimated copy with about `triangles` triangles (the game mesh of a dense surface)."""
    copy = obj.copy()
    copy.data = obj.data.copy()
    copy.name = copy.data.name = name
    bpy.context.scene.collection.objects.link(copy)
    have = sum(len(p.vertices) - 2 for p in copy.data.polygons)
    if have > triangles:
        mod = copy.modifiers.new("Decimate", "DECIMATE")
        mod.decimate_type = "COLLAPSE"
        mod.ratio = triangles / have
        mod.use_collapse_triangulate = True
        if symmetric:
            mod.use_symmetry = True
            mod.symmetry_axis = "Y"
        bpy.context.view_layer.objects.active = copy
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return copy


def set_attribute(obj, name: str, values, kind: str = "FLOAT") -> None:
    """A per-vertex attribute (FLOAT, or FLOAT_COLOR from an (N, 3|4) array) the materials read."""
    me = obj.data
    if name in me.attributes:
        me.attributes.remove(me.attributes[name])
    values = np.asarray(values, np.float32)
    if kind == "FLOAT_COLOR":
        if values.shape[1] == 3:
            values = np.concatenate([values, np.ones((len(values), 1), np.float32)], 1)
        attr = me.attributes.new(name, "FLOAT_COLOR", "POINT")
        attr.data.foreach_set("color", values.ravel())
    else:
        attr = me.attributes.new(name, "FLOAT", "POINT")
        attr.data.foreach_set("value", values.ravel())
