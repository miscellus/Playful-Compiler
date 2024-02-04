#!/usr/bin/env bash

set -e

find -type f -iname '*.[ch]' | xargs clang-format -i -style=file:.clang-format