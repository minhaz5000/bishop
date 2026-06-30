#!/bin/bash -eu

COMMON_FLAGS="$CXXFLAGS -std=c++17 -I include"

OBJECTS=""
for src in src/*.cpp; do
    obj="$(basename "${src%.cpp}").o"
    $CXX $COMMON_FLAGS -c "$src" -o "$obj"
    OBJECTS="$OBJECTS $obj"
done

for fuzz_src in fuzz/*_fuzzer.cc; do
    name="$(basename "${fuzz_src%.cc}")"
    $CXX $COMMON_FLAGS \
        $LIB_FUZZING_ENGINE \
        "$fuzz_src" \
        $OBJECTS \
        -o "$OUT/${name}"
done

for fuzz_src in fuzz/*_fuzzer.cc; do
    name="$(basename "${fuzz_src%.cc}")"
    short="${name%_fuzzer}"
    candidates=(
        "fuzz/corpus_${name}"
        "fuzz/corpus_${short}"
        "fuzz/${name}_corpus"
    )
    if [ "$short" = "regex" ]; then
        candidates+=("fuzz/corpus")
    fi
    if [ "$name" = "rewriter_identity_fuzzer" ]; then
        candidates+=("fuzz/corpus_rewriter")
    fi
    if [ "$name" = "splitter_roundtrip_fuzzer" ]; then
        candidates+=("fuzz/corpus_splitter")
    fi
    for dir in "${candidates[@]}"; do
        if [ -d "$dir" ] && [ -n "$(ls -A "$dir" 2>/dev/null)" ]; then
            zip -j "$OUT/${name}_seed_corpus.zip" "$dir"/*
            break
        fi
    done
done
