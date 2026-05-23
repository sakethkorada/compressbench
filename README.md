# CompressBench

CompressBench is a C++ compression toolkit and benchmark playground. It currently supports Huffman compression variants, early RLE support, byte-perfect decompression, run statistics, and benchmark sweeps across test files.

## Build

```bash
make
```

## CLI reference

```bash
./compressbench compress <input> <output> [--algo huffman|rle] [--variant <variant>] [--stats]
./compressbench decompress <input> <output> [--stats]
./compressbench benchmark <directory> [--algo <algorithm>] [--variant <variant>] [--csv <csv-output-path>]
```

Examples:

```bash
./compressbench compress tests/test_files/dna.txt dna.cbz --algo huffman --variant sparse --stats
./compressbench compress tests/test_files/dna.txt dna.rle --algo rle --variant naive --stats
./compressbench decompress dna.cbz dna.out --stats
```

Benchmark examples:

```bash
./compressbench benchmark tests/test_files/
./compressbench benchmark tests/test_files/ --csv benchmarks/results.csv
./compressbench benchmark tests/test_files/ --algo huffman
./compressbench benchmark tests/test_files/ --algo huffman --variant sparse
./compressbench benchmark tests/test_files/ --algo rle --variant naive
```

## Current features

- Huffman compression / decompression
- Shared self-describing file header
- Huffman variants: `naive`, `sparse`, `bitmask`
- Early RLE support with `naive` variant
- Compression stats: ratio, space saved, timing, header bytes, payload bytes
- Modular benchmark mode with algorithm/variant filters and optional CSV export
- Round-trip verification in tests and benchmarks

## Test helpers

```bash
bash tests/roundtrip.sh <input-file>
bash tests/roundtrip_variant.sh <input-file> <algorithm> <variant>
bash tests/smoke.sh
bash tests/test_all_headers.sh
bash tests/ci.sh
```

## Next directions

- Finish chunked RLE and add LZSS
- Expand benchmark datasets and reporting
- Explore raw image compression formats and preprocessing
