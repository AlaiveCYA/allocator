#!/bin/bash

CURRENTPATH=$(pwd)
DESTDIR=$1

gcc -fPIC -o libmylloc.o -c "$CURRENTPATH"/src/mylloc.c
gcc -shared -o libmylloc.so libmylloc.o

if [ -z "$DESTDIR" ]; then
    DESTDIR=/usr
    sudo mv libmylloc.so "$DESTDIR"/lib
    sudo cp "$CURRENTPATH"/src/mylloc.h "$DESTDIR"/include
else
    mkdir -p "$DESTDIR"
    cd "$DESTDIR" || exit 1
    mkdir -p build
    cd build || exit 1
    mkdir -p lib
    mkdir -p include
    mv "$CURRENTPATH"/libmylloc.so "$DESTDIR"/build/lib
    cp "$CURRENTPATH"/src/mylloc.h "$DESTDIR"/build/include
fi

rm -f "$CURRENTPATH"/libmylloc.o











