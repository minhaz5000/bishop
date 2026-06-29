#!/bin/bash -eu

COMMON_FLAGS="$CXXFLAGS -std=c++17 -I include"

OBJECTS=""
for src in src/*.cpp; do
    obj="$(basename "${src%.cpp}").o"
    $CXX $COMMON_FLAGS -c "$src" -o "$obj"
    OBJECTS="$OBJECTS $obj"
done

for fuzz in regex dfa lexer unicode rewriter splitter analyzer serializer optimizer token_stream cache; do
    if [ -f "fuzz/${fuzz}_fuzzer.cc" ]; then
        $CXX $COMMON_FLAGS \
            $LIB_FUZZING_ENGINE \
            "fuzz/${fuzz}_fuzzer.cc" \
            $OBJECTS \
            -o "$OUT/${fuzz}_fuzzer"
    fi
done

# Copy seed corpora that exist
for fuzz in regex dfa lexer unicode rewriter splitter analyzer serializer optimizer token_stream cache; do
    corpus_dir="fuzz/corpus_${fuzz}"
    if [ -d "$corpus_dir" ]; then
        zip -j "$OUT/${fuzz}_fuzzer_seed_corpus.zip" "$corpus_dir"/*
    fi
done
