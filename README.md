# CompressBench

CompressBench is a C++ compression toolkit and benchmark playground. It currently supports Huffman compression with multiple header strategies, byte-perfect decompression, run statistics, and simple benchmark sweeps across test files.

## Build

```bash
make
```

## CLI reference

```bash
./compressbench compress <input> <output> [--header naive|sparse|bitmask] [--stats]
./compressbench decompress <input> <output> [--stats]
./compressbench benchmark <directory> [--csv <csv-output-path>]
```

Examples:

```bash
./compressbench compress tests/test_files/dna.txt dna.cbz --header sparse --stats
./compressbench decompress dna.cbz dna.out --stats
./compressbench benchmark tests/test_files/ --csv benchmarks/results.csv
```

## Current features

- Huffman compression / decompression
- Shared self-describing file header
- Huffman header modes: `naive`, `sparse`, `bitmask`
- Compression stats: ratio, space saved, timing, header bytes, payload bytes
- Benchmark mode with optional CSV export
- Round-trip verification in tests and benchmarks

## Test helpers

```bash
bash tests/roundtrip.sh <input-file>
bash tests/roundtrip_header.sh <input-file> <header-mode>
bash tests/smoke.sh
bash tests/test_all_headers.sh
```

## Next directions

- Add more algorithms such as RLE and LZSS
- Expand benchmark datasets and reporting
- Explore raw image compression formats and preprocessing
