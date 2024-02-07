#!/usr/bin/env bash

set -xe

find -type f -iname '*.[ch]' | xargs clang-format -i -style=file:.clang-format