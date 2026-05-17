#!/usr/bin/env bash
set -euo pipefail

VARIANTS=(naive sparse bitmask)
TEST_FILES=(
    tests/test_files/alphaext.txt
    tests/test_files/binary
    tests/test_files/dna.txt
    tests/test_files/empty.txt
)

for mode in "${VARIANTS[@]}"; do
    for input in "${TEST_FILES[@]}"; do
        base="$(basename "$input")"
        bash tests/roundtrip_header.sh "$input" "$mode" "tests/out/${base}_${mode}"
    done
done

echo "PASS: all header-mode round trips matched"
