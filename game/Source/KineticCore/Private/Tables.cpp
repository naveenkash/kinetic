#include "Kinetic/Tables.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

namespace Kinetic
{
namespace
{
// A minimal reader for the project's CSV tables: comma separated, first column "Name",
// no quoting (the tables never need it; a quote is rejected rather than misread).
class FCsvTable
{
public:
	FCsvTable(const std::filesystem::path& Directory, std::string InName) : Name(std::move(InName))
	{
		const std::filesystem::path Path = Directory / (Name + ".csv");
		std::ifstream File(Path);
		if (!File)
		{
			throw FTableError("cannot open " + Path.string());
		}
		std::string Line;
		bool bHeader = true;
		while (std::getline(File, Line))
		{
			if (!Line.empty() && Line.back() == '\r')
			{
				Line.pop_back();
			}
			if (Line.empty())
			{
				continue;
			}
			if (Line.find('"') != std::string::npos)
			{
				throw FTableError(Name + ": quoted fields are not supported");
			}
			const std::vector<std::string> Cells = Split(Line);
			if (bHeader)
			{
				for (size_t I = 0; I < Cells.size(); ++I)
				{
					Columns[Cells[I]] = I;
				}
				bHeader = false;
			}
			else
			{
				Rows[Cells.at(0)] = Cells;
			}
		}
	}

	double Number(const std::string& Row, const std::string& Column) const
	{
		const std::optional<double> Value = OptionalNumber(Row, Column);
		if (!Value)
		{
			throw FTableError(Where(Row, Column) + " is empty");
		}
		return *Value;
	}

	std::optional<double> OptionalNumber(const std::string& Row, const std::string& Column) const
	{
		const std::string& Cell = CellAt(Row, Column);
		if (Cell.empty())
		{
			return std::nullopt;
		}
		// strtod rather than from_chars: floating-point from_chars needs a newer macOS deployment target.
		char* End = nullptr;
		const double Value = std::strtod(Cell.c_str(), &End);
		if (End != Cell.c_str() + Cell.size() || !std::isfinite(Value))
		{
			throw FTableError(Where(Row, Column) + " is not a finite number: '" + Cell + "'");
		}
		return Value;
	}

	int Integer(const std::string& Row, const std::string& Column) const
	{
		const double Value = Number(Row, Column);
		if (Value < 0.0 || Value > 1e6 || Value != std::floor(Value))
		{
			throw FTableError(Where(Row, Column) + " must be a whole number from 0 to 1,000,000");
		}
		return static_cast<int>(Value);
	}

	bool Boolean(const std::string& Row, const std::string& Column) const
	{
		const std::string& Cell = CellAt(Row, Column);
		if (Cell == "true")
		{
			return true;
		}
		if (Cell == "false")
		{
			return false;
		}
		throw FTableError(Where(Row, Column) + " must be true or false");
	}

private:
	static std::vector<std::string> Split(const std::string& Line)
	{
		std::vector<std::string> Cells;
		std::stringstream Stream(Line);
		std::string Cell;
		while (std::getline(Stream, Cell, ','))
		{
			Cells.push_back(Cell);
		}
		if (!Line.empty() && Line.back() == ',')
		{
			Cells.emplace_back();
		}
		return Cells;
	}

	std::string Where(const std::string& Row, const std::string& Column) const
	{
		return Name + "[" + Row + "]." + Column;
	}

	const std::string& CellAt(const std::string& Row, const std::string& Column) const
	{
		const auto RowIt = Rows.find(Row);
		if (RowIt == Rows.end())
		{
			throw FTableError(Name + " has no row '" + Row + "'");
		}
		const auto ColumnIt = Columns.find(Column);
		if (ColumnIt == Columns.end())
		{
			throw FTableError(Name + " has no column '" + Column + "'");
		}
		if (ColumnIt->second >= RowIt->second.size())
		{
			throw FTableError(Where(Row, Column) + " is missing");
		}
		return RowIt->second[ColumnIt->second];
	}

	std::string Name;
	std::map<std::string, size_t> Columns;
	std::map<std::string, std::vector<std::string>> Rows;
};

FHarnessTuning ReadHarness(const FCsvTable& T, const std::string& Row)
{
	FHarnessTuning H;
	H.MassKg = T.Number(Row, "MassKg");
	H.RunSpeed = T.Number(Row, "RunSpeed");
	H.SprintSpeed = T.Number(Row, "SprintSpeed");
	H.JumpHeight = T.Number(Row, "JumpHeight");
	H.MantleHeight = T.Number(Row, "MantleHeight");
	H.AirControl = T.Number(Row, "AirControl");
	H.CrouchSpeed = T.Number(Row, "CrouchSpeed");
	H.bCanBrace = T.Boolean(Row, "bCanBrace");
	H.bCanPerch = T.Boolean(Row, "bCanPerch");
	H.FootHold = T.Number(Row, "FootHold");
	H.BracedFootHold = T.Number(Row, "BracedFootHold");
	H.LineLength = T.Number(Row, "LineLength");
	H.TensionN = T.Number(Row, "TensionN");
	H.OverspinTensionN = T.Number(Row, "OverspinTensionN");
	H.KickLaunchSpeedV = T.Number(Row, "KickLaunchSpeedV");
	H.HangLoadN = T.Number(Row, "HangLoadN");
	H.DogSpeed = T.Number(Row, "DogSpeed");
	H.ModeSwitch = T.Number(Row, "ModeSwitch");
	H.Recall = T.Number(Row, "Recall");
	H.SetLines = T.Integer(Row, "SetLines");
	H.SetLineLife = T.Number(Row, "SetLineLife");
	H.ActiveLines = T.Integer(Row, "ActiveLines");
	H.HangStaggerSeconds = T.Number(Row, "HangStaggerSeconds");
	H.CutterWarmupOffset = T.Number(Row, "CutterWarmupOffset");
	H.SmallPropSpeedMul = T.Number(Row, "SmallPropSpeedMul");
	H.AirborneFireSpeedKeep = T.Number(Row, "AirborneFireSpeedKeep");
	H.LadderSpeed = T.Number(Row, "LadderSpeed");
	return H;
}

FSpoolGlobals ReadGlobals(const FCsvTable& T)
{
	const auto V = [&T](const char* Row) { return T.Number(Row, "Value"); };
	FSpoolGlobals G;
	G.HaulRamp = V("HaulRamp");
	G.KickTime = V("KickTime");
	G.InterlockRamp = V("InterlockRamp");
	G.KickForceCapMul = V("KickForceCapMul");
	G.ReelRate = V("ReelRate");
	G.ReelHoldThreshold = V("ReelHoldThreshold");
	G.KinematicSeverSubsteps = T.Integer("KinematicSeverSubsteps", "Value");
	G.HardVelocityCap = V("HardVelocityCap");
	G.HaulCloseCap = V("HaulCloseCap");
	G.JackSeparateCap = V("JackSeparateCap");
	G.SnapImpulseBase = V("SnapImpulseBase");
	G.SnapReferenceForce = V("SnapReferenceForce");
	G.CutRecoilPer1000N = V("CutRecoilPer1000N");
	G.SmallPropBonusMassCap = V("SmallPropBonusMassCap");
	G.ManifoldJackTrigger = V("ManifoldJackTrigger");
	G.DogsMeet = V("DogsMeet");
	G.DogRadius = V("DogRadius");
	G.SlackTimeout = V("SlackTimeout");
	G.SlipDistance = V("SlipDistance");
	G.SlipInterval = V("SlipInterval");
	G.OverspinRamp = V("OverspinRamp");
	G.OverspinOctave = V("OverspinOctave");
	G.OverspinSnap = V("OverspinSnap");
	G.OverspinLaunch = V("OverspinLaunch");
	G.OverspinStagger = V("OverspinStagger");
	G.OverspinCooldown = V("OverspinCooldown");
	G.DoublerSplit = V("DoublerSplit");
	G.DoublerRestore = V("DoublerRestore");
	G.SetLineFade = V("SetLineFade");
	G.SetLineProximity = V("SetLineProximity");
	G.RatedCommitHold = V("RatedCommitHold");
	G.DogMissRecall = V("DogMissRecall");
	G.ForceVolumeReferenceMassKg = V("ForceVolumeReferenceMassKg");
	G.ForceVolumeCapMul = V("ForceVolumeCapMul");
	G.Gravity = V("Gravity");
	return G;
}

FThresholds ReadThresholds(const FCsvTable& T, const std::string& Row)
{
	return {T.OptionalNumber(Row, "Stagger"), T.OptionalNumber(Row, "Flip"), T.OptionalNumber(Row, "Stall")};
}

FVolatileTuning ReadVolatile(const FCsvTable& T, const std::string& Row)
{
	return {T.Number(Row, "TriggerSpeed"),  T.Number(Row, "TriggerJackN"),     T.Number(Row, "BlastRadius"),
	        T.Number(Row, "BlastImpulseNs"), T.Number(Row, "BlastStallRadius"), T.Number(Row, "BlastFlipRadius")};
}
} // namespace

std::string_view ToString(EHarnessClass Class)
{
	static constexpr std::array<std::string_view, 5> Names{"Climber", "Scrapper", "Rigger", "Doubler", "Breaker"};
	return Names[static_cast<size_t>(Class)];
}

FKineticTables FKineticTables::LoadFromDirectory(const std::filesystem::path& Directory)
{
	FKineticTables Tables;

	const FCsvTable Harness(Directory, "DT_HarnessClass");
	for (const EHarnessClass Class : AllHarnessClasses)
	{
		Tables.HarnessRows[static_cast<size_t>(Class)] = ReadHarness(Harness, std::string(ToString(Class)));
	}

	const FCsvTable Cutter(Directory, "DT_CutterVariant");
	const std::array<const char*, 3> CutterNames{"Standard", "Hot", "Long"};
	for (size_t I = 0; I < CutterNames.size(); ++I)
	{
		const char* Row = CutterNames[I];
		Tables.CutterRows[I] = {Cutter.Number(Row, "Warmup"), Cutter.Number(Row, "Swing"),
		                        Cutter.Number(Row, "Recovery"), Cutter.Number(Row, "Reach"),
		                        Cutter.Number(Row, "ConeDeg")};
	}

	const FCsvTable Impact(Directory, "DT_ImpactThresholds");
	const std::array<const char*, 5> ImpactNames{"PlayerVsStatic", "PlayerVsKinematic", "ObjectVsPlayer",
	                                             "PlayerVsPlayerFloor", "HangLineHit"};
	for (size_t I = 0; I < ImpactNames.size(); ++I)
	{
		Tables.ImpactRows[I] = ReadThresholds(Impact, ImpactNames[I]);
	}
	Tables.CrushRule = {Impact.Number("Crush", "Stall"), Impact.Number("Crush", "Seconds")};
	Tables.LedgeGrabMax = Impact.Number("LedgeGrabMax", "Stall");

	const FCsvTable Surface(Directory, "DT_SurfaceModifiers");
	const std::array<const char*, 5> SurfaceNames{"Hard", "Wood", "Scrap", "DeepWater", "StallGel"};
	for (size_t I = 0; I < SurfaceNames.size(); ++I)
	{
		Tables.SurfaceRows[I] = Surface.Number(SurfaceNames[I], "Multiplier");
	}

	const FCsvTable BreakFree(Directory, "DT_BreakFree");
	const std::array<const char*, 6> PlacementNames{"Loose", "Rack", "Stack", "Bolted", "Rated", "HangingHouse"};
	for (size_t I = 0; I < PlacementNames.size(); ++I)
	{
		Tables.BreakFreeRows[I] = BreakFree.Number(PlacementNames[I], "BreakFreeN");
	}

	const FCsvTable Volatiles(Directory, "DT_Volatiles");
	const std::array<const char*, 4> VolatileNames{"Propane", "FuelDrum", "DemoCharge", "GasManifold"};
	for (size_t I = 0; I < VolatileNames.size(); ++I)
	{
		Tables.VolatileRows[I] = ReadVolatile(Volatiles, VolatileNames[I]);
	}

	const FCsvTable Tiers(Directory, "DT_MassTiers");
	Tables.TierBounds = {Tiers.Number("Light", "UpperBoundKg"), Tiers.Number("Medium", "UpperBoundKg"),
	                     Tiers.Number("Heavy", "UpperBoundKg")};

	Tables.Spool = ReadGlobals(FCsvTable(Directory, "DT_SpoolGlobals"));
	return Tables;
}
} // namespace Kinetic
