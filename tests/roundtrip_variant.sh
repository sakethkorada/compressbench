#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 3 || $# -gt 4 ]]; then
    echo "Usage: $0 <input-file> <algorithm> <variant> [output-prefix]"
    echo "Example: $0 tests/test_files/alphaext.txt huffman sparse tests/out/alphaext_huffman_sparse"
    exit 1
fi

INPUT="$1"
ALGO="$2"
VARIANT="$3"
PREFIX="${4:-tests/tmp_${ALGO}_${VARIANT}}"
COMPRESSED="${PREFIX}.cbz"
RESTORED="${PREFIX}.out"

if [[ ! -f "$INPUT" ]]; then
    echo "ERROR: input file not found: $INPUT"
    exit 1
fi

mkdir -p "$(dirname "$PREFIX")"

./compressbench compress "$INPUT" "$COMPRESSED" --algo "$ALGO" --variant "$VARIANT"
./compressbench decompress "$COMPRESSED" "$RESTORED"

if cmp -s "$INPUT" "$RESTORED"; then
    echo "PASS: $ALGO/$VARIANT round-trip matched for $INPUT"
else
    echo "FAIL: $ALGO/$VARIANT round-trip differed for $INPUT"
    echo "  compressed: $COMPRESSED"
    echo "  restored:   $RESTORED"
    exit 1
fi
