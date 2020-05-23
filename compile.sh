#!/bin/bash

set -eu

rum src/rum.rum

clang++ `llvm-config --cxxflags --ldflags --system-libs --libs core` src/rum.rum.o build/helper.o  -o src/rum
