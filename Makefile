# Makefile - builds Space Battle three ways:
#   make test     native test program for the battle rules (no raylib needed)
#   make desktop  native Windows game in bin/  (make run builds and starts it)
#   make web      WebAssembly game in web/ (open it with Live Server)
#   make shots    a PNG of every screen in build/shots/, to compare with docs/design/
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

# The battle logic is the only game file the tests compile: it never touches raylib.
LOGIC    = src/battle.cpp

# -static bakes the C++ runtime into the .exe so it runs on any Windows PC.
# opengl32, gdi32 and winmm are the Windows libraries raylib draws and plays sound with.
DESKTOP_LIBS = -I$(RAYLIB)/include -L$(RAYLIB)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static

.PHONY: test desktop run web shots

test:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(LOGIC) tests/test_battle.cpp -Isrc -o bin/test_battle.exe
	./bin/test_battle.exe

desktop:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SOURCES) $(DESKTOP_LIBS) -o bin/space-battle.exe

run: desktop
	./bin/space-battle.exe

shots:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(filter-out src/main.cpp,$(SOURCES)) tools/screenshots.cpp -Isrc $(DESKTOP_LIBS) -o bin/screenshots.exe
	./bin/screenshots.exe

# em++ is the Emscripten C++ compiler: same C++, but the output is WebAssembly.
#   -sUSE_GLFW=3            raylib's window layer in the browser
#   -sEXPORTED_RUNTIME...   raylib's sound code reads HEAPF32; new Emscripten versions hide it unless asked
#   --preload-file assets   packs the assets folder into index.data
#   --shell-file            our own minimal page instead of Emscripten's default one
EMCC = $(EMSDK)/upstream/emscripten/em++

web:
	$(EMCC) -std=c++17 -Os -Wall $(SOURCES) -DPLATFORM_WEB \
		-I$(RAYLIB_WEB)/include -L$(RAYLIB_WEB)/lib -lraylib \
		-sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=HEAPF32 \
		--preload-file assets --shell-file src/shell.html -o web/index.html
