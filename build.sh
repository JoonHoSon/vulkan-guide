#!/bin/bash

build_type=${1:-debug}

if [ "debug" == "$build_type" ]; then
    cmake --build ./build/debug
else
    cmake --build ./build/release
fi
