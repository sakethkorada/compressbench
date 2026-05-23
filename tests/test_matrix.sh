#!/usr/bin/env bash
set -euo pipefail

TEST_FILES=(
    tests/test_files/alphaext.txt
    tests/test_files/binary
    tests/test_files/dna.txt
    tests/test_files/empty.txt
)

# Add future algorithms here once their compressors are fully implemented.
HUFFMAN_VARIANTS=(naive sparse bitmask)

for input in "${TEST_FILES[@]}"; do
    base="$(basename "$input")"

    for variant in "${HUFFMAN_VARIANTS[@]}"; do
        bash tests/roundtrip_variant.sh \
            "$input" \
            huffman \
            "$variant" \
            "tests/out/${base}_huffman_${variant}"
    done
done

echo "PASS: all supported algorithm/variant/file combinations matched"
