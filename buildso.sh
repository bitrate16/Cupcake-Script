#!/bin/bash

# Utility script for compiling Native modules for Cupcake.
# Use:
# bash buildso.sh src/MyModule.cpp output/MyModule.so

reset

g++ -static -w -g -c -fPIC -march=x86-64 -std=c++11 -fpermissive $1 -o lib_out.o
gcc -shared -o $2 lib_out.o -Wl,-y
#ld -shared lib_out.o -o $2
rm lib_out.o

# nm <file> = list dependencies