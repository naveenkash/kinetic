# The browser client. The simulation in sim/ is KineticCore's rules on a grey-box world; it
# builds natively for its tests and with Emscripten for the browser.
#
#   make test     native tests of the browser simulation (any C++20 compiler)
#   make wasm     client/public/kinetic.js and kinetic.wasm (needs Emscripten: em++)
#   make client   wasm, then the client's production build in client/dist (needs npm)
#   make dev      wasm, then the client's dev server

ifeq ($(origin CXX),default)
CXX := clang++
endif
CORE := ../game/Source/KineticCore
TABLES := $(abspath ../game/Content/Core/Tables)
BUILD := build

WARN := -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wno-sign-conversion
CXXFLAGS := -std=c++20 -O2 $(WARN) -I$(CORE)/Public -Isim -DKINETIC_TABLES_DIR=\"$(TABLES)\"

CORE_SRC := $(wildcard $(CORE)/Private/*.cpp)
SIM_SRC := $(filter-out sim/Bindings.cpp,$(wildcard sim/*.cpp))
TEST_SRC := $(wildcard sim/Tests/*.cpp) ../game/Tests/KineticCore/TestMain.cpp
HEADERS := $(wildcard $(CORE)/Public/Kinetic/*.h) $(wildcard sim/*.h) $(wildcard ../game/Tests/KineticCore/*.h)

.PHONY: test wasm client dev clean

$(BUILD)/web-sim-tests: $(CORE_SRC) $(SIM_SRC) $(TEST_SRC) $(HEADERS) Makefile | $(BUILD)
	$(CXX) $(CXXFLAGS) -I../game/Tests/KineticCore $(CORE_SRC) $(SIM_SRC) $(TEST_SRC) -o $@

test: $(BUILD)/web-sim-tests
	./$<

client/public/kinetic.js: $(CORE_SRC) $(SIM_SRC) sim/Bindings.cpp $(HEADERS) $(wildcard $(TABLES)/*.csv) Makefile
	mkdir -p client/public
	em++ -std=c++20 -O2 -fexceptions $(WARN) -I$(CORE)/Public -Isim $(CORE_SRC) $(SIM_SRC) sim/Bindings.cpp \
	  -lembind -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createKinetic -sALLOW_MEMORY_GROWTH=1 \
	  -sENVIRONMENT=web,node --embed-file $(TABLES)@/tables -o $@

wasm: client/public/kinetic.js

client: wasm
	cd client && npm install && npm run build

dev: wasm
	cd client && npm install && npm run dev

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) client/public/kinetic.js client/public/kinetic.wasm client/dist
