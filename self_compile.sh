#!/bin/bash

set -eu

make rum

./rum src/rum.rum

clang++ `llvm-config --cxxflags --ldflags --system-libs --libs core` src/rum.rum.o build/helper.o  -o src/rum

./src/rum src/rum.rum

mv out.o src/out.o

clang++ `llvm-config --cxxflags --ldflags --system-libs --libs core` src/out.o build/helper.o  -o src/rum2
