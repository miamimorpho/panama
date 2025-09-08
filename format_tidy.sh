#!/bin/sh

find . \( -name "*.c" -o -name "*.h" \) -exec clang-format -i --style=file {} +
