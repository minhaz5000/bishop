#!/bin/bash -eu
$CXX $CXXFLAGS -std=c++17 -I include -c src/tokenizer.cpp -o tokenizer.o
$CXX $CXXFLAGS -std=c++17 -I include -c src/nfa.cpp        -o nfa.o
$CXX $CXXFLAGS -std=c++17 -I include $LIB_FUZZING_ENGINE \
    fuzz/regex_fuzzer.cc tokenizer.o nfa.o -o $OUT/regex_fuzzer

$CXX $CXXFLAGS -std=c++17 -I include $LIB_FUZZING_ENGINE \
    fuzz/dfa_fuzzer.cc tokenizer.o nfa.o dfa.o utils.o -o $OUT/dfa_fuzzer

$CXX $CXXFLAGS -std=c++17 -I include $LIB_FUZZING_ENGINE \
    fuzz/lexer_fuzzer.cc tokenizer.o nfa.o dfa.o utils.o debug.o serializer.o \
    -o $OUT/lexer_fuzzer
