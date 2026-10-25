"""Builds the Yard grey-box level, /Game/Arenas/YRD_Yard/L_YRD_Greybox, from
docs/arenas/the-yard/layout.md and prop-manifest.md.

    make -C game yard-greybox

(a headless editor running this with -ExecutePythonScript, after make import-props)

The geometry is boxes in the layout's own frame: metres, X east, Y north, Z up, origin at the
centre of the centre trench at Floor level. Unreal is left-handed, so a layout point (x, y, z) is
placed at (100 x, -100 y, 100 z), the same flip the FBX import gives a Blender asset. North is
-Y in the editor, and the arena is not mirrored.

The props that exist (art/out, imported by import_props.py) are placed where the manifest puts
them. Objects with no asset yet are grey placeholders at their spec size, named PH_<object>, so
the routes and sightlines can be walked before they are built. Run it again to rebuild the
level; it replaces the whole level.
"""
from __future__ import annotations

import json
import math
import os
import random

import unreal

LEVEL = "/Game/Arenas/YRD_Yard/L_YRD_Greybox"
REPO = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
KIT = "/Game/Arenas/YRD_Yard/Kit"
MATERIALS = "/Game/Arenas/YRD_Yard/Greybox"
CUBE = "/Engine/BasicShapes/Cube"
LADDER = unreal.load_class(None, "/Script/KineticGameplay.KineticLadder")
PROP = unreal.load_class(None, "/Script/KineticGameplay.KineticProp")
DIRECTOR = unreal.load_class(None, "/Script/KineticGameplay.KineticYardDirector")

# Each generated prop's mass (its spec) and how the manifest places it.
PROP_BODIES = {
    "SM_Volatile_FuelDrum": (180.0, "RACK"),           # On the drip tray, racked (200 N)
    "SM_Volatile_PropaneCylinder": (45.0, "RACK"),     # In the rack
    "SM_Volatile_CylinderRack": (95.0, "RACK"),        # Racked (200 N)
    "SM_Medium_SteelCrate_HTR": (250.0, "LOOSE"),
    "SM_Light_Plank": (12.0, "LOOSE"),
    "SM_Light_ClampCrate": (28.0, "LOOSE"),
    "SM_Light_ChainCoil_HTR": (25.0, "LOOSE"),
    "SM_Light_CableReel_HTR": (28.0, "LOOSE"),
    "SM_Light_FoldingChair": (6.0, "LOOSE"),
    "SM_Light_ToolBag_Guild": (20.0, "LOOSE"),
    "SM_Light_PaintCan_Office": (5.0, "LOOSE"),
    "SM_Light_Bucket_Water": (10.0, "LOOSE"),
    "SM_Light_FloodlightStand_Office": (22.0, "LOOSE"),
    "SM_Heavy_Container20_HTR": (2000.0, "LOOSE"),     # Heavy: no single line moves it
}

# What blows (05 6.4; DT_Volatiles).
VOLATILES = {"SM_Volatile_FuelDrum": "FUEL_DRUM", "SM_Volatile_PropaneCylinder": "PROPANE"}

# Scenery a bot may Haul an enemy into (AKineticBotController): tagged Kinetic.Anchor.
ANCHOR_ROLES = {"mesa", "ledge", "structure", "placeholder", "rated"}

# Grey-box colours by role, so a tier reads at a glance.
COLOURS = {
    "floor": (0.32, 0.31, 0.30),
    "trench": (0.16, 0.15, 0.15),
    "pit": (0.10, 0.06, 0.05),
    "mesa": (0.45, 0.42, 0.38),
    "ledge": (0.55, 0.50, 0.42),
    "rated": (0.80, 0.62, 0.10),
    "bridge": (0.62, 0.48, 0.18),
    "rails": (0.30, 0.38, 0.45),
    "structure": (0.40, 0.40, 0.42),
    "ladder": (0.85, 0.75, 0.15),
    "placeholder": (0.55, 0.20, 0.55),
}

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assets = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
lib = unreal.MaterialEditingLibrary


def to_unreal(x: float, y: float, z: float) -> unreal.Vector:
    return unreal.Vector(x * 100.0, -y * 100.0, z * 100.0)


# The kit's surface for a box it does not cover (the press ram, the plank bridges), once imported.
ROLE_SURFACE = {"floor": "Concrete", "trench": "Slag", "pit": "Rust", "mesa": "HullSteel", "ledge": "HullSteel",
                "rated": "Hazard", "bridge": "Timber", "rails": "HullSteel", "structure": "DarkSteel", "ladder": "DarkSteel",
                "placeholder": "DarkSteel"}


def material(role: str):
    surface = f"/Game/Arenas/YRD_Yard/Kit/MI_Surface_{ROLE_SURFACE.get(role, 'DarkSteel')}"
    if eal.does_asset_exist(surface):
        return unreal.load_asset(surface)
    name = f"MI_Greybox_{role.capitalize()}"
    path = f"{MATERIALS}/{name}"
    if eal.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = assets.create_asset(name, MATERIALS, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
        lib.set_material_instance_parent(mi, unreal.load_asset("/Engine/BasicShapes/BasicShapeMaterial"))
    r, g, b = COLOURS[role]
    lib.set_material_instance_vector_parameter_value(mi, "Color", unreal.LinearColor(r, g, b, 1.0))
    eal.save_loaded_asset(mi)
    return mi


class Builder:
    """Spawns the boxes and props, and keeps every solid's extents so later pieces (the grate
    stairs) can be routed round what is already built."""

    def __init__(self) -> None:
        self.cube = unreal.load_asset(CUBE)
        self.materials = {role: material(role) for role in COLOURS}
        self.count = 0
        self.solids: list[tuple[str, tuple[float, ...]]] = []
        self.missing: list[str] = []
        # Every box and ladder as built, for the visual kit (art/kit builds detail over them).
        self.layout: dict = {"boxes": [], "ladders": []}
        self.actors_by_box: list = []

    def box(self, label: str, role: str, x0: float, x1: float, y0: float, y1: float, z0: float, z1: float,
            folder: str = "Greybox") -> unreal.StaticMeshActor:
        """An axis-aligned box from its extents in the layout frame."""
        x0, x1, y0, y1, z0, z1 = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1), min(z0, z1), max(z0, z1)
        if min(x1 - x0, y1 - y0, z1 - z0) <= 0:
            raise ValueError(f"{label}: an empty box {x0, x1, y0, y1, z0, z1}")
        centre = to_unreal((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2)
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, centre, unreal.Rotator(0, 0, 0))
        component = actor.static_mesh_component
        component.set_static_mesh(self.cube)
        component.set_material(0, self.materials[role])
        actor.set_actor_scale3d(unreal.Vector(x1 - x0, y1 - y0, z1 - z0))
        actor.set_actor_label(label)
        actor.set_folder_path(f"{folder}/{role.capitalize()}")
        if role in ANCHOR_ROLES:
            actor.set_editor_property("tags", [unreal.Name("Kinetic.Anchor")])
        self.count += 1
        self.solids.append((role, (x0, x1, y0, y1, z0, z1)))
        self.layout["boxes"].append({"label": label, "role": role, "min": [x0, y0, z0], "max": [x1, y1, z1]})
        self.actors_by_box.append(actor)
        return actor

    def ladder(self, label: str, x0: float, x1: float, y0: float, y1: float, z0: float, z1: float,
               into: tuple[float, float]) -> None:
        """A climbable ladder: the rung marker (no collision) and an AKineticLadder volume on
        the climbing side, 60 cm deep. `into` is the way into the wall, in the layout frame."""
        marker = self.box(label, "ladder", x0, x1, y0, y1, z0, z1)
        marker.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        ix, iy = into
        # Grow the volume 0.6 m out from the face, away from the wall.
        vx0, vx1, vy0, vy1 = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1)
        if ix > 0:
            vx0 -= 0.6
        elif ix < 0:
            vx1 += 0.6
        if iy > 0:
            vy0 -= 0.6
        elif iy < 0:
            vy1 += 0.6
        centre = to_unreal((vx0 + vx1) / 2, (vy0 + vy1) / 2, (z0 + z1) / 2)
        volume = actors.spawn_actor_from_class(LADDER, centre, unreal.Rotator(0, 0, 0))
        volume.get_component_by_class(unreal.BoxComponent).set_box_extent(
            unreal.Vector((vx1 - vx0) * 50.0, (vy1 - vy0) * 50.0, (z1 - z0) * 50.0))
        volume.set_editor_property("into_wall", unreal.Vector(ix, -iy, 0.0))
        volume.set_actor_label(f"{label}_Climb")
        volume.set_folder_path("Greybox/Ladders")
        self.layout["ladders"].append({"label": label, "min": [min(x0, x1), min(y0, y1), z0], "max": [max(x0, x1), max(y0, y1), z1],
                                       "into": [ix, iy]})
        # The foot stays clear, 1.5 m out and 2 m across, so a climber (and a bot's path) can
        # reach it: later pieces (the grate stairs) are routed round it.
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        if ix:
            face = x1 if ix > 0 else x0
            clear = (face - ix * 1.6, face - ix * 0.1, cy - 1.0, cy + 1.0)
        else:
            face = y1 if iy > 0 else y0
            clear = (cx - 1.0, cx + 1.0, face - iy * 1.6, face - iy * 0.1)
        self.solids.append(("clearance", (min(clear[0], clear[1]), max(clear[0], clear[1]),
                                          min(clear[2], clear[3]), max(clear[2], clear[3]), z0, z0 + 2.0)))

    def reserve(self, role: str, x0: float, x1: float, y0: float, y1: float, z0: float, z1: float) -> None:
        """Room taken by something built at play time (a kinematic), so nothing is built in it."""
        self.solids.append((role, (min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1), min(z0, z1), max(z0, z1))))

    def blocked(self, x0, x1, y0, y1, z0, z1, ignore=("floor", "trench", "ladder")) -> bool:
        """Whether a box would overlap anything built so far (beyond the floors), or the Haul
        corridor, which the layout keeps clear end to end."""
        x0, x1, y0, y1 = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1)
        if y0 < -9 and y1 > -15 and z0 < 2:
            return True
        for role, (a0, a1, b0, b1, c0, c1) in self.solids:
            if role in ignore:
                continue
            if x0 < a1 - 1e-6 and a0 < x1 - 1e-6 and y0 < b1 - 1e-6 and b0 < y1 - 1e-6 and z0 < c1 - 1e-6 and c0 < z1 - 1e-6:
                return True
        return False

    def volatile_box(self, label: str, kind: str, centre, size, mass: float, yaw_deg: float):
        """A volatile with no generated asset yet: a grey cube at its spec size, as a fixed
        AKineticProp (it does not move, it goes off)."""
        x, y, z = centre
        actor = actors.spawn_actor_from_class(PROP, to_unreal(x, y, z), unreal.Rotator(0, 0, -yaw_deg))
        mesh = actor.get_component_by_class(unreal.StaticMeshComponent)
        mesh.set_static_mesh(self.cube)
        mesh.set_material(0, self.materials["placeholder"])
        actor.set_actor_scale3d(unreal.Vector(*size))
        actor.set_editor_property("mass_kg", mass)
        actor.set_editor_property("fixed_scenery", True)
        actor.set_editor_property("volatile", getattr(unreal.KineticVolatile, kind))
        actor.set_actor_label(f"PH_{label}")
        actor.set_folder_path("Placeholders")
        self.count += 1
        self.solids.append(("placeholder", (x - size[0] / 2, x + size[0] / 2, y - size[1] / 2, y + size[1] / 2,
                                            z - size[2] / 2, z + size[2] / 2)))
        return actor

    def prop(self, name: str, label: str, x: float, y: float, z: float, yaw_deg: float = 0.0,
             pitch_deg: float = 0.0):
        """A generated prop at its pivot, as an AKineticProp with its spec's mass and the
        manifest's placement. Yaw is counter-clockwise from east, as in the layout."""
        mesh_path = prop_path(name)
        mesh = unreal.load_asset(mesh_path) if eal.does_asset_exist(mesh_path) else None
        if mesh is None:
            self.missing.append(name)
            return None
        # The layout's counter-clockwise yaw is clockwise in Unreal's flipped frame.
        actor = actors.spawn_actor_from_class(PROP, to_unreal(x, y, z),
                                              unreal.Rotator(roll=0.0, pitch=pitch_deg, yaw=-yaw_deg))
        actor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(mesh)
        mass, placement = PROP_BODIES[name]
        actor.set_editor_property("mass_kg", mass)
        actor.set_editor_property("placement", getattr(unreal.KineticPlacement, placement))
        if name in VOLATILES:
            actor.set_editor_property("volatile", getattr(unreal.KineticVolatile, VOLATILES[name]))
        actor.set_actor_label(label)
        actor.set_folder_path("Props")
        self.count += 1
        return actor


def prop_path(name: str) -> str:
    """Where import_props.py puts a generated mesh."""
    tier = name.split("_")[1]
    return f"/Game/Objects/{tier}/{name}/{name}"


def floor(b: Builder) -> None:
    # The base under everything: trench floors at -4, with the pit cut out of it.
    b.box("Base_W", "trench", -80, 38, -45, 45, -5, -4)
    b.box("Base_E", "trench", 52, 80, -45, 45, -5, -4)
    b.box("Base_PitS", "trench", 38, 52, -45, -7, -5, -4)
    b.box("Base_PitN", "trench", 38, 52, 7, 45, -5, -4)
    # The islands, solid from the trench floor to the Floor datum. Trenches are at Y -33..-27,
    # -3..3 and 27..33; the pit is X 38..52, Y -7..7.
    b.box("SouthDock", "floor", -80, 80, -45, -33, -4, 0)
    b.box("HaulLane_W", "floor", -80, 38, -27, -3, -4, 0)
    b.box("HaulLane_E", "floor", 52, 80, -27, -3, -4, 0)
    b.box("HaulLane_PitS", "floor", 38, 52, -27, -7, -4, 0)
    b.box("MarkerFlat_W", "floor", -80, 38, 3, 27, -4, 0)
    b.box("MarkerFlat_E", "floor", 52, 80, 3, 27, -4, 0)
    b.box("MarkerFlat_PitN", "floor", 38, 52, 7, 27, -4, 0)
    b.box("NorthDock", "floor", -80, 80, 33, 45, -4, 0)
    # The Well: walls from the trench base to -10, the pit floor, and the centre trench's two
    # rail stringers across it at -4.
    b.box("Pit_Floor", "pit", 38, 52, -7, 7, -11, -10)
    b.box("Pit_WallN", "pit", 38, 52, 7, 8, -10, -4)
    b.box("Pit_WallS", "pit", 38, 52, -8, -7, -10, -4)
    b.box("Pit_WallW", "pit", 37, 38, -7, 7, -10, -4)
    b.box("Pit_WallE", "pit", 52, 53, -7, 7, -10, -4)
    for y in (-0.75, 0.75):
        b.box("Pit_Stringer", "structure", 38, 52, y - 0.15, y + 0.15, -4.3, -4)
    # Perimeter walls on the islands, 4 m; the trench ends stay open for the Haulers.
    for name, y0, y1 in (("Wall_S", -45.5, -45), ("Wall_N", 45, 45.5)):
        b.box(name, "structure", -80, 80, y0, y1, 0, 4)
    for x0, x1 in ((-80.5, -80), (80, 80.5)):
        for ya, yb in ((-45, -33), (-27, -3), (3, 27), (33, 45)):
            b.box("Wall_End", "structure", x0, x1, ya, yb, 0, 4)
    # Welded rungs on each trench's north wall every 20 m (layout zone 1), as ladder markers.
    for yc in (-30, 0, 30):
        for x in range(-70, 80, 20):
            if 38 <= x <= 52 and yc == 0:
                continue
            b.ladder("Trench_Rungs", x - 0.3, x + 0.3, yc + 2.9, yc + 3.0, -4, 0, (0, 1))


def grate_plan(b: Builder, x: float, yc: float):
    """The grate at X x over the trench at Y yc: for each end, the landing and a stair flight
    of ten 0.35 m risers over 5 m. A flight runs straight away from the trench when the way is
    clear and turns along the lip, east or west, when it is not. None if no plan fits."""
    pieces = []
    for s in (-1, 1):
        landing = (x - 1.2, x + 1.2, yc + s * 4, yc + s * 5.5, 0, 3.5)
        if b.blocked(*landing):
            return None
        edge = yc + s * 5.5
        for axis, d, ox, oy in (("y", s, x, edge), ("x", 1, x + 1.2, yc + s * 4.75), ("x", -1, x - 1.2, yc + s * 4.75)):
            steps = []
            for i in range(9):  # The tenth riser is the Floor itself
                top = 3.5 - 0.35 * (i + 1)
                if axis == "y":
                    steps.append((x - 1.2, x + 1.2, oy + d * 0.5 * i, oy + d * 0.5 * (i + 1), 0, top))
                else:
                    steps.append((ox + d * 0.5 * i, ox + d * 0.5 * (i + 1), oy - 0.75, oy + 0.75, 0, top))
            if not any(b.blocked(*st) for st in steps):
                pieces.append((landing, steps))
                break
        else:
            return None
    return pieces


def crossings(b: Builder) -> None:
    """Grate footbridges (deck 3.5 m, 8 m span, 2.4 m wide, a 1.5 m landing and a stair at each
    end) and plank bridges (0 m, 6.5 m by 0.9 m). Built after the Stacks, the press and the
    placeholders, so the stairs can be routed round them.

    The layout's grate positions (X -50, -10, +30, +70) put some landings inside a mesa that
    stands on the lip (SC2 at the south trench's X -10, for one). Such a grate slides along the
    trench to the nearest X, 2 m at a time, where both ends fit; each shift is logged, and is
    PROPOSED for the layout doc."""
    for yc in (-30, 0, 30):
        for x0 in (-50, -10, 30, 70):
            for dx in [0] + [d * k for k in range(2, 18, 2) for d in (1, -1)]:
                if abs(x0 + dx) > 78:
                    continue
                plan = grate_plan(b, x0 + dx, yc)
                if plan is not None:
                    break
            else:
                raise RuntimeError(f"no clear place for the grate near X {x0} over the trench at Y {yc}")
            x = x0 + dx
            if dx:
                unreal.log(f"Kinetic: the grate at X {x0} over the trench at Y {yc} moves to X {x} (PROPOSED)")
            b.box("Grate_Deck", "bridge", x - 1.2, x + 1.2, yc - 4, yc + 4, 3.4, 3.5)
            for side in (-1, 1):
                b.box("Grate_Rail", "bridge", x + side * 1.2 - 0.05, x + side * 1.2 + 0.05, yc - 4, yc + 4, 3.5, 4.6)
            for landing, steps in plan:
                b.box("Grate_Landing", "bridge", *landing)
                for st in steps:
                    b.box("Grate_Step", "bridge", *st)
        for x in (-30, 10):
            plank = b.box("PlankBridge", "bridge", x - 0.45, x + 0.45, yc - 3.25, yc + 3.25, -0.05, 0)
            # The first train through the trench sweeps it; a Breaker landing on it breaks it.
            plank.set_editor_property("tags", [unreal.Name("Kinetic.PlankBridge")])


# Mesa footprints (x0, x1, y0, y1, top) and their stepped ledges (x0, x1, top), layout zones 7-11.
MESAS = {
    "WestMesa": ((-80, -56, -45, -33, 8), []),
    "EastMesa": ((56, 80, 33, 45, 8), []),
    "PickStack": ((-56, -44, 8, 20, 8), [(-44, -40, 2.7)]),
    "EastPickStack": ((56, 68, 8, 20, 5.4), []),
    "NC1": ((-36, -24, 14, 26, 8), [(-40, -36, 2.7), (-24, -20, 5.4)]),
    "NC2": ((-12, 0, 16, 26, 8), [(0, 4, 5.4)]),
    "NC3": ((14, 26, 6, 18, 8), [(10, 14, 5.4), (26, 30, 2.7)]),
    "SC1": ((-44, -32, -27, -15, 8), [(-32, -28, 5.4)]),
    "SC2": ((-20, -8, -27, -15, 8), [(-8, -4, 2.7)]),
    "SC3": ((4, 16, -9, -3, 2.7), []),
    "SC4": ((22, 34, -27, -15, 8), [(34, 38, 5.4)]),
}

# Which face of each mesa carries its rungs (the Stacks plan's R marks; layout zones 8 to 11).
RUNG_FACE = {"WestMesa": "E", "EastMesa": "W", "PickStack": "E", "EastPickStack": "E", "NC1": "W", "NC2": "W",
             "NC3": "W", "SC1": "W", "SC2": "W", "SC3": "W", "SC4": "W"}


def stacks(b: Builder) -> None:
    for name, ((x0, x1, y0, y1, top), ledges) in MESAS.items():
        b.box(name, "mesa", x0, x1, y0, y1, 0, top)
        for lx0, lx1, ltop in ledges:
            b.box(f"{name}_Ledge", "ledge", lx0, lx1, y0, y1, 0, ltop)
    # Rungs (ladder markers), from the Stacks plan's R marks: the main rung face, and on a
    # stepped end one rung per step, each from the level below, so a Breaker climbs 2.7, 5.4, 8.
    for name, ((x0, x1, y0, y1, top), ledges) in MESAS.items():
        face = RUNG_FACE[name]
        ym = (y0 + y1) / 2
        mesa_x = x0 if face == "W" else x1
        on_face = [l for l in ledges if (l[1] == x0 if face == "W" else l[0] == x1)]
        start = 0.0
        into = (1, 0) if face == "W" else (-1, 0)
        for lx0, lx1, ltop in on_face:
            outer = lx0 if face == "W" else lx1
            b.ladder(f"{name}_Rungs", outer - 0.05, outer + 0.05, ym - 0.3, ym + 0.3, start, ltop, into)
            start = ltop
        b.ladder(f"{name}_Rungs", mesa_x - 0.05, mesa_x + 0.05, ym - 0.3, ym + 0.3, start, top, into)
        for lx0, lx1, ltop in [l for l in ledges if l not in on_face]:
            west = lx0 < x0
            outer = lx0 if west else lx1
            inner = lx1 if west else lx0
            step_into = (1, 0) if west else (-1, 0)
            b.ladder(f"{name}_StepRungs", outer - 0.05, outer + 0.05, ym - 0.3, ym + 0.3, 0, ltop, step_into)
            b.ladder(f"{name}_StepRungs", inner - 0.05, inner + 0.05, ym + 1.0, ym + 1.6, ltop, top, step_into)
    # West Mesa's 1.2 m parapet on its north edge (PROPOSED in the layout).
    b.box("WestMesa_Parapet", "mesa", -80, -56, -33.3, -33, 8, 9.2)
    # The rated loads (the hull sections R1 to R4, the control shack on its two columns) move on
    # their commit timelines, so the Yard director builds them as kinematics; their room is
    # kept here so later pieces are routed round them.
    for x0, x1, y0, y1 in ((-76, -64, 13, 19), (30, 42, 19, 25), (-76, -64, -9, -3), (62, 74, -25, -19)):
        b.reserve("rated", x0, x1, y0, y1, 0, 2.7)
    for y in (35, 41):
        b.reserve("rated", -62.3, -61.7, y - 0.3, y + 0.3, 0, 10.3)
    b.reserve("structure", -66, -58, 33, 43, 10.3, 14)


def press(b: Builder) -> None:
    # The press frame over the west half of the pit, 12 m, legs at Y +-7.
    for x in (38.5, 44.5):
        for y in (-7.5, 7.5):
            b.box("Press_Leg", "structure", x - 0.5, x + 0.5, y - 0.5, y + 0.5, 0, 12)
    for y in (-7.5, 7.5):
        b.box("Press_Beam", "structure", 38, 45, y - 0.5, y + 0.5, 12, 13)
    b.box("Press_Crosshead", "structure", 38, 45, -7.5, 7.5, 12, 13)
    # The counterweight, rated, hanging at (45, +9) with its underside at 6 m, its cable from an
    # arm off the frame's north beam.
    b.box("Press_CounterweightArm", "structure", 44.5, 45.5, 8, 9.5, 12, 13)
    b.reserve("rated", 44, 46, 8, 10, 6, 9)  # The counterweight and its cable: the director's
    # The ram at the bottom of its stroke, a centimetre proud of the pit floor.
    ram = b.box("Press_Ram", "structure", 38.5, 51.5, -6.5, 6.5, -10.5, -9.99)
    # It cycles (KineticYardDirector): movable, and tagged for the director to find.
    ram.static_mesh_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    ram.set_editor_property("tags", [unreal.Name("Kinetic.PressRam")])
    # 1.1 m pipe rails on the north and south lips (PROPOSED); the east lip is open.
    for y in (-7.1, 7.1):
        b.box("PitLipRail", "structure", 38, 52, y - 0.03, y + 0.03, 1.04, 1.1)
        for x in range(38, 53, 2):
            b.box("PitLipRail_Post", "structure", x - 0.03, x + 0.03, y - 0.03, y + 0.03, 0, 1.1)


def rails(b: Builder) -> None:
    """The gantry's portals straddle the outer trenches, as the layout's references to the leg
    at (-60, -30) and the trench floor beneath it put them: a leg against each trench wall,
    from the trench floor to the cross-head under the rail walk, clear of the 3.2 m Hauler
    (28.4 to 31.6 m off the centre line) and of the mesas on the lips. Each cage ladder climbs
    the inner leg's east face from the lip, so it is taken from the Floor (36 m, 22.5 s for a
    Breaker), with landings at 24 m and 36 m (and 10.5 m on the shack's west leg). The leg head
    stands 2 m above the walk, beside it: the Rails' only cover."""
    for x in (-60, -20, 20, 60):
        for s in (-1, 1):
            b.box("PortalLeg", "rails", x - 1, x + 1, s * 27, s * 28.3, -4, 34.1)
            b.box("PortalLeg", "rails", x - 1, x + 1, s * 31.7, s * 33, -4, 34.1)
            b.box("Portal_Crosshead", "rails", x - 1, x + 1, s * 27, s * 34.2, 34.1, 35.6)
            b.box("PortalLeg_Head", "rails", x - 1, x + 1, s * 32.2, s * 34.2, 35.6, 38)
            b.ladder("PortalLeg_CageLadder", x + 1, x + 1.7, s * 27, s * 27.7, 0, 34.1, (-1, 0))
            for z in (24, 10.5) if (x == -60 and s > 0) else (24,):
                # The landing, with a hatch the climber passes through (0.9 m by 0.8 m).
                b.box("PortalLeg_Landing", "rails", x + 1.9, x + 2.2, s * 27, s * 28.3, z - 0.1, z)
                b.box("PortalLeg_Landing", "rails", x + 1, x + 1.9, s * 27.8, s * 28.3, z - 0.1, z)
    # Rail walks, 1.2 m wide at Y +-31.5, deck at 36, handrail outboard only, rail head inboard
    # on the cross-heads.
    for s in (-1, 1):
        yc = s * 31.5
        b.box("RailWalk", "rails", -80, 80, yc - 0.6, yc + 0.6, 35.9, 36)
        out = yc + s * 0.6
        b.box("RailWalk_Handrail", "rails", -80, 80, out - 0.03, out + 0.03, 36.94, 37)
        head = yc - s * 0.75
        b.box("GantryRail", "structure", -80, 80, head - 0.15, head + 0.15, 35.6, 36)
    # The Gantry (bridge beam, cab, magnet) is a kinematic the Yard director makes and moves.
    # Floodlight masts with their 24 m platforms.
    for x in (-52, 52):
        for y in (-40, 40):
            b.box("FloodlightMast", "structure", x - 0.4, x + 0.4, y - 0.4, y + 0.4, 0, 24)
            side = 1 if y < 0 else -1  # The ladder on the face toward the Yard
            # The platform, with a hatch over the ladder (1 m by 1.3 m) the climber passes through.
            hy0, hy1 = sorted((y + side * 0.4, y + side * 1.7))
            b.box("MastPlatform", "structure", x - 2, x - 0.5, y - 2, y + 2, 23.8, 24)
            b.box("MastPlatform", "structure", x + 0.5, x + 2, y - 2, y + 2, 23.8, 24)
            b.box("MastPlatform", "structure", x - 0.5, x + 0.5, y - 2, hy0, 23.8, 24)
            b.box("MastPlatform", "structure", x - 0.5, x + 0.5, hy1, y + 2, 23.8, 24)
            b.ladder("MastLadder", x - 0.35, x + 0.35, y + side * 0.4, y + side * 1.1, 0, 23.8, (0, -side))


def placeholders(b: Builder) -> None:
    """Objects with no generated asset yet, at their spec sizes, so the routes and cover can be
    walked. Each is named PH_<object>."""
    folder = "Placeholders"
    # Haul trucks: a 12 m by 3 m flatbed with its bed at 1.2 m, the cab against the end wall and
    # a 4 m ramp at 17 degrees on the inboard end, as six steps. The layout's (-75, -12) puts the
    # cab past the wall, so the truck stands inboard of it.
    for wall, d in ((-80, 1), (80, -1)):
        cab0 = wall
        bed0 = wall + d * 2.5
        bed1 = bed0 + d * 12
        b.box("PH_HaulTruck_Cab", "placeholder", cab0, bed0, -13.5, -10.5, 0, 3.2, folder)
        b.box("PH_HaulTruck_Bed", "placeholder", bed0, bed1, -13.5, -10.5, 0, 1.2, folder)
        for i in range(6):
            b.box("PH_HaulTruck_Ramp", "placeholder", bed1 + d * 0.64 * i, bed1 + d * 0.64 * (i + 1), -13.5, -10.5,
                  0, 1.2 * (6 - i) / 7, folder)
    b.box("PH_HaulSpool", "placeholder", -0.8, 0.8, -12.6, -11.4, 0, 1.6, folder)
    b.box("PH_ClaimMarker", "placeholder", -0.6, 0.6, 11.5, 12.5, 0, 0.55, folder)
    b.box("PH_ClaimMarker_Beacon", "placeholder", -0.05, 0.05, 11.95, 12.05, 0.55, 1.8, folder)
    for x, y in ((-60, -20), (-50, -20), (50, -20), (60, -20), (-28, -6), (30, -6)):
        b.box("PH_BilletStack", "placeholder", x - 1.2, x + 1.2, y - 0.6, y + 0.6, 0, 3.6, folder)
    for x in (-6, 6):
        b.box("PH_WeldingCart", "placeholder", x - 0.55, x + 0.55, -14.3, -13.7, 0, 1.5, folder)
    # The gas manifold: a volatile (a Jack over 600 N vents a 6 m jet toward the marker).
    b.volatile_box("GasManifold", "GAS_MANIFOLD", (20, 22, 0.8), (2.0, 0.6, 1.6), 400.0,
                   math.degrees(math.atan2(12 - 22, 0 - 20)))
    # The north leg's demolition charges (events-and-moving-parts 4): either sheds its ladder cage.
    for y in (28.45, 31.55):
        b.volatile_box("DemoCharge", "DEMO_CHARGE", (20, y, 3.0), (0.3, 0.2, 0.3), 5.0, 0.0)
    # Scrap heaps, 4 m across and 1.2 m high (South Dock, Marker Flat, North Dock), and the two
    # on the 5.4 m ledges of NC1 and SC4: cover and a x2.0 landing.
    heaps = [(-30, -40), (0, -40), (30, -40), (-10, 6), (8, 6), (10, 18), (-8, 20), (-40, 22), (-20, 40), (20, 40)]
    for x, y in heaps:
        # Two of the manifest's heaps fall inside a mesa or a ledge: they are piled against it.
        # The nearest clear spot, searched outward in rings of half a metre.
        candidates = [(x + 0.5 * i, y + 0.5 * j) for i in range(-16, 17) for j in range(-16, 17)]
        candidates.sort(key=lambda c: (c[0] - x) ** 2 + (c[1] - y) ** 2)
        nx, ny = next(c for c in candidates if not b.blocked(c[0] - 2, c[0] + 2, c[1] - 2, c[1] + 2, 0, 1.2))
        if (nx, ny) != (x, y):
            unreal.log(f"Kinetic: the scrap heap at ({x}, {y}) is in a mesa; piled against it at ({nx}, {ny})")
        b.box("PH_ScrapHeap", "placeholder", nx - 2, nx + 2, ny - 2, ny + 2, 0, 1.2, folder)
    for x, y, z in ((-22, 20, 5.4), (36, -21, 5.4)):
        b.box("PH_ScrapHeap", "placeholder", x - 1.5, x + 1.5, y - 1.5, y + 1.5, z, z + 0.8, folder)
    for x, y in ((40, -42), (0, 42)):
        b.box("PH_HandTruck", "placeholder", x - 0.3, x + 0.3, y - 0.35, y + 0.35, 0, 1.2, folder)
    for x, y in ((-20, 8), (6, 24), (40, 12)):
        b.box("PH_CartTrolley", "placeholder", x - 0.6, x + 0.6, y - 0.4, y + 0.4, 0, 0.9, folder)
    for x, y in ((-18, 22), (28, 24)):
        b.box("PH_BrickPallet", "placeholder", x - 0.6, x + 0.6, y - 0.5, y + 0.5, 0, 0.9, folder)
    b.box("PH_ScaffoldBoardStack", "placeholder", -31.2, -28.8, 7.8, 8.2 + 0.23 * 2, 0, 0.5, folder)
    b.box("PH_LaundryLine", "placeholder", 64, 68, 35.98, 36.02, 8, 10, folder)


def props(b: Builder) -> None:
    rng = random.Random(7)
    # South Dock: the fuel point at (+60, -42), two rows of three.
    for i in range(3):
        for j in range(2):
            b.prop("SM_Volatile_FuelDrum", "FuelDrum", 59.35 + 0.65 * i, -42.33 + 0.66 * j, 0, rng.uniform(0, 360))
    # Chain coils against the mast bases; cable reels on their rims along the south wall.
    for k, (x, y) in enumerate(((-51, -39), (-53, -41), (51, -39), (53, -41))):
        b.prop("SM_Light_ChainCoil_HTR", "ChainCoil", x, y, 0, 90 * k + 20)
    for x in (-20, 0, 20):
        b.prop("SM_Light_CableReel_HTR", "CableReel", x, -44, 0.3, 90)
    # The racks, four cylinders each: against SC2 with the front toward it (east), and by the
    # pit with the front toward the pit's north lip.
    for x, y, yaw in ((-24, -20, 0.0), (34, 10, math.degrees(math.atan2(7 - 10, 45 - 34)))):
        b.prop("SM_Volatile_CylinderRack", "CylinderRack", x, y, 0, yaw)
        c, s = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
        for sx, sy in ((-0.4, -0.2), (0.0, -0.2), (0.4, -0.2), (-0.4, 0.2)):
            b.prop("SM_Volatile_PropaneCylinder", "PropaneCylinder", x + sx * c - sy * s, y + sx * s + sy * c, 0.092,
                   rng.uniform(0, 360))
    for x in (-70.5, 70.5):
        for y in (-9.0, -15.0):
            b.prop("SM_Medium_SteelCrate_HTR", "SteelCrate", x, y, 0, 0)
    # Tool bags on the welding carts, at Truck A, at the West and East Mesa rung heads and at two
    # leg heads on the Rails.
    for x, y, z in ((-6, -14, 1.5), (6, -14, 1.5), (-67, -9.2, 0), (-57.5, -39, 8), (57.5, 38, 8), (-20, 31.5, 36),
                    (20, -31.5, 36)):
        b.prop("SM_Light_ToolBag_Guild", "ToolBag", x, y, z, rng.uniform(0, 360))
    for x, y, z in ((-38, -14.4, 0), (28, -14.4, 0), (60, 31.5, 36)):
        b.prop("SM_Light_ClampCrate", "ClampCrate", x, y, z, 0)
    # Marker Flat: planks within 10 m of the marker, clear of the heaps; buckets by the trolleys.
    placed = 0
    while placed < 6:
        a, r = rng.uniform(0, 2 * math.pi), rng.uniform(3, 10)
        x, y = r * math.cos(a), 12 + r * math.sin(a)
        if b.blocked(x - 1.2, x + 1.2, y - 1.2, y + 1.2, 0, 0.1):
            continue
        b.prop("SM_Light_Plank", "Plank", x, y, 0, rng.uniform(0, 180))
        placed += 1
    for x, y in ((-19, 9.0), (-21, 7.0), (7, 25.0), (41, 13.0)):
        b.prop("SM_Light_Bucket_Water", "Bucket", x, y, 0, rng.uniform(0, 360))
    # The Stacks: steel crates on NC2's top; paint cans on NC3, SC2 and East Mesa; the West Mesa
    # smoke corner (a reel lying flat as a table, four chairs); chairs under the shack.
    for x in (-9, -6, -3):
        b.prop("SM_Medium_SteelCrate_HTR", "SteelCrate", x, 17, 8, 0)
    for x, y, z in ((18, 9, 8), (18.4, 9.3, 8), (19, 8.8, 8), (-14, -24, 8), (-13.6, -24.3, 8), (-13.9, -23.6, 8),
                    (57.6, 39.5, 8), (57.9, 39.8, 8), (58.2, 39.4, 8)):
        b.prop("SM_Light_PaintCan_Office", "PaintCan", x, y, z, rng.uniform(0, 360))
    b.prop("SM_Light_CableReel_HTR", "CableReel_Table", -68, -36, 8.2, 0, pitch_deg=90)
    for k in range(4):
        a = math.pi / 2 * k + 0.4
        x, y = -68 + 1.0 * math.cos(a), -36 + 1.0 * math.sin(a)
        b.prop("SM_Light_FoldingChair", "FoldingChair", x, y, 8, math.degrees(a) + 180)  # Facing the reel
    for y in (37, 39):
        b.prop("SM_Light_FoldingChair", "FoldingChair", -62, y, 0, -90)  # Facing south
    for y in (35.8, 41.8):
        b.prop("SM_Light_CableReel_HTR", "CableReel", -61, y, 0.3, 0)
    # The Press: floodlight stands on the east lip corners aimed into the pit; spares on the masts.
    for y in (-6.4, 6.4):
        b.prop("SM_Light_FloodlightStand_Office", "FloodlightStand", 52.7, y, 0, 180)
    for x in (-52, 52):
        for y in (-40, 40):
            b.prop("SM_Light_FloodlightStand_Office", "FloodlightStand", x + 1.2, y + 1.2, 24, 0)
    containers(b)
    if b.missing:
        raise RuntimeError(f"not imported (make import-props): {sorted(set(b.missing))}")


def containers(b: Builder) -> None:
    """HTR containers in the Stacks, between the cut hull sections (shipping-container.md: the
    Yard's billet and parts containers). The layout gives no positions, so each stands at the
    clear spot nearest a point beside a rated hull section, end-on to it (PROPOSED)."""
    for label, (x, y) in (("R1", (-70, 22)), ("R2", (36, 15)), ("R3", (-70, 0)), ("R4", (68, -15))):
        candidates = [(x + 0.5 * i, y + 0.5 * j) for i in range(-12, 13) for j in range(-12, 13)]
        candidates.sort(key=lambda c: (c[0] - x) ** 2 + (c[1] - y) ** 2)
        spot = next((c for c in candidates if not b.blocked(c[0] - 3.1, c[0] + 3.1, c[1] - 1.3, c[1] + 1.3, 0, 2.6)), None)
        if spot is None:
            unreal.log_warning(f"Kinetic: no clear spot for the container by {label}")
            continue
        body = b.prop("SM_Heavy_Container20_HTR", f"Container_{label}", spot[0], spot[1], 0.0, 0.0)
        if body is None:
            continue
        b.solids.append(("placeholder", (spot[0] - 3.1, spot[0] + 3.1, spot[1] - 1.3, spot[1] + 1.3, 0, 2.6)))
        # The doors, shut, on the body's hinge sockets (they come with it if it moves).
        mesh = body.get_component_by_class(unreal.StaticMeshComponent)
        for side in ("L", "R"):
            door_name = f"SM_Heavy_Container20_HTR_Door_{side}"
            door_path = f"/Game/Objects/Heavy/SM_Heavy_Container20_HTR/{door_name}"  # With its body (import_props.py)
            door_mesh = unreal.load_asset(door_path) if eal.does_asset_exist(door_path) else None
            if door_mesh is None:
                b.missing.append(door_name)
                continue
            door = actors.spawn_actor_from_class(unreal.StaticMeshActor, body.get_actor_location(), unreal.Rotator(0, 0, 0))
            door.static_mesh_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
            door.static_mesh_component.set_static_mesh(door_mesh)
            door.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            door.attach_to_component(mesh, f"ATTACH_{door_name}", unreal.AttachmentRule.SNAP_TO_TARGET,
                                     unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.KEEP_WORLD, False)
            door.set_actor_label(f"Container_{label}_Door_{side}")
            door.set_folder_path("Props")


def game_mode() -> None:
    """The level plays the Kinetic game mode (KineticGameplay)."""
    mode = unreal.load_class(None, "/Script/KineticGameplay.KineticGameMode")
    if mode is None:
        raise RuntimeError("KineticGameMode is not built (make editor)")
    world = unreal.EditorLevelLibrary.get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", mode)


def emissive_material(name: str, unlit: bool) -> unreal.Material:
    """A glow: Color x Strength. Unlit for the sky dome (two-sided, no shadows), lit with
    emission for the lamps (KineticYardDirector recolours them)."""
    path = f"{MATERIALS}/{name}"
    if eal.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = assets.create_asset(name, MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    colour = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, 0)
    colour.set_editor_property("parameter_name", "Color")
    colour.set_editor_property("default_value", unreal.LinearColor(1.0, 0.55, 0.1, 1.0))
    strength = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 200)
    strength.set_editor_property("parameter_name", "Strength")
    strength.set_editor_property("default_value", 8.0)
    glow = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    if unlit:
        # The sodium cloud deck: dull orange at the horizon fading to near black overhead
        # (lighting-and-mood.md, Sky), by the height of the point on the dome.
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property("two_sided", True)
        mat.set_editor_property("is_sky", True)  # So the real-time sky light captures it
        zenith = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -900, -300)
        zenith.set_editor_property("parameter_name", "Zenith")
        zenith.set_editor_property("default_value", unreal.LinearColor(0.010, 0.006, 0.004, 1.0))
        world = lib.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -1300, -100)
        mask = lib.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -1100, -100)
        mask.set_editor_property("r", False)
        mask.set_editor_property("g", False)
        mask.set_editor_property("b", True)
        lib.connect_material_expressions(world, "", mask, "")
        scale = lib.create_material_expression(mat, unreal.MaterialExpressionDivide, -950, -100)
        lib.connect_material_expressions(mask, "", scale, "A")
        scale.set_editor_property("const_b", 40000.0)
        up = lib.create_material_expression(mat, unreal.MaterialExpressionSaturate, -800, -100)
        lib.connect_material_expressions(scale, "", up, "")
        mix = lib.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -500, -150)
        lib.connect_material_expressions(colour, "", mix, "A")
        lib.connect_material_expressions(zenith, "", mix, "B")
        lib.connect_material_expressions(up, "", mix, "Alpha")
        lib.connect_material_expressions(mix, "", glow, "A")
    else:
        lib.connect_material_expressions(colour, "", glow, "A")
        lib.connect_material_property(colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_expressions(strength, "", glow, "B")
    lib.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    errors = lib.recompile_material(mat)
    if errors:
        raise RuntimeError(f"{name} does not compile: {list(errors)}")
    eal.save_loaded_asset(mat)
    return mat


def spot(label: str, at, aim, kelvin: float, candela: float, cone: float, radius_m: float = 70.0,
         shadows: bool = True) -> None:
    """A flood: a shadowed spot at a layout point, aimed at another."""
    origin, target = to_unreal(*at), to_unreal(*aim)
    rotation = unreal.MathLibrary.find_look_at_rotation(origin, target)
    light = actors.spawn_actor_from_class(unreal.SpotLight, origin, rotation)
    c = light.spot_light_component
    c.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    c.set_editor_property("intensity", candela)
    c.set_editor_property("use_temperature", True)
    c.set_editor_property("temperature", kelvin)
    c.set_editor_property("outer_cone_angle", cone)
    c.set_editor_property("inner_cone_angle", cone * 0.6)
    c.set_editor_property("attenuation_radius", radius_m * 100.0)
    c.set_editor_property("cast_shadows", shadows)
    light.set_actor_label(label)
    light.set_folder_path("Lighting")


def lighting(b: Builder) -> None:
    """Night shift, 02:40, overcast (lighting-and-mood.md): no sun or moon; the key is the four
    corner masts' floods and the shack's roof floods, pooled; the fill is the sodium cloud deck
    (a sky dome the sky light captures) and the trench lamps (KineticYardDirector); thin fog on
    the Floor only. Exposure is fixed, so a live line stays the brightest thing in frame."""
    dome_mat = emissive_material("M_Yard_SkyDome", unlit=True)
    dome_mi = assets.create_asset("MI_Yard_SkyDome", MATERIALS, unreal.MaterialInstanceConstant,
                                  unreal.MaterialInstanceConstantFactoryNew()) if not eal.does_asset_exist(
        f"{MATERIALS}/MI_Yard_SkyDome") else unreal.load_asset(f"{MATERIALS}/MI_Yard_SkyDome")
    lib.set_material_instance_parent(dome_mi, dome_mat)
    lib.set_material_instance_vector_parameter_value(dome_mi, "Color", unreal.LinearColor(0.45, 0.09, 0.012, 1.0))
    lib.set_material_instance_scalar_parameter_value(dome_mi, "Strength", SKY_FILL)
    eal.save_loaded_asset(dome_mi)
    emissive_material("M_Yard_Lamp", unlit=False)
    dome = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    dome.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Sphere"))
    dome.static_mesh_component.set_material(0, dome_mi)
    # Not solid: the whole Yard is inside it, and a solid sphere pushes every player out to 800 m.
    dome.static_mesh_component.set_collision_profile_name("NoCollision")
    dome.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)
    dome.static_mesh_component.set_editor_property("cast_shadow", False)
    dome.set_actor_scale3d(unreal.Vector(1600, 1600, 1600))  # 800 m radius
    dome.set_actor_label("Sky_SodiumDome")
    dome.set_folder_path("Lighting")
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 3000), unreal.Rotator(0, 0, 0))
    sky.light_component.set_editor_property("real_time_capture", True)
    sky.light_component.set_editor_property("intensity", SKY_LIGHT)  # The fill, from the dim dome
    sky.set_folder_path("Lighting")
    fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    f = fog.component
    f.set_editor_property("fog_density", 0.02)
    f.set_editor_property("fog_height_falloff", 1.2)  # Thin, at the Floor
    f.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.035, 0.014, 0.006, 1.0))
    fog.set_folder_path("Lighting")
    # The corner masts: three 30-degree pools each, inboard and down.
    for x in (-52, 52):
        for y in (-40, 40):
            for k, (tx, ty) in enumerate(((x * 0.6, y * 0.72), (x * 0.86, y * 0.2), (x * 0.3, y * 0.55))):
                spot(f"Flood_Mast_{x}_{y}_{k}", (x, y, 24.6), (tx, ty, 0), 5600, FLOOD_CD, 17.0, 75.0)
    # The shack's roof floods over the west third of the Rails and Marker Flat.
    for k, (tx, ty) in enumerate(((-40, 20), (-30, 8), (-55, 12), (-20, 30))):
        spot(f"Flood_Shack_{k}", (-62 + (k - 1.5) * 2.0, 33.2, 14.3), (tx, ty, 0), 5600, 45000, 20.0, 60.0)
    # The spawn mesas' cage lamps over the Drop points (PROPOSED: the crews drop there in the dark).
    for label, (x, y) in (("A", (-68, -39)), ("B", (68, 39))):
        spot(f"CageLamp_Spawn{label}", (x, y, 14.0), (x, y, 8.0), 4000, FLOOD_CD * 0.015, 45.0, 20.0)
    # The press: frame work lights into the pit, and the east lip's stands.
    for k, (x, y) in enumerate(((38.5, -7.5), (38.5, 7.5), (44.5, -7.5), (44.5, 7.5))):
        spot(f"WorkLight_Press_{k}", (x, y, 11.5), (45, 0, -10), 4000, 25000, 30.0, 30.0)
    for y in (-6.4, 6.4):
        spot(f"Flood_PitStand_{y}", (52.7, y, 1.9), (44, 0, -8), 5600, 30000, 22.0, 30.0)
    # The fixed exposure: the floods' pools read, the Floor between them is dark (map rule 8).
    pp = actors.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    pp.set_editor_property("unbound", True)
    settings = pp.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_method", True)
    settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
    settings.set_editor_property("override_auto_exposure_bias", True)
    settings.set_editor_property("auto_exposure_bias", YARD_EXPOSURE)
    settings.set_editor_property("override_auto_exposure_apply_physical_camera_exposure", True)
    settings.set_editor_property("auto_exposure_apply_physical_camera_exposure", False)
    pp.set_editor_property("settings", settings)
    pp.set_actor_label("PostProcess_Yard")
    pp.set_folder_path("Lighting")
    # Spawns (layout: Spawn A at (-68, -39, 8), Spawn B at (+68, +39, 8)), facing the Yard.
    for label, (x, y, z), yaw in (("Spawn_A", (-68, -39, 8), 30), ("Spawn_B", (68, 39, 8), 210)):
        start = actors.spawn_actor_from_class(unreal.PlayerStart, to_unreal(x, y, z + 1.0),
                                              unreal.Rotator(roll=0, pitch=0, yaw=-yaw))
        start.set_actor_label(label)
        start.set_editor_property("player_start_tag", label)


YARD_EXPOSURE = -1.5  # EV: manual exposure is x8 at 0 EV; tuned on make shots
SKY_FILL = 1.0        # The cloud deck's own glow, seen
SKY_LIGHT = 20.0      # The fill the sky light carries from it
FLOOD_CD = 40000.0    # A mast flood's centre intensity


def kit(b: Builder) -> None:
    """The visual kit (art/kit, make import-kit): detailed meshes over the grey boxes, in the
    layout's frame. Where the kit covers a box, the box stays for collision and the navmesh but
    is not drawn."""
    meshes = [a for a in eal.list_assets(KIT, recursive=False) if a.split("/")[-1].split(".")[0].startswith("SM_Yard_")] \
        if eal.does_directory_exist(KIT) else []
    if not meshes:
        unreal.log("Kinetic: no Yard kit imported (make import-kit): the grey boxes are drawn")
        return
    for path in meshes:
        mesh = unreal.load_asset(path.split(".")[0])
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        actor.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)
        actor.set_actor_label(mesh.get_name())
        actor.set_folder_path("Kit")
    covered = json.load(open(os.path.join(REPO, "art", "kit", "covered.json"))) if os.path.exists(
        os.path.join(REPO, "art", "kit", "covered.json")) else []
    hidden = 0
    for index, actor in enumerate(b.actors_by_box):
        if index in covered:
            actor.static_mesh_component.set_editor_property("visible", False)
            actor.static_mesh_component.set_editor_property("cast_shadow", False)
            hidden += 1
    unreal.log(f"Kinetic: Yard kit: {len(meshes)} meshes, {hidden} grey boxes covered")


def navigation() -> None:
    """The navmesh's bounds: the whole arena, the Well's floor to above the gantry. The navmesh
    itself is built when play starts (Config/DefaultEngine.ini: dynamic generation); the ladders
    link its levels."""
    x0, x1, y0, y1, z0, z1 = -81, 81, -46, 46, -12, 40
    volume = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume,
                                           to_unreal((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
                                           unreal.Rotator(0, 0, 0))
    # The default brush is a 2 m cube.
    volume.set_actor_scale3d(unreal.Vector((x1 - x0) / 2, (y1 - y0) / 2, (z1 - z0) / 2))
    volume.set_actor_label("Nav_Bounds")


def main() -> None:
    # The level is rebuilt whole: an existing one is opened and emptied (it may be the map the
    # editor opened on, which cannot be deleted while loaded), a missing one created.
    if eal.does_asset_exist(LEVEL):
        if not levels.load_level(LEVEL):
            raise RuntimeError(f"could not open {LEVEL}")
        # Everything but the world settings and the builder brush (a volume is a brush too).
        doomed = [a for a in actors.get_all_level_actors()
                  if not isinstance(a, unreal.WorldSettings)
                  and not (isinstance(a, unreal.Brush) and not isinstance(a, unreal.Volume))]
        actors.destroy_actors(doomed)
    elif not levels.new_level(LEVEL):
        raise RuntimeError(f"could not create {LEVEL}")
    b = Builder()
    floor(b)
    stacks(b)
    press(b)
    rails(b)
    placeholders(b)
    crossings(b)
    props(b)
    lighting(b)
    navigation()
    director = actors.spawn_actor_from_class(DIRECTOR, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    director.set_actor_label("Yard_Director")
    # The layout for the visual kit, and the kit itself if it has been built and imported.
    import json
    for path in (os.path.join(REPO, "art", "kit", "yard_layout.json"),):
        with open(path, "w") as f:
            json.dump(b.layout, f, indent=1)
    kit(b)
    game_mode()
    if not levels.save_current_level():
        raise RuntimeError(f"could not save {LEVEL}")
    unreal.log(f"Kinetic: built {LEVEL}, {b.count} actors")


def run() -> None:
    """Runs main(). In an unattended editor (make), it then ends the process with main()'s
    status, 0 or 1: everything is saved by then, and the 5.8.4 editor's own shutdown, after a
    script has shown its import notifications, crashes in Slate's teardown on macOS and opens
    the crash reporter."""
    unattended = "-unattended" in unreal.SystemLibrary.get_command_line().lower()
    status = 0
    try:
        main()
    except Exception:
        import traceback
        for line in traceback.format_exc().splitlines():
            unreal.log_error(line)
        status = 1
    if unattended:
        unreal.SystemLibrary.execute_console_command(None, "FLUSHLOG")
        os._exit(status)


run()
