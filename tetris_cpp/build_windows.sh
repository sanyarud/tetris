#!/bin/bash
# Cross-compile Tetris for Windows (i686 32-bit) using MinGW
set -e

DEPS="win_deps"
SDL2="$DEPS/SDL2-2.30.12/i686-w64-mingw32"
SDL2_TTF="$DEPS/SDL2_ttf-2.22.0/i686-w64-mingw32"
SDL2_MIXER="$DEPS/SDL2_mixer-2.8.1/i686-w64-mingw32"

MINGW_DIR="/opt/homebrew/Cellar/mingw-w64/14.0.0/toolchain-i686/i686-w64-mingw32"
CXX=i686-w64-mingw32-g++

BUILD_DIR="build_win"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "=== Compiling Tetris for Windows (32-bit) ==="

# Force static linking of winpthread by pointing directly to the static lib
$CXX -std=c++17 -O2 -Wall \
    -I"$SDL2/include/SDL2" \
    -I"$SDL2_TTF/include/SDL2" \
    -I"$SDL2_MIXER/include/SDL2" \
    -o "$BUILD_DIR/tetris.exe" \
    main.cpp \
    -L"$SDL2/lib" \
    -L"$SDL2_TTF/lib" \
    -L"$SDL2_MIXER/lib" \
    -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer \
    -mwindows -static-libgcc -static-libstdc++ \
    "$MINGW_DIR/lib/libwinpthread.a"

echo "=== Copying SDL DLLs ==="
cp "$SDL2/bin/SDL2.dll" "$BUILD_DIR/"
cp "$SDL2_TTF/bin/"*.dll "$BUILD_DIR/"
cp "$SDL2_MIXER/bin/"*.dll "$BUILD_DIR/"

# Copy icon if exists
if [ -f icon_1024.png ]; then
    cp icon_1024.png "$BUILD_DIR/"
fi

echo "=== Checking dependencies ==="
i686-w64-mingw32-objdump -p "$BUILD_DIR/tetris.exe" | grep "DLL Name"

echo ""
echo "=== Done! ==="
echo "Build output in $BUILD_DIR/"
ls -la "$BUILD_DIR/"
