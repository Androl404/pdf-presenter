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

cd ./resources/
    glib-compile-resources --generate-source --target=resources.c --sourcedir=. resources.xml
    glib-compile-resources --generate-header --target=resources.h --sourcedir=. resources.xml
cd ./../

if [ -z ${WINDIR+x} ]; then
    echo "Not on MSYS2.";
else
    MSYS2_COMPILER_OPTIONS="-mwindows"
fi

# Compiler options
COMPILER="cc" # gcc
CFLAGS="$(pkg-config --cflags gtk4 poppler-glib)"
CLIBS="$(pkg-config --libs gtk4 poppler-glib)"
CWARNINGS="-Wall -Wextra -Wno-deprecated-declarations"
DEBUG="-g"

$COMPILER $DEBUG $CWARNINGS $CFLAGS -o ./build/pdf-presenter src/*.c resources/*.c $CLIBS $MSYS2_COMPILER_OPTIONS
