#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "HCTree.hpp"
#include "Helper.hpp"
#include "Header.hpp"
#include "Stats.hpp"
#include "Benchmark.hpp"

using namespace std;
using namespace std::chrono;

void printFreqs(const vector<int> &freqs);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage:\n";
        cerr << "  ./compressbench compress <input> <output> [--header naive|sparse|bitmask] [--stats]\n";
        cerr << "  ./compressbench decompress <input> <output> [--stats]\n";
        cerr << "  ./compressbench benchmark <directory> [--csv <path>]\n";
        return 1;
    }

    string command = argv[1];

    if (command == "benchmark") {
        if (argc < 3) {
            error("Missing benchmark directory");
        }

        string csv_path;
        for (int i = 3; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--csv") {
                if (i + 1 >= argc) {
                    error("Missing value after --csv");
                }
                csv_path = argv[++i];
            } else {
                error("Unknown option");
            }
        }

        Benchmark::run(argv[2], csv_path);
        return 0;
    }

    if (argc < 4) {
        error("Missing input or output path");
    }
    
    HeaderMode header_mode = HeaderMode::SPARSE;
    bool show_stats = false;

    // Find, set, and evaluate appropriate flags.
    for (int i = 4; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--header") {
            if (i + 1 >= argc) {
                error("Missing value after --header");
            }
            string value = argv[++i];
            if (value == "naive") {
                header_mode = HeaderMode::NAIVE;
            } else if (value == "sparse") {
                header_mode = HeaderMode::SPARSE;
            } else if (value == "bitmask") {
                header_mode = HeaderMode::BITMASK;
            } else {
                error("Unknown header mode");
            }
        } else if (arg == "--stats") {
            show_stats = true;
        } else {
            error("Unknown option");
        }
    }

    string inputFile = argv[2];
    string outputFile = argv[3];

    CompressionStats stats;
    stats.input_path = inputFile;
    stats.output_path = outputFile;
    stats.algorithm = AlgorithmId::HUFFMAN;

    HCTree hc_tree;
    vector<int> freqs(256, 0);
    FancyInputStream read(argv[2]);
    FancyOutputStream write(argv[3]);

    if (command == "compress") {
        cout << "Compressing " << inputFile << " -> " << outputFile << endl;

        stats.header_mode = header_mode;
        stats.original_bytes = read.filesize();

        auto start = steady_clock::now();

        if (stats.original_bytes == 0) {
            write.flush();
            if (show_stats) {
                auto end = steady_clock::now();
                stats.compression_ms = duration<double, std::milli>(end - start).count();
                stats.compressed_bytes = 0;
                Stats::print_summary(stats);
            }
            return 0;
        }
        
        // Read data and update frequency of symbols from input.
        int nextByte = read.read_byte();
        while (nextByte != -1) {
            freqs[nextByte] += 1;
            nextByte = read.read_byte();
        }

        hc_tree.build(freqs);
        Header::write_file_header(write, AlgorithmId::HUFFMAN, header_mode, stats.original_bytes);
        Header::write_huffman_header(write, header_mode, freqs);

        stats.header_bytes = write.byte_count();

        // Encode data using Huffman codes.
        read.reset();
        nextByte = read.read_byte();
        while (nextByte != -1) {
            hc_tree.encode(nextByte, write);
            nextByte = read.read_byte();
        }

        write.flush();

        if (show_stats) {
            auto end = steady_clock::now();
            stats.compression_ms = duration<double, std::milli>(end - start).count();
            stats.compressed_bytes = write.byte_count();
            stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;
            Stats::print_summary(stats);
        }
        
        return 0;
    }

    if (command == "decompress") {
        cout << "Decompressing " << inputFile << " -> " << outputFile << endl;

        auto start = steady_clock::now();
        int total_bytes;
        HeaderMode header;
        vector<int> freqs;

        if (read.filesize() == 0) return 0;

        FileHeader fh = Header::read_file_header(read);

        total_bytes = fh.original_size;
        header = fh.header_mode;
        stats.algorithm = fh.algorithm;
        stats.header_mode = fh.header_mode;
        stats.original_bytes = fh.original_size;
        stats.compressed_bytes = read.filesize();

        freqs = Header::read_huffman_header(read, header);
        stats.header_bytes = Header::huffman_header_bytes(header, freqs);
        stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;
        
        HCTree hc_tree;
        hc_tree.build(freqs);
    
        for (int i = 0; i < total_bytes; ++i) {
            int next_char = hc_tree.decode(read);
            write.write_byte(next_char);
        }

        write.flush();

        if (show_stats) {
            auto end = steady_clock::now();
            stats.decompression_ms = duration<double, std::milli>(end - start).count();
            Stats::print_summary(stats);
        }
        return 0;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}

void printFreqs(const vector<int> &freqs) {
    for (std::size_t i = 0; i < freqs.size(); ++i) {
        cout << "Symbol: " << char(i) << " Freq: " << freqs[i] << endl;
    }
}
