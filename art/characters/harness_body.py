"""The soft parts of a harness character, modelled as signed distance fields (harness_sdf.py)
and meshed from code; art/characters/harness.py weights and rigs them.

- The work suit: a coverall over an adult figure 1.82 m tall (the eye at 1.70 m): a torso lofted
  from anthropometric sections (seat, waist, ribcage, chest, shoulder line), the shoulder girdle
  and deltoids, sleeves and trouser legs with the cloth's ease, a stand collar and a zip placket,
  cuffs, the trousers bloused into the boots, pockets, knee patches, and the folds cloth makes at
  the elbow, the back of the knee, the waist and the crotch. It is built in an A-pose (the arms
  35 degrees out from the rest pose) so the sleeves stay clear of the torso while it is weighted;
  harness.py then poses it down onto the rest skeleton.
- The head: cranium, brow ridge, eye sockets with lids, cheekbones, nose, lips, jaw and chin,
  ears, cropped hair and the neck, wrapped from a sphere; eyeballs of their own.
- The hands: gloved (or the Climber's taped), knuckles, palm pads, the thumb's ball, fingers in
  three joints, relaxed in a light curl; a cuff under the sleeve.
- The boots: a last-shaped upper with the shaft to the class's boot height, the padded collar,
  toe cap and heel counter, and a separate sole with its welt, heel and tread lugs.

Every piece is meshed dense (the detail the textures are baked from) and decimated (the game
mesh). Proportions follow the bulk and limb factors of each class (harness.py CLASSES), with
width growing slower than depth so the arms stay outside the torso on the shared skeleton.
"""
from __future__ import annotations

import math

import numpy as np

import harness_sdf as sdf
from harness_sdf import ellipsoid, round_cone, smax, smin, smoothstep

APOSE = 35.0  # Degrees the arms are raised from the rest pose while the suit is modelled and weighted


def bone_table(bones) -> dict:
    return {name: (np.array(head, float), np.array(tail, float)) for name, _parent, head, tail in bones}


def apose_bones(bones) -> list:
    """The skeleton with each arm chain swung out by APOSE degrees about the shoulder joint (X)."""
    out = []
    joints = {s: np.array(next(b[2] for b in bones if b[0] == f"upperarm_{s}"), float) for s in "lr"}
    for name, parent, head, tail in bones:
        side = name[-1] if name[-2:] in ("_l", "_r") else None
        if side and name.split("_")[0] in ("upperarm", "lowerarm", "hand"):
            j = joints[side]
            rot = sdf.rotation("X", APOSE if side == "l" else -APOSE)
            head = tuple(j + rot @ (np.array(head, float) - j))
            tail = tuple(j + rot @ (np.array(tail, float) - j))
        out.append((name, parent, head, tail))
    return out


# The suit ---------------------------------------------------------------------------------------

class Suit:
    """The coverall's field in the A-pose, and what the materials and the gear need to know
    about it (the boot top, the patches). `spec` is the class's entry in harness.CLASSES."""

    def __init__(self, spec: dict, bones):
        self.spec = spec
        self.k = k = spec["bulk"]
        limb = spec.get("limb", 1.0)
        suit = spec.get("suit", {})
        self.ease = ease = suit.get("ease", 1.0)       # 1 a loose work coverall, under 1 fitted
        self.width = 1.0 + (k - 1.0) * 0.5              # Width grows slower than depth
        self.limb = 1.0 + (k * limb - 1.0) * 0.6
        self.boot_top = spec.get("boot_top", 0.2)
        self.tuck = spec.get("boots", ("work", "work"))[0] == "tall"
        self.patches = suit.get("patches", ())
        self.pockets = suit.get("pockets", ("thigh_r", "seat"))
        self.knees = suit.get("knee_pads", True)
        self.folds = suit.get("folds", 1.0)
        self.shoulder = suit.get("shoulder", 1.0)  # The deltoid's breadth (the Climber's is slight)
        self.bones = bone_table(apose_bones(bones))
        w, d = self.width, k
        e = 0.008 * (ease - 1.0)  # Cloth ease beyond the loose default (negative: fitted)
        rows = [  # z, cx, cy, front, back, left, right, n: the torso's sections, clothed
            (0.885, -0.012, 0, 0.074 * d, 0.088 * d, 0.07, 0.07, 2.0),
            (0.93, -0.012, 0, 0.094 * d, 0.113 * d, 0.15 * w, 0.15 * w, 2.3),
            (0.98, -0.006, 0, 0.104 * d, 0.112 * d, 0.168 * w, 0.168 * w, 2.3),
            (1.04, -0.001, 0, 0.108 * d, 0.105 * d, 0.165 * w, 0.165 * w, 2.3),
            (1.12, 0.004, 0, 0.112 * d, 0.100 * d, 0.158 * w, 0.158 * w, 2.3),
            (1.20, 0.009, 0, 0.118 * d, 0.104 * d, 0.161 * w, 0.161 * w, 2.3),
            (1.30, 0.012, 0, 0.123 * d, 0.110 * d, 0.167 * w, 0.167 * w, 2.4),
            (1.38, 0.008, 0, 0.116 * d, 0.116 * d, 0.168 * w, 0.168 * w, 2.6),
            (1.44, 0.000, 0, 0.094 * d, 0.104 * d, 0.152 * w, 0.152 * w, 2.7),
            (1.48, -0.008, 0, 0.068 * d, 0.078 * d, 0.105 * w, 0.105 * w, 2.4),
        ]
        rows = [(z, cx, cy, f + e, b + e, l + e, r + e, n) for z, cx, cy, f, b, l, r, n in rows]
        self.torso = sdf.Loft((0, 0, 0), np.eye(3), rows, round_ends=0.05)
        self.limbs = []
        for s, side in ((1.0, "l"), (-1.0, "r")):
            self.limbs.append(self._arm(side, s))
            self.limbs.append(self._leg(side, s))

    def _arm(self, side: str, s: float):
        j, e = self.bones[f"upperarm_{side}"]
        _, wr = self.bones[f"lowerarm_{side}"]
        L, ez = self.limb, 0.006 * (self.ease - 1.0)
        sh = self.shoulder
        up = sdf.Loft(j, sdf.frame(e - j, out_sign=s), [
            (-0.02, 0, 0.000, 0.050 * L, 0.052 * L, 0.052 * L, 0.046 * L, 2),
            (0.03, 0, 0.006 * sh, 0.058 * L, 0.058 * L, 0.062 * L * sh, 0.053 * L, 2),
            (0.10, 0, 0.006 * sh, 0.059 * L, 0.057 * L, 0.061 * L * sh, 0.055 * L, 2),
            (0.18, 0, 0.002, 0.056 * L, 0.055 * L, 0.056 * L, 0.054 * L, 2),
            (0.26, -0.004, 0, 0.053 * L, 0.054 * L, 0.052 * L, 0.051 * L, 2),
            (0.31, -0.006, 0, 0.051 * L, 0.053 * L, 0.050 * L, 0.049 * L, 2)], round_ends=0.03)
        fl = np.linalg.norm(wr - e)
        fore = sdf.Loft(e, sdf.frame(wr - e, out_sign=s), [
            (-0.03, -0.004, 0, 0.051 * L, 0.053 * L, 0.050 * L, 0.049 * L, 2),
            (0.04, 0.0, 0, 0.052 * L, 0.051 * L, 0.050 * L, 0.049 * L, 2),
            (0.12, 0.0, 0, 0.049 * L, 0.047 * L, 0.047 * L, 0.046 * L, 2),
            (fl - 0.07, 0.0, 0, 0.047 * L, 0.045 * L, 0.045 * L, 0.044 * L, 2),
            (fl - 0.045, 0.0, 0, 0.046 * L, 0.044 * L, 0.044 * L, 0.043 * L, 2),
            (fl - 0.035, 0.0, 0, 0.040, 0.039, 0.039, 0.038, 2),
            (fl - 0.004, 0.0, 0, 0.039, 0.038, 0.038, 0.037, 2)], round_ends=0.006)
        up.y[:, 2:6] += ez
        fore.y[:-2, 2:6] += ez
        # The deltoid: the shoulder's cap, in the arm's frame just below and outside the joint.
        R = sdf.frame(e - j, out_sign=s)
        sh = self.shoulder
        delt_c = j + R @ np.array([0.004, 0.009 * sh, 0.035])
        delt_r = np.array([0.060, 0.058 * sh, 0.082]) * L
        return dict(kind="arm", side=side, s=s, up=up, fore=fore, joint=j, elbow=e, wrist=wr,
                    delt=(delt_c, delt_r, R), frame_up=R, frame_fore=sdf.frame(wr - e, out_sign=s))

    def _leg(self, side: str, s: float):
        hip, knee = self.bones[f"thigh_{side}"]
        _, ankle = self.bones[f"calf_{side}"]
        L, ez = self.limb, 0.007 * (self.ease - 1.0)
        inner = min(1.0, 0.084 / (0.08 * L))  # Inner thighs stop short of each other
        thigh = sdf.Loft(hip, sdf.frame(knee - hip, out_sign=s), [
            (-0.05, 0.000, 0.010, 0.095 * L, 0.110 * L, 0.085 * L, 0.075 * L * inner, 2.1),
            (0.05, 0.008, 0.008, 0.096 * L, 0.104 * L, 0.089 * L, 0.080 * L * inner, 2.0),
            (0.15, 0.010, 0.005, 0.092 * L, 0.094 * L, 0.088 * L, 0.080 * L * inner, 2.0),
            (0.25, 0.010, 0.002, 0.087 * L, 0.086 * L, 0.085 * L, 0.078 * L * inner, 2.0),
            (0.35, 0.012, 0.000, 0.081 * L, 0.079 * L, 0.080 * L, 0.075 * L, 2.0),
            (0.45, 0.014, 0.000, 0.077 * L, 0.075 * L, 0.076 * L, 0.072 * L, 2.0)], round_ends=0.03)
        thigh.y[:, 2:6] += ez
        gap = abs(hip[1]) - 0.011  # The legs never meet: an inner side stops 1.1 cm short of the midline
        thigh.y[:, 5] = np.minimum(thigh.y[:, 5], gap + thigh.y[:, 1])
        top = self.boot_top
        rows = []
        for z, r in ((0.55, 0.078), (0.46, 0.077), (0.36, 0.075), (0.26, 0.073), (0.18, 0.071)):
            if z > top + 0.06:
                rows.append((knee[2] - z, 0.012 * (z - 0.1) / 0.42, 0.0, r * L + ez, r * L + ez, r * L + ez, (r - 0.004) * L + ez, 2.0))
        blouse = max(0.074 * L, 0.064) + 0.004
        if self.tuck:  # Bloused over the boot top, then tucked into it
            hem = ((top + 0.05, blouse - 0.002), (top + 0.022, blouse + 0.002), (top - 0.004, 0.062 * min(L, 1.15)),
                   (top - 0.035, 0.054 * min(L, 1.15)))
        else:  # Bloused and hanging over the boot's top
            hem = ((top + 0.06, blouse - 0.003), (top + 0.03, blouse + 0.001), (top + 0.005, blouse + 0.002), (top - 0.018, blouse - 0.004))
        for z, r in hem:
            rows.append((knee[2] - z, 0.0, 0.0, r, r, r, r - 0.002, 2.0))
        calf = sdf.Loft(knee, sdf.frame(ankle - knee, out_sign=s), rows, round_ends=0.01)
        calf.y[:, 5] = np.minimum(calf.y[:, 5], gap + calf.y[:, 1])
        return dict(kind="leg", side=side, s=s, thigh=thigh, calf=calf, hip=hip, knee=knee, ankle=ankle)

    # The field ------------------------------------------------------------------------------

    def __call__(self, p):
        k, w = self.k, self.width
        d = self.torso(p)
        # The shoulder girdle and the trapezius up to the neck.
        for s in (1.0, -1.0):
            d = smin(d, round_cone(p, (-0.012, 0.05 * s, 1.47), (-0.004, 0.172 * s * w, 1.446), 0.06 * k, 0.05 * self.limb), 0.035)
        d = smin(d, round_cone(p, (-0.012, 0, 1.43), (-0.002, 0, 1.53), 0.074 * w, 0.064), 0.03)
        # The seat.
        for s in (1.0, -1.0):
            d = smin(d, ellipsoid(p, (-0.062, 0.07 * s, 0.905), np.array((0.06, 0.078, 0.085)) * k), 0.04)
        d = smin(d, self.collar(p), 0.01)
        core, legs = d, []
        for limb in self.limbs:
            if limb["kind"] == "arm":
                arm = smin(limb["up"](p), limb["fore"](p), 0.02)
                c, r, R = limb["delt"]
                arm = smin(arm, ellipsoid(p, c, r, R), 0.03)
                core = smin(core, arm, 0.025)
            else:
                legs.append(smin(limb["thigh"](p), limb["calf"](p), 0.02))
        # Each leg blends into the hips on its own and only meets the other with a hard edge, so
        # no web of cloth forms between the thighs to tear when they part.
        d = core
        for leg in legs:
            d = np.minimum(d, smin(core, leg, 0.045))
        return d - self.surface(p)

    def collar(self, p):
        """The stand collar: a short tube round the neck, open at the front, its top edge rolled."""
        R = sdf.frame((0.18, 0.0, 1.0))
        q = (p - np.array((-0.004, 0.0, 1.51))) @ R
        radial = np.sqrt(q[:, 0] ** 2 + (q[:, 1] * 1.08) ** 2)
        wall = np.abs(radial - 0.068) - 0.0045
        height = np.abs(q[:, 2]) - 0.026 + 0.012 * smoothstep(0.0, 0.07, q[:, 0])  # Lower at the front
        ring = smax(wall, height, 0.004)
        gap = 0.012 - np.abs(q[:, 1]) + 0.05 * (q[:, 0] < 0.0)  # The opening at the zip
        return smax(ring, gap, 0.003)

    def surface(self, p):
        """Outward displacement (m): placket, pockets, knee patches, folds, the cloth's drape."""
        x, y, z = p[:, 0], p[:, 1], p[:, 2]
        out = 0.0015 * (sdf.fbm(p, 9.0, 3, seed=11) - 0.5) * 2.0  # The cloth never lies perfectly smooth
        # The zip placket down the centre front.
        out = out + 0.0018 * smoothstep(0.026, 0.021, np.abs(y)) * (x > 0.03) * smoothstep(0.83, 0.86, z) * smoothstep(1.53, 1.5, z)
        if "seat" in self.pockets:
            for s in (1.0, -1.0):
                out = out + 0.0035 * self._patch(-x, y - 0.075 * s, z - 0.94, 0.06, 0.065)
        noise = sdf.value_noise(p, 30.0, seed=5)
        for limb in self.limbs:
            s = limb["s"]
            if limb["kind"] == "leg":
                q = limb["thigh"].local(p)
                theta = np.arctan2(q[:, 1], q[:, 0])  # 0 forward, +pi/2 outward
                r = np.sqrt(q[:, 0] ** 2 + q[:, 1] ** 2)
                arc = theta * np.maximum(r, 0.05)
                if f"thigh_{limb['side']}" in self.pockets:  # The cargo pocket on the outer thigh and its flap
                    body = self._patch(arc - 1.5708 * 0.088, q[:, 2] - 0.29, 0.0, 0.075, 0.09, depth=False)
                    flap = self._patch(arc - 1.5708 * 0.088, q[:, 2] - 0.205, 0.0, 0.082, 0.028, depth=False)
                    near = smoothstep(0.13, 0.11, r)
                    out = out + near * (0.006 * body + 0.004 * flap)
                if self.knees:  # Reinforced knees: a patch over the kneecap
                    knee = self._patch(arc, q[:, 2] - 0.43, 0.0, 0.068, 0.08, depth=False, soft=0.012) * smoothstep(0.12, 0.1, r)
                    out = out + 0.002 * knee
                # Folds fanning from the crotch across the front of the upper thigh.
                inner = smoothstep(-0.2, -1.2, theta) * smoothstep(0.22, 0.02, q[:, 2]) * smoothstep(-0.05, 0.02, q[:, 2])
                t = q[:, 2] * 0.8 - arc * 0.6
                out = out + self.folds * 0.003 * inner * self._ridges(t + 0.006 * noise, 0.03) * smoothstep(0.13, 0.1, r)
                # The back of the knee and the bunching above the boot.
                qc = limb["calf"].local(p)
                th = np.arctan2(qc[:, 1], qc[:, 0])
                rc = np.sqrt(qc[:, 0] ** 2 + qc[:, 1] ** 2)
                back = smoothstep(1.2, 2.6, np.abs(th)) * smoothstep(0.08, 0.0, np.abs(qc[:, 2] + 0.0))
                out = out + self.folds * 0.004 * back * self._ridges(qc[:, 2] + 0.004 * np.sin(th * 3) + 0.006 * noise, 0.024) * smoothstep(0.12, 0.09, rc)
                kz = limb["knee"][2] - qc[:, 2]  # Height above the floor along the calf
                lo_z = self.boot_top - (0.005 if self.tuck else 0.02)
                stack = smoothstep(lo_z, lo_z + 0.015, kz) * smoothstep(self.boot_top + 0.075, self.boot_top + 0.04, kz)
                out = out + self.folds * 0.0016 * stack * self._ridges(kz + 0.008 * np.sin(th * 2 + 1.0) + 0.005 * np.sin(th * 5) + 0.016 * noise, 0.021) * smoothstep(0.11, 0.08, rc)
            else:
                qf = limb["fore"].local(p)
                th = np.arctan2(qf[:, 1], qf[:, 0])
                rf = np.sqrt(qf[:, 0] ** 2 + qf[:, 1] ** 2)
                near = smoothstep(0.085, 0.065, rf)
                # The inside of the elbow, where the sleeve folds when the arm bends.
                front = smoothstep(1.7, 0.2, np.abs(th)) * smoothstep(0.08, 0.0, np.abs(qf[:, 2] + 0.005))
                out = out + self.folds * 0.0035 * front * near * self._ridges(qf[:, 2] + 0.004 * np.sin(th * 3) + 0.006 * noise, 0.026)
                # The sleeve gathered above the cuff.
                fl = np.linalg.norm(limb["wrist"] - limb["elbow"])
                gather = smoothstep(fl - 0.11, fl - 0.08, qf[:, 2]) * smoothstep(fl - 0.03, fl - 0.045, qf[:, 2])
                out = out + self.folds * 0.0025 * gather * near * self._ridges(qf[:, 2] + 0.004 * np.sin(th * 2) + 0.006 * noise, 0.013)
        # The waist bunched under the belt.
        torso_near = smoothstep(0.25, 0.2, np.sqrt((x / 1.2) ** 2 + y ** 2))
        band = smoothstep(0.98, 1.01, z) * smoothstep(1.11, 1.07, z) * torso_near
        around = np.arctan2(y, x)
        wobble = 0.008 * np.sin(around * 3 + 0.5) + 0.005 * np.sin(around * 7) + 0.012 * noise
        out = out + self.folds * 0.0022 * band * self._ridges(z + wobble, 0.026) * (0.6 + 0.4 * sdf.value_noise(p, 12.0, seed=8))
        return out

    @staticmethod
    def _ridges(t, wavelength):
        """Rounded cloth folds: soft crests, narrower valleys."""
        c = 0.5 + 0.5 * np.cos(2 * math.pi * t / wavelength)
        return c ** 1.6 - 0.35

    @staticmethod
    def _patch(a, b, c, half_a, half_b, depth=True, soft=0.006):
        """A rounded rectangle mask in two surface coordinates; with depth, only on the side
        where the first coordinate is positive (the back, for the seat)."""
        m = smoothstep(half_a + soft * 0.5, half_a - soft * 0.5, np.abs(b) if depth else np.abs(a))
        m = m * smoothstep(half_b + soft * 0.5, half_b - soft * 0.5, np.abs(c) if depth else np.abs(b))
        if depth:
            m = m * (a > 0.04)
        return m

    def layout(self, bones) -> dict:
        """Where the seams are in the rest pose (harness_look.suit_fabric sews them there)."""
        t = bone_table(bones)
        out = {"shoulders": [tuple(t[f"upperarm_{s}"][0]) for s in "lr"], "cuffs": [], "hems": [], "patches": []}
        for side in "lr":
            e, w = t[f"lowerarm_{side}"]
            axis = (w - e) / np.linalg.norm(w - e)
            fl = float(np.linalg.norm(w - e))
            out["cuffs"].append((tuple(e), tuple(axis), (fl - 0.035, fl - 0.011)))
        out["hems"].append(self.boot_top + (0.022 if self.tuck else -0.008))
        for limb in self.limbs:
            if limb["kind"] != "leg":
                continue
            R = limb["thigh"].R
            frame_ = (tuple(limb["hip"]), tuple(R[:, 0]), tuple(R[:, 1]), tuple(R[:, 2]))
            if self.knees:
                out["patches"].append(frame_ + ((0.0, 0.43), (0.068, 0.08), "limb"))
            if f"thigh_{limb['side']}" in self.pockets:
                out["patches"].append(frame_ + ((1.5708 * 0.088, 0.29), (0.075, 0.09), "limb"))
                out["patches"].append(frame_ + ((1.5708 * 0.088, 0.205), (0.082, 0.028), "limb"))
        if "seat" in self.pockets:
            for s in (1.0, -1.0):
                out["patches"].append(((0.0, 0.075 * s, 0.94), (-1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0),
                                       (0.0, 0.0), (0.06, 0.065), "plane"))
        return out

    def grime(self, v, bones):
        """Where work dirties a coverall (0 to 1 per vertex, rest pose): knees, shins and the
        trouser bottoms, the seat, the cuffs and elbows, the chest where the plate rubs; and the
        sweat under the arms, at the collar and down the back."""
        t = bone_table(bones)
        x, y, z = v[:, 0], v[:, 1], v[:, 2]
        n = sdf.fbm(v, 14.0, 3, seed=31)
        g = 0.75 * smoothstep(self.boot_top + 0.22, self.boot_top, z)
        g = g + 0.7 * smoothstep(0.62, 0.55, np.abs(z - 0.5) + 0.48) * smoothstep(0.0, 0.05, x)
        g = g + 0.5 * smoothstep(0.03, -0.05, x) * smoothstep(0.85, 0.9, z) * smoothstep(1.02, 0.96, z)
        g = g + 0.35 * smoothstep(0.04, 0.09, x) * smoothstep(0.13, 0.09, np.abs(y)) * smoothstep(1.18, 1.24, z) * smoothstep(1.48, 1.42, z)
        g = g + 0.3 * smoothstep(0.03, 0.08, x) * smoothstep(0.62, 0.7, z) * smoothstep(0.9, 0.82, z)  # Hands wiped on the thighs
        for side in "lr":
            e, w = t[f"lowerarm_{side}"]
            g = g + 0.65 * smoothstep(0.09, 0.03, np.linalg.norm(v - w, axis=1))
            g = g + 0.45 * smoothstep(0.08, 0.03, np.linalg.norm(v - e, axis=1)) * smoothstep(0.0, -0.03, x)
        grime = np.clip(g * (0.55 + 0.9 * n), 0.0, 1.0)
        sweat = 0.0
        for s in (1.0, -1.0):
            sweat = sweat + smoothstep(0.08, 0.03, np.linalg.norm(v - np.array((-0.005, 0.165 * s, 1.33)), axis=1))
        sweat = sweat + smoothstep(1.44, 1.5, z) * smoothstep(0.0, -0.04, x) * smoothstep(0.12, 0.06, np.abs(y))
        sweat = sweat + 0.6 * smoothstep(-0.03, -0.08, x) * smoothstep(0.1, 0.03, np.abs(y)) * smoothstep(1.12, 1.25, z) * smoothstep(1.45, 1.38, z)
        sweat = np.clip(sweat * (0.6 + 0.8 * sdf.fbm(v, 9.0, 2, seed=33)), 0.0, 1.0)
        return grime, sweat

    def bounds(self):
        reach = max(0.62, 0.19 + (0.56 + 0.17) * math.sin(math.radians(8 + APOSE)) + 0.1)
        return (-0.25, -reach, 0.0), (0.27, reach, 1.6)


# The head ---------------------------------------------------------------------------------------

class Head:
    """A neutral adult head and neck in the head's frame (its origin between the ears, 1.69 m up):
    star-shaped about a point behind the nose, so a sphere can be wrapped onto it."""

    def __init__(self, spec: dict):
        self.o = np.array((0.012, 0.0, 1.69))
        self.eye_r = 0.0118
        self.eyes = [np.array((0.0738, 0.0318 * s, 0.0)) for s in (1.0, -1.0)]
        self.hair = spec.get("hair", 0.003)
        self.centre = self.o + np.array((0.012, 0.0, -0.02))

    def world_eyes(self):
        return [self.o + c for c in self.eyes]

    def __call__(self, pw):
        p = pw - self.o
        d = ellipsoid(p, (-0.010, 0, 0.026), (0.098, 0.0765, 0.097))  # The cranium
        d = smin(d, ellipsoid(p, (0.038, 0, 0.036), (0.062, 0.066, 0.068)), 0.02)  # The forehead
        for s in (1.0, -1.0):  # The parietals: the skull is broad at the top and back
            d = smin(d, ellipsoid(p, (-0.03, 0.03 * s, 0.045), (0.074, 0.05, 0.066)), 0.02)
        d = smin(d, ellipsoid(p, (0.046, 0, -0.03), (0.054, 0.061, 0.052)), 0.02)  # The upper jaw and mid-face
        for s in (1.0, -1.0):
            d = smin(d, ellipsoid(p, (0.06, 0.047 * s, -0.013), (0.024, 0.021, 0.015)), 0.012)  # Cheekbone
            d = smin(d, round_cone(p, (-0.006, 0.058 * s, -0.056), (0.062, 0.021 * s, -0.095), 0.017, 0.0135), 0.022)  # Jaw line
            d = smin(d, ellipsoid(p, (0.048, 0.046 * s, -0.048), (0.03, 0.021, 0.028)), 0.016)  # The cheek's fullness
            d = smin(d, ellipsoid(p, (0.064, 0.027 * s, -0.062), (0.028, 0.022, 0.025)), 0.012)  # Beside the mouth
        d = smin(d, ellipsoid(p, (0.048, 0, -0.07), (0.046, 0.048, 0.034)), 0.02)  # The lower face
        d = smin(d, ellipsoid(p, (0.077, 0, -0.097), (0.019, 0.023, 0.015)), 0.012)  # The chin
        d = smin(d, round_cone(p, (0.08, -0.038, 0.019), (0.08, 0.038, 0.019), 0.0095, 0.0095), 0.016)  # Brow ridge
        # The nose: bridge, sides, tip, wings and nostrils.
        d = smin(d, round_cone(p, (0.089, 0, 0.004), (0.113, 0, -0.033), 0.0068, 0.0088), 0.008)
        d = smin(d, ellipsoid(p, (0.086, 0, -0.022), (0.02, 0.0145, 0.025)), 0.01)
        d = smin(d, ellipsoid(p, (0.1095, 0, -0.037), (0.0096, 0.0102, 0.0092)), 0.006)
        for s in (1.0, -1.0):
            d = smin(d, ellipsoid(p, (0.0985, 0.0152 * s, -0.0435), (0.0102, 0.0082, 0.0072)), 0.006)
            d = smax(d, -ellipsoid(p, (0.1055, 0.0078 * s, -0.0515), (0.0062, 0.0034, 0.0024)), 0.002)
        # The lips and the line of the mouth.
        d = smin(d, ellipsoid(p, (0.0915, 0, -0.0662), (0.0115, 0.0238, 0.0068)), 0.006)
        d = smin(d, ellipsoid(p, (0.0888, 0, -0.0778), (0.0108, 0.0208, 0.0068)), 0.006)
        d = smax(d, -ellipsoid(p, (0.1, 0, -0.0716), (0.0135, 0.0245, 0.0010)), 0.0012)
        d = smax(d, -ellipsoid(p, (0.1015, 0, -0.058), (0.004, 0.0035, 0.004)), 0.003)  # The philtrum
        # The ears: a flap on each side with its bowl.
        for s in (1.0, -1.0):
            R = sdf.rotation("Z", -22.0 * s) @ sdf.rotation("Y", 14.0)
            c = np.array((-0.012, 0.074 * s, -0.006))
            ear = ellipsoid(p, c, (0.017, 0.0085, 0.031), R)
            ear = smax(ear, -ellipsoid(p, c + np.array((0.002, 0.0085 * s, -0.002)), (0.0095, 0.003, 0.015), R), 0.003)
            d = smin(d, ear, 0.006)
        for c, s in zip(self.eyes, (1.0, -1.0)):
            d = self.lids(p, c, s, d)
        # The neck, down into the collar; the Adam's apple.
        d = smin(d, round_cone(p, (-0.014, 0, -0.06), (-0.026, 0, -0.205), 0.053, 0.06), 0.028)
        d = smin(d, ellipsoid(p, (0.032, 0, -0.135), (0.012, 0.014, 0.018)), 0.012)
        return d - self.hair * self.hair_mask(p)

    def lids(self, p, c, s, d):
        """The eyelids: the face bulges over the eyeball (a shell just outside it) and is cut open
        in an almond where the eye shows; the eyeball is a mesh of its own."""
        q = p - c
        r = np.maximum(sdf.length(q), 1e-9)
        # The orbit cleared in front of the eye, then the lids' mound over the eyeball.
        d = smax(d, -ellipsoid(p, c + np.array((0.014, 0.0, 0.002)), (0.0135, 0.0195, 0.0145)), 0.006)
        d = smin(d, ellipsoid(p, c + np.array((0.0004, 0.0, 0.0006)), (self.eye_r + 0.0026, 0.0168, 0.0142)), 0.006)
        az = np.arctan2(s * q[:, 1], q[:, 0])     # Toward the outer corner positive
        el = np.arcsin(np.clip(q[:, 2] / r, -1, 1))
        centre = 0.02 + 0.09 * az
        half = 0.37 * np.clip(1.0 - (az / 1.25) ** 2, 0.0, 1.0) ** 0.6 - 0.015
        almond = (np.abs(el - centre) - half) * r
        almond = np.where(q[:, 0] > 0, almond, 0.01)
        cut = np.maximum(almond, (self.eye_r - 0.0025) - r)
        return smax(d, -cut, 0.0012)

    @staticmethod
    def hair_mask(p):
        """Cropped hair: above the hairline at the forehead and temples, to the nape at the back."""
        x, y, z = p[:, 0], p[:, 1], p[:, 2]
        line = 0.058 - 0.05 * smoothstep(0.075, -0.01, x) - 0.06 * smoothstep(0.0, -0.08, x) + 0.012 * smoothstep(0.04, 0.07, np.abs(y))
        line = line - 0.035 * smoothstep(0.03, 0.0, np.abs(x - 0.005)) * smoothstep(0.06, 0.075, np.abs(y))  # Sideburns
        return smoothstep(line - 0.006, line + 0.01, z)

    def colour(self, pw, skin):
        """Per-vertex skin colour: the warmth of cheeks, nose and ears, the lips, the darker
        sockets, brows, stubble and hair (linear RGB)."""
        p = pw - self.o
        x, y, z = p[:, 0], p[:, 1], p[:, 2]
        base = np.array(skin, float)[None, :3] * (0.94 + 0.12 * sdf.fbm(pw, 60.0, 3, seed=21))[:, None]
        red = np.array((0.75, 0.25, 0.2))
        warm = (0.35 * np.exp(-(((x - 0.075) / 0.02) ** 2 + ((np.abs(y) - 0.042) / 0.022) ** 2 + ((z + 0.03) / 0.02) ** 2))
                + 0.3 * np.exp(-(((x - 0.112) / 0.015) ** 2 + (y / 0.018) ** 2 + ((z + 0.04) / 0.015) ** 2))
                + 0.35 * smoothstep(0.065, 0.08, np.abs(y)) * smoothstep(0.03, -0.0, x) * smoothstep(-0.05, 0.03, z) * smoothstep(0.05, 0.02, z))
        col = base * (1 - warm[:, None] * 0.35) + (base * red)[:] * warm[:, None] * 0.35 * 2.0
        lips = np.exp(-(((x - 0.09) / 0.014) ** 2 + (y / 0.022) ** 4 + ((z + 0.072) / 0.0095) ** 2))
        col = col * (1 - 0.45 * lips[:, None]) + (np.array(skin)[None, :3] * np.array((0.95, 0.42, 0.42))) * 0.45 * lips[:, None] * 1.4
        sockets = sum(np.exp(-(((x - (c[0] + 0.012)) / 0.016) ** 2 + ((y - c[1]) / 0.02) ** 2 + ((z - c[2] + 0.004) / 0.016) ** 2)) for c in self.eyes)
        col = col * (1 - 0.28 * sockets[:, None])
        brow = sum(np.exp(-(((x - 0.094) / 0.012) ** 2 + ((y - c[1] * 1.08) / 0.019) ** 4 + ((z - 0.0185 + 0.06 * (np.abs(y) - 0.03) ** 2 * 20) / 0.0042) ** 2))
                   for c in self.eyes)
        brow = brow * (0.55 + 0.45 * sdf.value_noise(pw, 900.0, seed=3))
        hair_col = np.array((0.035, 0.026, 0.02))
        col = col * (1 - 0.85 * brow[:, None]) + hair_col * 0.85 * brow[:, None]
        jaw = smoothstep(-0.02, -0.06, z) * smoothstep(0.02, 0.06, x) * (1 - lips) * smoothstep(-0.16, -0.11, z)
        col = col * (1 - 0.16 * jaw[:, None]) + hair_col * 2.0 * 0.16 * jaw[:, None]  # Stubble
        hair = self.hair_mask(p) * (0.85 + 0.15 * sdf.value_noise(pw, 400.0, seed=4))
        col = col * (1 - hair[:, None]) + hair_col * (0.8 + 0.4 * sdf.value_noise(pw, 80.0, seed=6))[:, None] * hair[:, None]
        return np.clip(col, 0.0, 1.0)


# The hands --------------------------------------------------------------------------------------

FINGERS = (  # name, across the hand (+ toward the thumb), knuckle along it, phalanx lengths, radius, spread (deg)
    ("index", 0.0305, 0.098, (0.046, 0.027, 0.022), 0.0098, 4.0),
    ("middle", 0.0100, 0.102, (0.050, 0.031, 0.023), 0.0100, 0.0),
    ("ring", -0.0108, 0.099, (0.047, 0.029, 0.022), 0.0095, -3.5),
    ("little", -0.0300, 0.092, (0.037, 0.021, 0.019), 0.0084, -8.0),
)


class Hand:
    """A gloved hand hanging from the wrist in the rest pose: palm to the thigh, thumb forward,
    fingers in a relaxed curl. `glove` thickens it by the leather."""

    def __init__(self, side: str, bones, glove: float = 0.0025, curl: float = 1.0, missing: int = 0):
        t = bone_table(bones)
        self.s = s = 1.0 if side == "l" else -1.0
        self.wrist, tip = t[f"hand_{side}"]
        self.R = sdf.frame(tip - self.wrist, out_sign=s)  # u thumb side (forward), v back of the hand, w to the fingertips
        self.g = glove
        self.segments = []
        for name, across, knuckle, lengths, radius, spread in FINGERS[:len(FINGERS) - missing] if missing else FINGERS:
            pos = np.array((across, 0.0, knuckle))
            direction = np.array((math.sin(math.radians(spread)), 0.0, math.cos(math.radians(spread))))
            bend = 0.0
            r = radius + glove
            for i, (seg, flex) in enumerate(zip(lengths, (24.0, 38.0, 24.0))):
                bend += flex * curl
                rot = sdf.rotation("X", bend)  # Toward the palm (-v)
                dvec = rot @ direction
                end = pos + dvec * seg
                r_end = r * (0.92 if i < 2 else 0.8)
                self.segments.append((pos.copy(), end, r, r_end))
                pos, r = end, r_end * 1.01
        # The thumb: metacarpal from the wrist's thumb side, two phalanges, curled a little in.
        pos = np.array((0.02, -0.009, 0.022))
        dvec = np.array((0.3, -0.46, 0.84))
        dvec /= np.linalg.norm(dvec)
        for i, (seg, r0, r1) in enumerate(((0.046, 0.0125, 0.0115), (0.034, 0.0112, 0.0102), (0.027, 0.0102, 0.0085))):
            end = pos + dvec * seg
            self.segments.append((pos, end, r0 + glove, r1 + glove))
            pos = end
            dvec = sdf.rotation("X", 14.0) @ sdf.rotation("Z", -8.0) @ dvec
        g = glove
        self.palm = sdf.Loft((0, 0, 0), np.eye(3), [
            (-0.006, 0.0, 0.0, 0.029 + g, 0.029 + g, 0.016 + g, 0.018 + g, 2.2),
            (0.03, 0.003, 0.0, 0.036 + g, 0.037 + g, 0.0135 + g, 0.017 + g, 2.6),
            (0.07, 0.002, 0.0, 0.041 + g, 0.042 + g, 0.0125 + g, 0.016 + g, 3.0),
            (0.098, 0.0, 0.0, 0.041 + g, 0.041 + g, 0.0115 + g, 0.013 + g, 3.0)], round_ends=0.012)
        self.cuff = sdf.Loft((0, 0, 0), np.eye(3), [
            (-0.055, 0.0, 0.0, 0.034, 0.034, 0.033, 0.033, 2.0),
            (-0.01, 0.0, 0.0, 0.032 + g, 0.032 + g, 0.022 + g, 0.024 + g, 2.0),
            (0.01, 0.0, 0.0, 0.031 + g, 0.031 + g, 0.017 + g, 0.019 + g, 2.2)], round_ends=0.006)

    def local(self, pw):
        return (pw - self.wrist) @ self.R

    def __call__(self, pw):
        p = self.local(pw)
        d = smin(self.palm(p), self.cuff(p), 0.012)
        d = smin(d, ellipsoid(p, (0.019, -0.012, 0.04), (0.02, 0.013, 0.03)), 0.01)  # The thumb's ball
        d = smin(d, ellipsoid(p, (-0.03, -0.01, 0.052), (0.014, 0.011, 0.034)), 0.01)  # The palm's outer pad
        for a, b, r0, r1 in self.segments:
            d = smin(d, round_cone(p, a, b, r0, r1), 0.0045)
        for name, across, knuckle, *_ in FINGERS:  # Knuckles on the back
            d = smin(d, ellipsoid(p, (across, 0.006 + self.g, knuckle - 0.004), (0.009, 0.006, 0.009)), 0.008)
        return d

    def bounds(self):
        pts = self.wrist + (np.array([[a, b, c] for a in (-0.07, 0.09) for b in (-0.07, 0.06) for c in (-0.07, 0.21)]) @ self.R.T)
        return pts.min(0), pts.max(0)


# The boots ---------------------------------------------------------------------------------------

class Boot:
    """A boot on the rest-pose foot: the upper (a last shape along the foot, the shaft up to the
    boot top round the tucked trouser), the padded collar, toe cap and heel counter; and its sole
    with the welt, the heel and the tread. Styles: work (laced leather), climb (soft shoe with a
    rubber rand), hitop (canvas with a rubber toe), tall (the Breaker's knee boot)."""

    def __init__(self, side: str, spec: dict, suit: Suit, style: str):
        self.s = s = 1.0 if side == "l" else -1.0
        self.y0 = 0.1 * s
        self.style = style
        self.top = spec.get("boot_top", 0.2)
        self.sole_h = {"climb": 0.012, "tall": 0.04, "hitop": 0.022}.get(style, 0.028)
        lift = self.sole_h - 0.03
        L = suit.limb
        wide = 1.0 + (L - 1.0) * 0.4
        rows = [  # x, z centre, y offset, up, down, out, in, n
            (-0.075, 0.073, 0.000, 0.045, 0.043, 0.038, 0.037, 2.6),
            (-0.050, 0.078, 0.000, 0.052, 0.048, 0.044, 0.042, 2.6),
            (-0.015, 0.082, 0.000, 0.054, 0.052, 0.047, 0.044, 2.7),
            (0.025, 0.080, 0.002, 0.050, 0.050, 0.049, 0.045, 2.8),
            (0.070, 0.072, 0.004, 0.040, 0.042, 0.053, 0.047, 2.8),
            (0.120, 0.062, 0.006, 0.032, 0.032, 0.056, 0.049, 2.8),
            (0.170, 0.054, 0.004, 0.027, 0.024, 0.053, 0.046, 2.6),
            (0.210, 0.049, 0.002, 0.023, 0.019, 0.043, 0.037, 2.4),
            (0.236, 0.047, 0.000, 0.019, 0.017, 0.027, 0.023, 2.2),
        ]
        if style == "climb":  # A slipper-shaped shoe, low and close
            rows = [(x, cz - 0.004, cy, up * 0.9, dn * 0.95, o * 0.95, i * 0.95, n) for x, cz, cy, up, dn, o, i, n in rows]
        rows = [(x, cz + lift, cy, up, dn, o * wide, i * wide, n) for x, cz, cy, up, dn, o, i, n in rows]
        self.foot = sdf.Loft((0, self.y0, 0), sdf.frame((1, 0, 0), forward=(0, 0, 1), out_sign=s), rows, round_ends=0.015)
        # The shaft round the ankle and up to the boot top: slim under a bloused trouser, wide
        # enough for a tucked one in a tall boot.
        top = self.top
        tuck = style == "tall"
        r_top = (0.066 * min(L, 1.15) + 0.004) if tuck else 0.05 * min(L, 1.1)
        shaft = [(0.06 + lift, -0.02, 0.0, 0.05, 0.052, 0.044, 0.044, 2.2), (0.12, -0.018, 0.0, 0.046, 0.05, 0.042, 0.042, 2.0)]
        for z in np.linspace(0.17, top, max(2, int((top - 0.17) / 0.06) + 1)) if top > 0.17 else (top,):
            f = (z - 0.12) / max(top - 0.12, 1e-3)
            calf = 0.012 * math.sin(math.pi * min(1.0, (z - 0.12) / 0.3)) if tuck else 0.0
            r = 0.046 + (r_top - 0.046) * f + calf
            shaft.append((z, -0.018 + 0.006 * f, 0.0, r, r + 0.002, r * 0.95, r * 0.95, 2.0 + (0.5 if tuck else 0.0)))
        shaft = sorted({round(row[0], 4): row for row in shaft if row[0] <= top + 1e-6}.values())
        self.shaft = sdf.Loft((0, self.y0, 0), np.eye(3), shaft, round_ends=0.006)
        self.r_top = r_top

    def upper(self, p):
        x, y, z = p[:, 0], p[:, 1], p[:, 2]
        d = smin(self.foot(p), self.shaft(p), 0.02)
        top = self.top
        # The padded collar at the top.
        q = p - np.array((-0.01, self.y0, top - 0.004))
        rr = np.sqrt((q[:, 0] / 1.04) ** 2 + (q[:, 1] / 0.95) ** 2)
        d = smin(d, np.sqrt((rr - self.r_top + 0.002) ** 2 + q[:, 2] ** 2) - 0.0085, 0.006)
        yl = (y - self.y0) * self.s
        out = 0.0
        if self.style in ("work", "tall", "hitop"):
            # The toe cap and the heel counter: overlaid panels, their edges stitched.
            cap = smoothstep(0.148, 0.154, x - 0.55 * np.maximum(z - self.sole_h - 0.005, 0.0) - 0.3 * yl ** 2)
            heel = smoothstep(-0.035, -0.041, x + 0.35 * np.maximum(z - 0.05, 0.0)) * smoothstep(0.13, 0.12, z)
            out = 0.0016 * cap + 0.0014 * heel
            # The lacing gap down the instep, where the tongue shows.
            lace = smoothstep(0.016, 0.011, np.abs(yl)) * smoothstep(0.0, 0.02, x) * smoothstep(0.15, 0.13, x) * (z > 0.07)
            out = out - 0.0035 * lace
        # Creases across the vamp where the foot flexes.
        flex = smoothstep(0.08, 0.1, x) * smoothstep(0.15, 0.13, x) * smoothstep(0.06, 0.09, z)
        out = out + 0.0012 * flex * np.sin((x * 1.0 + 0.1 * yl) * 2 * math.pi / 0.012 + 3.0 * sdf.value_noise(p, 40, seed=2)) ** 2
        # Leather is never perfectly smooth.
        out = out + 0.0006 * (sdf.fbm(p, 40.0, 2, seed=13) - 0.5)
        return d - out

    def footprint(self, p):
        """The sole's outline: the upper's widest extent, plus the welt."""
        q = self.foot.local(p)
        P = self.foot.params(np.clip(q[:, 2], self.foot.s[0], self.foot.s[-1]))
        level = p.copy()
        level[:, 2] = P[:, 0]  # The section's centre height: its widest
        return self.foot(level) - (0.0045 if self.style in ("work", "tall") else 0.003)

    def sole(self, p):
        x, y, z = p[:, 0], p[:, 1], p[:, 2]
        h = self.sole_h
        d = smax(self.footprint(p), np.abs(z - h / 2) - h / 2, 0.003 if self.style != "climb" else 0.004)
        if self.style in ("work", "tall"):
            # The heel and the step up under the arch.
            arch = smoothstep(-0.03, -0.022, x) * smoothstep(0.075, 0.06, x)
            d = smax(d, -(z - 0.0) + 0.0075 * arch - 0.0001, 0.003)
        yl = (y - self.y0) * self.s
        if self.style == "climb":
            return d
        depth = 0.006
        if self.style == "hitop":  # Tire tread: blocks across the sole
            pitch = 0.016
            t = np.abs(((x + 0.3 * np.abs(yl)) / pitch) % 1.0 - 0.5)
            groove = np.maximum(z - depth, (t - 0.14) * pitch)
        else:  # Chevron lugs and a centre channel
            pitch = 0.021
            t = np.abs(((x + 1.4 * np.abs(yl)) / pitch) % 1.0 - 0.5)
            groove = np.minimum(np.maximum(z - depth, (t - 0.16) * pitch), np.maximum(z - depth, np.abs(yl) - 0.004))
        return smax(d, -groove, 0.0012)

    def bounds(self):
        return (-0.11, self.y0 - 0.08, -0.002), (0.26, self.y0 + 0.08, self.top + 0.02)
