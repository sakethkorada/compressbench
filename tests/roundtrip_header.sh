#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: $0 <input-file> <header-mode> [output-prefix]"
    echo "Example: $0 tests/test_files/alphaext.txt sparse tests/out/alphaext_sparse"
    exit 1
fi

INPUT="$1"
HEADER_MODE="$2"
PREFIX="${3:-tests/tmp_${HEADER_MODE}}"
COMPRESSED="${PREFIX}.cbz"
RESTORED="${PREFIX}.out"

if [[ ! -f "$INPUT" ]]; then
    echo "ERROR: input file not found: $INPUT"
    exit 1
fi

mkdir -p "$(dirname "$PREFIX")"

./compressbench compress "$INPUT" "$COMPRESSED" --header "$HEADER_MODE"
./compressbench decompress "$COMPRESSED" "$RESTORED"

if cmp -s "$INPUT" "$RESTORED"; then
    echo "PASS: $HEADER_MODE round-trip matched for $INPUT"
else
    echo "FAIL: $HEADER_MODE round-trip differed for $INPUT"
    echo "  compressed: $COMPRESSED"
    echo "  restored:   $RESTORED"
    exit 1
fi
