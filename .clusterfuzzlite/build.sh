#!/bin/bash -eu
$CXX $CXXFLAGS -std=c++17 -I. -c tokenizer.cpp -o tokenizer.o
$CXX $CXXFLAGS -std=c++17 -I. -c nfa.cpp        -o nfa.o
$CXX $CXXFLAGS -std=c++17 -I. $LIB_FUZZING_ENGINE \
    fuzz/regex_fuzzer.cc tokenizer.o nfa.o -o $OUT/regex_fuzzer
