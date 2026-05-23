#!/usr/bin/env bash
set -euo pipefail

mkdir -p tests/out benchmarks

./compressbench compress tests/test_files/dna.txt tests/out/cmd_default.cbz
./compressbench decompress tests/out/cmd_default.cbz tests/out/cmd_default.out
cmp tests/test_files/dna.txt tests/out/cmd_default.out

./compressbench compress tests/test_files/dna.txt tests/out/cmd_stats.cbz --algo huffman --variant sparse --stats
./compressbench decompress tests/out/cmd_stats.cbz tests/out/cmd_stats.out --stats
cmp tests/test_files/dna.txt tests/out/cmd_stats.out

./compressbench benchmark tests/test_files/
./compressbench benchmark tests/test_files/ --csv benchmarks/results.csv
test -s benchmarks/results.csv

echo "PASS: command surface checks passed"
