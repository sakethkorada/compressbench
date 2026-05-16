#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <input-file> [output-prefix]"
    echo "Example: $0 tests/test_files/alphaext.txt tests/out/alphaext"
    exit 1
fi

INPUT="$1"
PREFIX="${2:-tests/tmp_roundtrip}"
COMPRESSED="${PREFIX}.cbz"
RESTORED="${PREFIX}.out"

if [[ ! -f "$INPUT" ]]; then
    echo "ERROR: input file not found: $INPUT"
    exit 1
fi

mkdir -p "$(dirname "$PREFIX")"

./compressbench compress "$INPUT" "$COMPRESSED"
./compressbench decompress "$COMPRESSED" "$RESTORED"

if cmp -s "$INPUT" "$RESTORED"; then
    echo "PASS: round-trip matched"
    echo "  input:      $INPUT"
    echo "  compressed: $COMPRESSED"
    echo "  restored:   $RESTORED"
else
    echo "FAIL: restored file differs from input"
    echo "  input:    $INPUT"
    echo "  restored: $RESTORED"
    exit 1
fi
