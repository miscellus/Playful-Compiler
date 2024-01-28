#!/usr/bin/env bash

set -e

cc -std=c99 -Wall -Wextra -pedantic -O0 -ggdb calculator.c tokenizer.c parser.c -o calculator -lm

./calculator -print-infix -print-rpn -print-s '(1 + 2*(3 - 4^0))/7 - 5^2'
