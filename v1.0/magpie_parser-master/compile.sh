#!/bin/bash
g++ -lpthread ./src/synchronizer/inet_local.c ./src/analysis/processor.c ./src/analysis/generic.c ./src/analysis/math_func.c ./src/analysis/windowing.c $(find ./src -maxdepth 1 | grep "\.c$") -o ./parse -g
gcc ./src/synchronizer/inet_local.c ./src/synchronizer/main.c -o synchro
