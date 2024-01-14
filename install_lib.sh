#!/bin/bash

CURRENTPATH=$(pwd)
DESTDIR=$1

gcc -fPIC -o libmylloc.o -c "$CURRENTPATH"/src/mylloc.c
gcc -shared -o libmylloc.so libmylloc.o

if [ -z "$DESTDIR" ]; then
    DESTDIR=/usr/local
    mv libmylloc.so "$DESTDIR"/lib
    cp "$CURRENTPATH"/src/mylloc.h "$DESTDIR"/include
else
    mkdir -p "$DESTDIR"
    cd "$DESTDIR" || exit 1
    mkdir -p build
    cd build || exit 1
    mv "$CURRENTPATH"/libmylloc.so "$DESTDIR"/lib
    cp "$CURRENTPATH"/src/mylloc.h "$DESTDIR"/include
fi











