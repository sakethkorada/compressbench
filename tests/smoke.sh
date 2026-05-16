#!/usr/bin/env bash
set -euo pipefail

bash tests/roundtrip_header.sh tests/test_files/dna.txt sparse tests/out/smoke_sparse
bash tests/roundtrip_header.sh tests/test_files/dna.txt naive tests/out/smoke_naive
bash tests/roundtrip_header.sh tests/test_files/dna.txt bitmask tests/out/smoke_bitmask

echo "PASS: smoke tests matched"
