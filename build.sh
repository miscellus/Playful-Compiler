#!/usr/bin/env bash

set -e

[[ ! -d build ]] && mkdir build


COMPILATION_UNITS="
../src/calculator.c
../src/tokenizer.c
../src/parser.c
"

CFLAGS="
-std=c99
-Wall
-Wextra
-pedantic
-O0
-ggdb
-I../src
"

LDFLAGS="
-lm
"

pushd build

set -x

cc $CFLAGS $COMPILATION_UNITS -o calculator $LDFLAGS

popd

bash test.sh

build/calculator -print-infix -print-rpn -print-s '(1 + 2*(3 - 4^0))/7 - 5^2'