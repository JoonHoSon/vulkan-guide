#!/bin/bash

build_type=${1:-debug}

VENDOR_DIR="./vendor"
rm -rf build

#if [ -d "$VENDOR_DIR" ]; then
#    echo "Remove all subdirectories in vendor."
#    rm -rf "${VENDOR_DIR}/*"
#else
#    echo "Create vendor directory."
#    mkdir "$VENDOR_DIR"
#fi

# cp ./CMakeLists_vendor.txt "$VENDOR_DIR/CMakeLists.txt"

if [ "debug" == "$build_type" ]; then
    echo "-----------------------------------------------------------------------------"
    echo "Start project[Debug] initializing..."
    echo "-----------------------------------------------------------------------------"
    cmake -G Ninja -S . -DCMAKE_BUILD_TYPE=Debug -B build/debug
else
    echo "-----------------------------------------------------------------------------"
    echo "Start project[Release] initializing..."
    echo "-----------------------------------------------------------------------------"
    cmake -G Ninja -S . -DCMAKE_BUILD_TYPE=Release -B build/release
fi
