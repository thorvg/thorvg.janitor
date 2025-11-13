#!/bin/bash

TARGET=tvggame.html
SRC=tvggame.cpp

echo "🎮 Building ThorVG Janitor game..."
emcc $SRC \
  -o $TARGET \
  -std=c++20 \
  -O3 \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lthorvg \
  -sUSE_SDL=2 \
  -sUSE_WEBGL2=1 \
  -sFULL_ES3=1 \
  -sMAX_WEBGL_VERSION=2 \
  -sALLOW_MEMORY_GROWTH=1 \
  -sINITIAL_MEMORY=134217728 \
  -sMAXIMUM_MEMORY=536870912 \
  -sTOTAL_STACK=16777216 \
  -sSTACK_SIZE=16777216

ls -lrt $TARGET
