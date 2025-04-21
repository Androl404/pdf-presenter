#!/bin/sh

set -xe

if [ "$1" = "clean" ]; then
    if test -e build/
    then
        rm -r build/
    fi
    return
fi

mkdir -p build

# Compiler options
COMPILER="gcc"
CFLAGS="$(pkg-config --cflags gtk4 poppler-glib)"
CLIBS="$(pkg-config --libs gtk4 poppler-glib)"
# CWARNINGS="-Wall -Wextra"
DEBUG="-g"

$COMPILER $DEBUG $CWARNINGS $CFLAGS -o ./build/pdf-presenter src/*.c $CLIBS
