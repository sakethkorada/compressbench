#!/usr/bin/env bash
set -euo pipefail

make clean
make

bash tests/smoke.sh
bash tests/test_matrix.sh
bash tests/test_commands.sh
bash tests/test_invalid.sh

echo "PASS: full CI test suite passed"
