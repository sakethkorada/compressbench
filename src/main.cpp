#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "HCTree.hpp"
#include "Helper.hpp"
#include "Header.hpp"
#include "Stats.hpp"
#include "HuffmanCompressor.hpp"
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
    Compressor* compressor = new HuffmanCompressor();
    if (command == "compress") {
        cout << "Compressing " << inputFile << " -> " << outputFile << endl;
        stats = compressor->compress(inputFile,outputFile,header_mode);
        if(show_stats) Stats::print_summary(stats);
        return 0;
    }

    if (command == "decompress") {
        cout << "Decompressing " << inputFile << " -> " << outputFile << endl;
        stats = compressor->decompress(inputFile,outputFile);
        if(show_stats) Stats::print_summary(stats);
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
