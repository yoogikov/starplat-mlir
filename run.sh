#!/bin/bash

printf "Running Tests\n\n"

MLIR_FILE=$(mktemp /tmp/starplat-XXXXXX.mlir)

./build/bin/app "$1" > "$MLIR_FILE"
if [ $? -ne 0 ]; then
    echo "Frontend failed"
    rm -f "$MLIR_FILE"
    exit 1
fi

./build/bin/starplat-opt "$MLIR_FILE" --starplatir-to-base

rm -f "$MLIR_FILE"

printf '\n\nTesting complete\n'
