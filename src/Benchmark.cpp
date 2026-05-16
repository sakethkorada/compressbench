#include "Benchmark.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "HCTree.hpp"
#include "Header.hpp"
#include "Helper.hpp"
#include "Stats.hpp"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

namespace {
    InputKind infer_input_kind(const fs::path& path) {
        string name = path.filename().string();
        string ext = path.extension().string();

        if (name.find("dna") != string::npos) return InputKind::DNA;
        if (ext == ".txt") return InputKind::TEXT;
        return InputKind::BINARY;
    }

    bool files_equal(const fs::path& lhs, const fs::path& rhs) {
        if (fs::file_size(lhs) != fs::file_size(rhs)) {
            return false;
        }

        ifstream a(lhs, ios::binary);
        ifstream b(rhs, ios::binary);

        if (!a.good() || !b.good()) return false;

        char ca;
        char cb;
        while (a.get(ca) && b.get(cb)) {
            if (ca != cb) return false;
        }

        return true;
    }

    CompressionStats run_huffman_trial(
        const fs::path& input_path,
        HeaderMode header_mode,
        const fs::path& compressed_path,
        const fs::path& restored_path
    ) {
        CompressionStats stats;
        stats.input_path = input_path.string();
        stats.output_path = compressed_path.string();
        stats.input_kind = infer_input_kind(input_path);
        stats.algorithm = AlgorithmId::HUFFMAN;
        stats.header_mode = header_mode;

        string input_name = input_path.string();
        string compressed_name = compressed_path.string();
        string restored_name = restored_path.string();

        FancyInputStream input(input_name.c_str());
        stats.original_bytes = input.filesize();

        auto compression_start = steady_clock::now();

        {
            FancyOutputStream compressed(compressed_name.c_str());

            if (stats.original_bytes > 0) {
                vector<int> freqs(256, 0);
                int next_byte = input.read_byte();
                while (next_byte != -1) {
                    freqs[next_byte] += 1;
                    next_byte = input.read_byte();
                }

                HCTree tree;
                tree.build(freqs);

                Header::write_file_header(
                    compressed,
                    AlgorithmId::HUFFMAN,
                    header_mode,
                    stats.original_bytes
                );
                Header::write_huffman_header(compressed, header_mode, freqs);
                stats.header_bytes = compressed.byte_count();

                input.reset();
                next_byte = input.read_byte();
                while (next_byte != -1) {
                    tree.encode(next_byte, compressed);
                    next_byte = input.read_byte();
                }
            }

            compressed.flush();
            stats.compressed_bytes = compressed.byte_count();
        }

        auto compression_end = steady_clock::now();
        stats.compression_ms = duration<double, milli>(compression_end - compression_start).count();
        stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;

        if (stats.original_bytes == 0) {
            ofstream restored(restored_path, ios::binary);
            restored.close();
            stats.verified = files_equal(input_path, restored_path);
            return stats;
        }

        auto decompression_start = steady_clock::now();

        FancyInputStream encoded(compressed_name.c_str());
        FancyOutputStream restored(restored_name.c_str());
        FileHeader file_header = Header::read_file_header(encoded);
        vector<int> freqs = Header::read_huffman_header(encoded, file_header.header_mode);

        HCTree rebuilt_tree;
        rebuilt_tree.build(freqs);

        for (uint64_t i = 0; i < file_header.original_size; ++i) {
            restored.write_byte(rebuilt_tree.decode(encoded));
        }

        restored.flush();
        auto decompression_end = steady_clock::now();
        stats.decompression_ms = duration<double, milli>(decompression_end - decompression_start).count();
        stats.verified = files_equal(input_path, restored_path);
        return stats;
    }

    void print_table(const vector<CompressionStats>& results) {
        ios old_state(nullptr);
        old_state.copyfmt(cout);

        cout << left
             << setw(18) << "File"
             << setw(10) << "Header"
             << right
             << setw(12) << "Original"
             << setw(12) << "Compressed"
             << setw(10) << "Ratio"
             << setw(10) << "Header"
             << setw(12) << "Comp ms"
             << setw(12) << "Decomp ms"
             << setw(11) << "Verified"
             << '\n';
        cout << string(107, '-') << '\n';

        for (const CompressionStats& stats : results) {
            fs::path path(stats.input_path);
            cout << left
                 << setw(18) << path.filename().string()
                 << setw(10) << Header::header_mode_name(stats.header_mode)
                 << right
                 << setw(12) << stats.original_bytes
                 << setw(12) << stats.compressed_bytes;

            if (stats.original_bytes == 0) {
                cout << setw(10) << "N/A";
            } else {
                cout << fixed << setprecision(1)
                     << setw(9) << stats.compression_ratio() * 100.0 << "%";
            }

            cout << setw(10) << stats.header_bytes
                 << fixed << setprecision(3)
                 << setw(12) << stats.compression_ms
                 << setw(12) << stats.decompression_ms
                 << setw(11) << (stats.verified ? "yes" : "no")
                 << '\n';
        }

        cout.copyfmt(old_state);
    }

    void write_csv(const vector<CompressionStats>& results, const string& csv_path) {
        ofstream csv(csv_path);
        if (!csv.good()) {
            error("Unable to open CSV output path");
        }

        csv << "file,input_kind,algorithm,header_mode,original_bytes,compressed_bytes,"
               "header_bytes,payload_bytes,compression_ratio,space_saved_percent,"
               "compression_ms,decompression_ms,verified\n";

        for (const CompressionStats& stats : results) {
            csv << stats.input_path << ','
                << Stats::input_kind_name(stats.input_kind) << ','
                << Header::algorithm_name(stats.algorithm) << ','
                << Header::header_mode_name(stats.header_mode) << ','
                << stats.original_bytes << ','
                << stats.compressed_bytes << ','
                << stats.header_bytes << ','
                << stats.payload_bytes << ','
                << stats.compression_ratio() << ','
                << stats.space_saved_percent() << ','
                << stats.compression_ms << ','
                << stats.decompression_ms << ','
                << (stats.verified ? "true" : "false") << '\n';
        }
    }
}

void Benchmark::run(const string& input_directory, const string& csv_path) {
    fs::path input_dir(input_directory);
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        error("Benchmark input path must be an existing directory");
    }

    vector<HeaderMode> header_modes = {
        HeaderMode::NAIVE,
        HeaderMode::SPARSE,
        HeaderMode::BITMASK
    };

    fs::path temp_dir = "benchmarks/tmp";
    fs::create_directories(temp_dir);

    vector<CompressionStats> results;
    for (const fs::directory_entry& entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;

        for (HeaderMode mode : header_modes) {
            string stem = entry.path().filename().string() + "_" + Header::header_mode_name(mode);
            fs::path compressed_path = temp_dir / (stem + ".cbz");
            fs::path restored_path = temp_dir / (stem + ".out");
            results.push_back(run_huffman_trial(entry.path(), mode, compressed_path, restored_path));
        }
    }

    print_table(results);

    if (!csv_path.empty()) {
        write_csv(results, csv_path);
        cout << "\nCSV written to " << csv_path << '\n';
    }
}
