#!/usr/bin/env bash

PLUGIN="$(dirname "$0")/../build/lib/libMyPass.so"

exec clang                           \
    -flto=full                       \
    -fuse-ld=lld                     \
    -fpass-plugin="$PLUGIN"          \
    -Wl,--load-pass-plugin="$PLUGIN" \
    "$@"
