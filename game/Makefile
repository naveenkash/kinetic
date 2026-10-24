# Builds and tests KineticCore without Unreal: any C++20 compiler (Apple clang on macOS).
# When Unreal 5.8 is set up, the same Public/Private sources build as the KineticCore module.
#
#   make test        unit tests
#   make test-asan   unit tests under AddressSanitizer and UndefinedBehaviorSanitizer
#   make bench       the panic-window bench
#   make check       the bible-table check and its tests
#   make report      plays a sample Scrap match and checks its report and raw events as the
#                    backend will read them
#   make all-checks  everything above
#
#   build/diff-recordings A B [tolerance_m]   compares two determinism recordings

ifeq ($(origin CXX),default)
CXX := clang++
endif
CORE := Source/KineticCore
TABLES := $(abspath Content/Core/Tables)
BUILD := build

CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wno-sign-conversion \
            -I$(CORE)/Public -DKINETIC_TABLES_DIR=\"$(TABLES)\"
SANITIZE := -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer

LIB_SRC := $(wildcard $(CORE)/Private/*.cpp)
TEST_SRC := $(wildcard $(CORE)/Tests/*.cpp)
HEADERS := $(wildcard $(CORE)/Public/Kinetic/*.h) $(wildcard $(CORE)/Tests/*.h)

# Rebuild when the compiler, the flags (the tables path is one), or the source list change:
# the stamp's name is a hash of all three.
BUILD_KEY := $(shell printf '%s' "$(CXX) $(CXXFLAGS) $(SANITIZE) $(LIB_SRC) $(TEST_SRC)" | shasum | cut -c1-16)
STAMP := $(BUILD)/flags-$(BUILD_KEY).stamp
DEPS := $(HEADERS) $(STAMP) Makefile

.PHONY: test test-asan bench check report all-checks clean

$(BUILD)/kinetic-core-tests: $(LIB_SRC) $(TEST_SRC) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -I$(CORE)/Tests $(LIB_SRC) $(TEST_SRC) -o $@

$(BUILD)/kinetic-core-tests-asan: $(LIB_SRC) $(TEST_SRC) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SANITIZE) -I$(CORE)/Tests $(LIB_SRC) $(TEST_SRC) -o $@

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
