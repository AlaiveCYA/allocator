#!/bin/bash

DIR="$(find . -type d -name "build")"

cd "$DIR" || exit 1
cd .. || exit 1

rm -rf build
rm -rf test/e2e_correct
rm -rf test/e2e_fault