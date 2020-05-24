#!/bin/bash

set -eu

rum src/rum.rum

clang++ `llvm-config --cxxflags --ldflags --system-libs --libs core` out.o build/helper.o  -o src/rum

src/rum src/rum.rum

clang++ `llvm-config --cxxflags --ldflags --system-libs --libs core` out.o build/helper.o  -o src/rum2
