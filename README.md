![Build and test status](https://github.com/miscellus/Playful-Compiler/actions/workflows/run_tests.yaml/badge.svg)

# Playful Compiler
This is just a toy compiler that I am tinkering with. Feel free to play along. (^:

As of writing this is just a calculator program that parses and evaluates math expressions.

## Build and Run
```sh
# Bootstrap nob 
cc nob.c -o nob

# Build calculator
./nob build

# Run calculator with input as parameter
./build/calculator -input='1 + 2 * 3 + 4 ^ (2 - 1 * 2)'
8

# Run without command line arguments to see options.
./build/calculator
Usage: calculator [Options] <Expression>
Options:
  -print-infix             Print parenthesized expression with infix operators.
  -print-s                 Print parenthesized s-expression.
  -print-rpn               Print expression in reverse polish notation (RPN).
  -input=<expression>      Directly passed input

# Run tests
./nob test
```
