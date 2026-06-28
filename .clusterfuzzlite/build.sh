#!/bin/bash -eu

COMMON_FLAGS="$CXXFLAGS -std=c++17 -I include"

OBJECTS=""

for src in src/*.cpp; do
    obj="$(basename "${src%.cpp}").o"
    $CXX $COMMON_FLAGS -c "$src" -o "$obj"
    OBJECTS="$OBJECTS $obj"
done

for fuzz in regex dfa lexer unicode; do
    if [ -f "fuzz/${fuzz}_fuzzer.cc" ]; then
        $CXX $COMMON_FLAGS \
            $LIB_FUZZING_ENGINE \
            "fuzz/${fuzz}_fuzzer.cc" \
            $OBJECTS \
            -o "$OUT/${fuzz}_fuzzer"
    fi
done
