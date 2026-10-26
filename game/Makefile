# Builds and tests KineticCore without Unreal: any C++20 compiler (Apple clang on macOS).
# The same Public/Private sources build as the KineticCore module in Kinetic.uproject; the tests
# live outside the module (Tests/KineticCore), since Unreal compiles every source in a module.
#
#   make test        unit tests
#   make test-asan   unit tests under AddressSanitizer and UndefinedBehaviorSanitizer
#   make bench       the panic-window bench
#   make check       the bible-table check and its tests
#   make report      plays a sample Scrap match and checks its report and raw events as the
#                    backend will read them
#   make all-checks  everything above
#
# With Unreal 5.8 installed (UE_ROOT, the Launcher's folder by default):
#   make editor        builds the KineticEditor target
#   make unreal-test   runs the Kinetic.* automation tests in a headless editor (UNREAL_TESTS=
#                      Kinetic.Yard.Scrap runs just those)
#   make import-props  imports the generated props in art/out (art/README.md) into Content
#   make yard-greybox  builds the Yard grey-box level from the layout docs, with the props
#   make import-audio  generates the tells' sound cues (art/audio) and imports them
#   make import-characters  imports the harness characters (art/characters, built with Blender)
#   make import-kit    imports the arena kit and its surfaces (art/kit, art/surfaces, built with Blender)
#   make import-port   imports the port round the Yard (art/port), after import-kit
#   make import-tagout makes Tag-Out's tell materials (dust, glint, haze), after import-port
#   make metahumans    creates and shapes the five class MetaHumans (building them needs a sign-in)
#   make export-metahumans  exports the built MetaHumans' bodies and faces for art/characters to fit
#   make retarget-metahumans  retargets the classes' takes onto their MetaHumans
#   make import-metahuman-gear  imports the gear art/characters fitted to each MetaHuman
#   make shots         renders views of the Yard (crews, mesas, the Gantry) to Saved/Kinetic/Shots
#
#   build/diff-recordings A B [tolerance_m]   compares two determinism recordings

ifeq ($(origin CXX),default)
CXX := clang++
endif
CORE := Source/KineticCore
TESTS := Tests/KineticCore
TABLES := $(abspath Content/Core/Tables)
BUILD := build

CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wno-sign-conversion \
            -I$(CORE)/Public -DKINETIC_TABLES_DIR=\"$(TABLES)\"
SANITIZE := -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer

LIB_SRC := $(wildcard $(CORE)/Private/*.cpp)
TEST_SRC := $(wildcard $(TESTS)/*.cpp)
HEADERS := $(wildcard $(CORE)/Public/Kinetic/*.h) $(wildcard $(TESTS)/*.h)

# Rebuild when the compiler, the flags (the tables path is one), or the source list change:
# the stamp's name is a hash of all three.
BUILD_KEY := $(shell printf '%s' "$(CXX) $(CXXFLAGS) $(SANITIZE) $(LIB_SRC) $(TEST_SRC)" | shasum | cut -c1-16)
STAMP := $(BUILD)/flags-$(BUILD_KEY).stamp
DEPS := $(HEADERS) $(STAMP) Makefile

UNREAL_TESTS ?= Kinetic

.PHONY: test test-asan bench check report all-checks clean editor unreal-test import-props yard-greybox import-audio import-characters shots metahumans export-metahumans retarget-metahumans assemble-metahumans import-metahuman-gear import-kit import-port import-tagout

$(BUILD)/kinetic-core-tests: $(LIB_SRC) $(TEST_SRC) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -I$(TESTS) $(LIB_SRC) $(TEST_SRC) -o $@

$(BUILD)/kinetic-core-tests-asan: $(LIB_SRC) $(TEST_SRC) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SANITIZE) -I$(TESTS) $(LIB_SRC) $(TEST_SRC) -o $@

$(BUILD)/panic-bench: $(LIB_SRC) Tools/bench/PanicBench.cpp $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(LIB_SRC) Tools/bench/PanicBench.cpp -o $@

$(BUILD)/sample-match: $(LIB_SRC) Tools/report/SampleMatch.cpp $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(LIB_SRC) Tools/report/SampleMatch.cpp -o $@

$(BUILD)/diff-recordings: $(LIB_SRC) Tools/determinism/DiffRecordings.cpp $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(LIB_SRC) Tools/determinism/DiffRecordings.cpp -o $@

$(STAMP): | $(BUILD)
	rm -f $(BUILD)/flags-*.stamp
	touch $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(BUILD)/kinetic-core-tests
	./$<

test-asan: $(BUILD)/kinetic-core-tests-asan
	./$<

bench: $(BUILD)/panic-bench
	./$<

check:
	python3 Tools/ci/check_tables.py
	python3 -m unittest discover -s Tools/ci -q

report: $(BUILD)/sample-match $(BUILD)/diff-recordings
	./$(BUILD)/sample-match $(BUILD)/sample-report.json $(BUILD)/sample-events.jsonl
	python3 Tools/ci/check_report.py $(BUILD)/sample-report.json $(BUILD)/sample-events.jsonl

all-checks: check test test-asan bench report

clean:
	rm -rf $(BUILD)

UE_ROOT ?= /Users/Shared/Epic Games/UE_5.8
UPROJECT := $(abspath Kinetic.uproject)
UE_CMD = "$(UE_ROOT)/Engine/Binaries/Mac/UnrealEditor-Cmd" "$(UPROJECT)" -unattended -nullrhi -nosplash -nopause -NoSound -nocrashreports

editor:
	"$(UE_ROOT)/Engine/Build/BatchFiles/Mac/Build.sh" KineticEditor Mac Development -Project="$(UPROJECT)" -WaitMutex

# The editor's exit code does not carry test failures, so the log is checked for them.
unreal-test: editor | $(BUILD)
	@rm -f $(BUILD)/unreal-test.log
	$(UE_CMD) -ExecCmds="Automation RunTests $(UNREAL_TESTS); Quit" -abslog="$(abspath $(BUILD))/unreal-test.log" > /dev/null 2>&1 || true
	@grep "Test Completed" $(BUILD)/unreal-test.log
	@grep -q "TEST COMPLETE. EXIT CODE: 0" $(BUILD)/unreal-test.log
	@! grep -q "Result={Fail" $(BUILD)/unreal-test.log

# Editor scripts run in a headless editor, not the pythonscript commandlet: the editor
# subsystems they use need GEditor. When unattended, a script ends the process itself with its
# status, once its work is saved (see run() in the scripts), and that status is the target's.
# The log is the editor's own file (-abslog).
define run_editor_script
	@rm -f $(BUILD)/$(2).log
	@$(UE_CMD) -ExecutePythonScript="$(abspath $(1))" -abslog="$(abspath $(BUILD))/$(2).log" > /dev/null 2>&1; \
	  status=$$?; \
	  grep -E "LogPython: (Display: )?Kinetic:" $(BUILD)/$(2).log; \
	  grep "LogPython: Error" $(BUILD)/$(2).log | head -40; \
	  exit $$status
endef

import-props: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_props.py,import-props)

yard-greybox: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/build_yard_greybox.py,yard-greybox)

UE_RENDER = "$(UE_ROOT)/Engine/Binaries/Mac/UnrealEditor-Cmd" "$(UPROJECT)" -unattended -nosplash -nopause -NoSound -nocrashreports -RenderOffscreen -ResX=1600 -ResY=900

shots: editor | $(BUILD)
	@rm -f $(BUILD)/shots.log
	$(UE_RENDER) -ExecCmds="Automation RunTests Kinetic.Shots; Quit" -abslog="$(abspath $(BUILD))/shots.log" > /dev/null 2>&1 || true
	@grep "Kinetic.Shots\|shot " $(BUILD)/shots.log | grep -v "Queued" | tail -8
	@ls Saved/Kinetic/Shots

import-kit: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_kit.py,import-kit)

import-port: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_port.py,import-port)

import-tagout: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_tagout.py,import-tagout)

metahumans: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/build_metahumans.py,metahumans)

# Re-assembles the MetaHumans (their hair, after a change to build_metahumans.py) from the rig and
# textures they already have; KINETIC_MH_CLASSES=Rigger,... for some of them.
assemble-metahumans: editor | $(BUILD)
	@rm -f $(BUILD)/assemble-metahumans.log
	@KINETIC_MH_BUILD=assemble $(UE_RENDER) -ExecutePythonScript="$(abspath Tools/unreal/build_metahumans.py)" -abslog="$(abspath $(BUILD))/assemble-metahumans.log" > /dev/null 2>&1; \
	  status=$$?; grep -E "LogPython: (Display: )?Kinetic:|LogPython: Error|Failed to build" $(BUILD)/assemble-metahumans.log; exit $$status

import-metahuman-gear: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_metahuman_gear.py,import-metahuman-gear)

retarget-metahumans: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/retarget_metahumans.py,retarget-metahumans)

# Exporting a skinned mesh needs a renderer: offscreen, not -nullrhi.
export-metahumans: editor | $(BUILD)
	@rm -f $(BUILD)/export-metahumans.log
	@$(UE_RENDER) -ExecutePythonScript="$(abspath Tools/unreal/export_metahumans.py)" -abslog="$(abspath $(BUILD))/export-metahumans.log" > /dev/null 2>&1; \
	  status=$$?; grep -E "LogPython: (Display: )?Kinetic:|LogPython: Error" $(BUILD)/export-metahumans.log; exit $$status

import-characters: editor | $(BUILD)
	$(call run_editor_script,Tools/unreal/import_characters.py,import-characters)

import-audio: editor | $(BUILD)
	python3 ../art/audio/build_audio.py
	$(call run_editor_script,Tools/unreal/import_audio.py,import-audio)
