#!/usr/bin/env bash
set -euo pipefail

expect_fail() {
    if "$@" >/tmp/compressbench_invalid_stdout.txt 2>/tmp/compressbench_invalid_stderr.txt; then
        echo "FAIL: expected command to fail: $*"
        exit 1
    else
        echo "PASS: failed as expected: $*"
    fi
}

mkdir -p tests/out

expect_fail ./compressbench
expect_fail ./compressbench compress
expect_fail ./compressbench compress tests/test_files/dna.txt tests/out/bad.cbz --algo
expect_fail ./compressbench compress tests/test_files/dna.txt tests/out/bad.cbz --variant
expect_fail ./compressbench compress tests/test_files/dna.txt tests/out/bad.cbz --algo unknown
expect_fail ./compressbench compress tests/test_files/dna.txt tests/out/bad.cbz --algo huffman --variant nope
expect_fail ./compressbench decompress tests/test_files/dna.txt tests/out/not_compressed.out
expect_fail ./compressbench benchmark tests/does_not_exist

echo "PASS: invalid command checks passed"
