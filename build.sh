#!/usr/bin/env bash

set -e

[[ ! -d build ]] && mkdir build

BUILD=$(realpath build)
SRC=$(realpath src)


COMPILATION_UNITS="
$SRC/calculator.c
$SRC/tokenizer.c
$SRC/parser.c
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

set -x

cc $CFLAGS $COMPILATION_UNITS -o $BUILD/calculator $LDFLAGS

cat > $BUILD/input.txt <<EOF 
(
  1
 +
  2
  *(3 - 4^0))/7 
 -
 5^2
EOF
$BUILD/calculator -print-infix -print-rpn -print-s $BUILD/input.txt
