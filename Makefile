# Makefile - builds Space Battle three ways:
#   make test     native test program for the battle rules (no raylib needed)
#   make desktop  native Windows game in bin/
#   make web      WebAssembly game in web/ (open it with Live Server)
# The VS Code tasks in .vscode/tasks.json just call these targets.
#
# Tool locations. Change these if you installed raylib or emsdk elsewhere,
# or override them on the command line: make desktop RAYLIB=D:/raylib
RAYLIB     ?= C:/raylib/raylib-5.5_win64_mingw-w64
RAYLIB_WEB ?= C:/raylib/raylib-5.5_webassembly
EMSDK      ?= C:/emsdk

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SOURCES  = $(wildcard src/*.cpp)

# The logic files are the only ones the tests compile: they never touch raylib.
LOGIC    = src/battle.cpp

.PHONY: test desktop web run shots

test:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(LOGIC) tests/test_battle.cpp -Isrc -o bin/test_battle.exe
	./bin/test_battle.exe

# -static bakes the C++ runtime into the .exe so it runs on any Windows PC.
# opengl32, gdi32 and winmm are the Windows libraries raylib draws and plays sound with.
desktop:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SOURCES) -I$(RAYLIB)/include -L$(RAYLIB)/lib \
		-lraylib -lopengl32 -lgdi32 -lwinmm -static -o bin/space-battle.exe

run: desktop
	./bin/space-battle.exe

# Saves a PNG of every screen into build/shots/, to compare with docs/design/.
shots:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(filter-out src/main.cpp,$(SOURCES)) tools/screenshots.cpp -Isrc 		-I$(RAYLIB)/include -L$(RAYLIB)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static -o bin/screenshots.exe
	./bin/screenshots.exe

# em++ is the Emscripten C++ compiler: same C++, but the output is WebAssembly.
#   -sUSE_GLFW=3          raylib's window layer in the browser
#   --preload-file assets packs the assets folder into index.data
#   --shell-file          our own minimal page instead of Emscripten's default one
EMCC = $(EMSDK)/upstream/emscripten/em++
EMENV = EMSDK=$(EMSDK) EMSDK_PYTHON=$(wildcard $(EMSDK)/python/*/python.exe) \
	EMSDK_NODE=$(wildcard $(EMSDK)/node/*/bin/node.exe)

web:
	$(EMCC) -std=c++17 -Os -Wall $(SOURCES) -DPLATFORM_WEB \
		-I$(RAYLIB_WEB)/include -L$(RAYLIB_WEB)/lib -lraylib \
		-sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 \
		--preload-file assets --shell-file src/shell.html -o web/index.html
