# Builds and tests KineticCore without Unreal: any C++20 compiler (Apple clang on macOS).
# When Unreal 5.8 is set up, the same Public/Private sources build as the KineticCore module.
#
#   make test        unit tests
#   make test-asan   unit tests under AddressSanitizer and UndefinedBehaviorSanitizer
#   make bench       the panic-window bench
#   make check       the bible-table check and its tests
#   make all-checks  everything above

CXX ?= clang++
CORE := Source/KineticCore
TABLES := $(abspath Content/Core/Tables)
BUILD := build

CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wno-sign-conversion \
            -I$(CORE)/Public -DKINETIC_TABLES_DIR=\"$(TABLES)\"
SANITIZE := -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer

LIB_SRC := $(wildcard $(CORE)/Private/*.cpp)
TEST_SRC := $(wildcard $(CORE)/Tests/*.cpp)
HEADERS := $(wildcard $(CORE)/Public/Kinetic/*.h) $(wildcard $(CORE)/Tests/*.h)

.PHONY: test test-asan bench check all-checks clean

$(BUILD)/kinetic-core-tests: $(LIB_SRC) $(TEST_SRC) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -I$(CORE)/Tests $(LIB_SRC) $(TEST_SRC) -o $@

$(BUILD)/kinetic-core-tests-asan: $(LIB_SRC) $(TEST_SRC) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SANITIZE) -I$(CORE)/Tests $(LIB_SRC) $(TEST_SRC) -o $@

$(BUILD)/panic-bench: $(LIB_SRC) Tools/bench/PanicBench.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(LIB_SRC) Tools/bench/PanicBench.cpp -o $@

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

all-checks: check test test-asan bench

clean:
	rm -rf $(BUILD)
