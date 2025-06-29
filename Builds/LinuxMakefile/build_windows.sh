#!/bin/bash

# Windows Cross-Compilation Build Script for Cellyz Plugin
echo "Building Cellyz Plugin for Windows using MinGW cross-compiler..."

# Set MinGW cross-compiler tools
export CXX=x86_64-w64-mingw32-g++
export CC=x86_64-w64-mingw32-gcc
export AR=x86_64-w64-mingw32-ar
export STRIP=x86_64-w64-mingw32-strip
export PKG_CONFIG=x86_64-w64-mingw32-pkg-config

# Create a Windows-specific Makefile by modifying the Linux one
cp Makefile Makefile.windows

# Replace Linux-specific definitions with Windows ones
sed -i '' 's/-DLINUX=1/-DWIN32=1/g' Makefile.windows
sed -i '' 's/\.so/.dll/g' Makefile.windows
sed -i '' 's/-linux/-win64/g' Makefile.windows
sed -i '' 's/alsa//g' Makefile.windows
sed -i '' 's/gtk+-x11-3\.0//g' Makefile.windows
sed -i '' 's/webkit2gtk-4\.[01]//g' Makefile.windows
sed -i '' 's/-lrt -ldl -lpthread/-lwinmm -lws2_32 -lwininet/g' Makefile.windows

# Remove Linux-specific pkg-config calls
sed -i '' 's/\$(shell \$(PKG_CONFIG) --cflags.*freetype2 fontconfig libcurl[^)]*)//' Makefile.windows
sed -i '' 's/\$(shell \$(PKG_CONFIG) --libs.*freetype2 fontconfig libcurl[^)]*)//' Makefile.windows

# Build using the modified Makefile
echo "Starting Windows build..."
make -f Makefile.windows CONFIG=Release

echo "Windows build complete! Check build/ directory for output." 