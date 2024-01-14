#!/bin/bash

DESTDIR=$1
if [ -z "$DESTDIR" ]; then
    echo "Usage: $0 <destination directory>"
    exit 1
fi


mkdir -p "$DESTDIR"
cd "$DESTDIR" || exit 1
mkdir -p build
cd build || exit 1

gcc -fPIC -o libmylloc.o -c ../src/mylloc.c
gcc -shared -o libmylloc.so libmylloc.o
ar -rcs libmymylloc.a libmylloc.o
rm -f libmylloc.o





