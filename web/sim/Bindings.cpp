// The browser's entry point (Emscripten embind): one Game wraps a session. The data tables
// are embedded in the module at /tables. Buttons are a bitmask so one call carries a tick.
#include "Session.h"

#include <emscripten/bind.h>

#include <memory>

namespace
{
using namespace Kinetic;
using namespace Kinetic::Web;

enum EButton : int
{
	Jump = 1 << 0,
	Sprint = 1 << 1,
	Brace = 1 << 2,
	Fire = 1 << 3,
	SelfDog = 1 << 4,
	Release = 1 << 5,
	Draw = 1 << 6,
	SwingButton = 1 << 7,
};

class FGame
{
public:
	// bPlay false: a bot takes slot 0 too (the demo, watched from behind slot 0).
	FGame(int Seed, int PlayerClass, bool bPlay)
	    : Tables(FKineticTables::LoadFromDirectory("/tables")),
	      Session(std::make_unique<FSession>(Tables, static_cast<uint64_t>(Seed), ClassOf(PlayerClass), bPlay))
	{
	}

	void Step(double Forward, double Right, double Yaw, double Pitch, int Buttons, int Mode)
	{
		FPlayerInput In;
		In.MoveForward = Forward;
		In.MoveRight = Right;
		In.Yaw = Yaw;
		In.Pitch = Pitch;
		In.bJump = (Buttons & Jump) != 0;
		In.bSprint = (Buttons & Sprint) != 0;
		In.bBrace = (Buttons & Brace) != 0;
		In.bFire = (Buttons & Fire) != 0;
		In.bSelfDog = (Buttons & SelfDog) != 0;
		In.bRelease = (Buttons & Release) != 0;
		In.bDraw = (Buttons & Draw) != 0;
		In.bSwing = (Buttons & SwingButton) != 0;
		In.Mode = Mode;
		Session->Step(In);
	}

	std::string Snapshot() const { return Session->SnapshotJson(); }
	std::string Arena() const { return Session->ArenaJson(); }

private:
	static EHarnessClass ClassOf(int Index)
	{
		return Index >= 0 && Index < 5 ? static_cast<EHarnessClass>(Index) : EHarnessClass::Rigger;
	}

	FKineticTables Tables;
	std::unique_ptr<FSession> Session;
};
} // namespace

EMSCRIPTEN_BINDINGS(kinetic)
{
	emscripten::class_<FGame>("Game")
	    .constructor<int, int, bool>()
	    .function("step", &FGame::Step)
	    .function("snapshot", &FGame::Snapshot)
	    .function("arena", &FGame::Arena);
}
