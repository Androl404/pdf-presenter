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
    glib-compile-resources --generate-source --target=resources.c --sourcedir=. --generate-source resources.xml
    glib-compile-resources --generate-header --target=resources.h --sourcedir=. --generate-source resources.xml
cd ./../

# Compiler options
COMPILER="cc" # gcc
CFLAGS="$(pkg-config --cflags gtk4 poppler-glib)"
CLIBS="$(pkg-config --libs gtk4 poppler-glib)"
CWARNINGS="-Wall -Wextra -Wno-deprecated-declarations"
DEBUG="-g"

$COMPILER $DEBUG $CWARNINGS $CFLAGS -o ./build/pdf-presenter src/*.c resources/*.c $CLIBS
