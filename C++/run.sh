#!/bin/bash

g++ -Iinclude -std=c++20 -O0 \
  -Wall -Wextra -Wpedantic \
  -Wshadow -Wconversion -Wsign-conversion \
  -Wundef -Wdouble-promotion \
  -fsanitize=address,undefined \
  -fno-omit-frame-pointer \
  src/*.cpp main.cpp -o baloot && MallocNanoZone=0 ./baloot