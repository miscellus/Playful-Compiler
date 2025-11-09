#!/usr/bin/env bash

set -e

SRC=$(realpath src)
TESTS=$(realpath tests)
RUNNERS=$TESTS/runners

CFLAGS="
	-std=c99
	-Wall
	-Wextra
	-pedantic
	-O0
	-ggdb
	-DUNITY_INCLUDE_DOUBLE
	$TESTS/unity.c
"

[[ ! -d $RUNNERS ]] && mkdir $RUNNERS

cc $CFLAGS $SRC/tokenizer.c $TESTS/test_tokenizer.c -o $RUNNERS/test_tokenizer.test
cc $CFLAGS $SRC/tokenizer.c $SRC/parser.c $TESTS/test_parser.c -o $RUNNERS/test_parser.test -lm

$RUNNERS/test_tokenizer.test
$RUNNERS/test_parser.test