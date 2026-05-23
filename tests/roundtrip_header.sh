#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: $0 <input-file> <variant> [output-prefix]"
    echo "Example: $0 tests/test_files/alphaext.txt sparse tests/out/alphaext_sparse"
    exit 1
fi

INPUT="$1"
VARIANT="$2"
PREFIX="${3:-tests/tmp_${VARIANT}}"
COMPRESSED="${PREFIX}.cbz"
RESTORED="${PREFIX}.out"

if [[ ! -f "$INPUT" ]]; then
    echo "ERROR: input file not found: $INPUT"
    exit 1
fi

mkdir -p "$(dirname "$PREFIX")"

bash tests/roundtrip_variant.sh "$INPUT" huffman "$VARIANT" "$PREFIX"
