#!/bin/sh

set -xe

if [ "$1" = "clean" ]; then
    rm -r build/
    return
fi

mkdir -p build/
gcc $(pkg-config --cflags gtk+-3.0 poppler-glib) -o ./build/main main.c $(pkg-config --libs gtk+-3.0 poppler-glib)

