#!/bin/bash

build_type=${1:-debug}

if [ "debug" == "$build_type" ]; then
    cmake --build ./build/debug --config Debug --target all
else
    cmake --build ./build/release --config Release --target all
fi
