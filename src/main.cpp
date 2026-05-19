#include <iostream>
#include <string>
#include "Helper.hpp"
#include "Header.hpp"
#include "Stats.hpp"
#include "CompressorFactory.hpp"
#include "Benchmark.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage:\n";
        cerr << "  ./compressbench compress <input> <output> [--algo huffman] [--variant naive|sparse|bitmask] [--stats]\n";
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
    
    string algorithm_name = "default";
    string variant = "default";
    bool show_stats = false;

    // Find, set, and evaluate appropriate flags.
    for (int i = 4; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--algo") {
            if (i + 1 >= argc) {
                error("Missing value after --algo");
            }
            algorithm_name = argv[++i];
        } else if (arg == "--variant") {
            if (i + 1 >= argc) {
                error("Missing value after --variant");
            }
            variant = argv[++i];
        } else if (arg == "--stats") {
            show_stats = true;
        } else {
            error("Unknown option");
        }
    }

    string inputFile = argv[2];
    string outputFile = argv[3];

    CompressionStats stats;
    if (command == "compress") {
        cout << "Compressing " << inputFile << " -> " << outputFile << endl;
        unique_ptr<Compressor> compressor = create_compressor(algorithm_name);
        stats = compressor->compress(inputFile, outputFile, variant);
        if(show_stats) Stats::print_summary(stats);
        return 0;
    }

    if (command == "decompress") {
        cout << "Decompressing " << inputFile << " -> " << outputFile << endl;
        FancyInputStream read(inputFile.c_str());
        FileHeader file_header = Header::read_file_header(read);
        unique_ptr<Compressor> compressor = create_compressor(file_header.algorithm);
        stats = compressor->decompress(inputFile, outputFile);
        if(show_stats) Stats::print_summary(stats);
        return 0;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}
