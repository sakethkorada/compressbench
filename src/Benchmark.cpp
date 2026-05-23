#include "Benchmark.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "CompressorFactory.hpp"
#include "Header.hpp"
#include "Helper.hpp"
#include "Stats.hpp"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

namespace {
    struct BenchmarkConfig {
        string algorithm;
        string variant;
    };

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

    vector<string> default_variants_for(const string& algorithm) {
        if (algorithm == "huffman") {
            return {"naive", "sparse", "bitmask"};
        }
        if (algorithm == "rle") {
            return {"naive"};
        }

        error("Unknown benchmark algorithm");
        return {};
    }

    vector<BenchmarkConfig> build_configs(
        const vector<string>& algorithm_filters,
        const vector<string>& variant_filters
    ) {
        vector<BenchmarkConfig> configs;

        vector<string> algorithms = algorithm_filters.empty()
            ? vector<string>{"huffman"}
            : algorithm_filters;

        for (const string& algorithm : algorithms) {
            vector<string> variants = variant_filters.empty()
                ? default_variants_for(algorithm)
                : variant_filters;

            for (const string& variant : variants) {
                configs.push_back({algorithm, variant});
            }
        }

        return configs;
    }

    CompressionStats run_trial(
        const fs::path& input_path,
        const BenchmarkConfig& config,
        const fs::path& compressed_path,
        const fs::path& restored_path
    ) {
        unique_ptr<Compressor> compressor = create_compressor(config.algorithm);
        CompressionStats stats = compressor->compress(
            input_path.string(),
            compressed_path.string(),
            config.variant
        );
        CompressionStats decompression_stats = compressor->decompress(
            compressed_path.string(),
            restored_path.string()
        );
        stats.input_kind = infer_input_kind(input_path);
        stats.decompression_ms = decompression_stats.decompression_ms;
        stats.verified = files_equal(input_path, restored_path);
        return stats;
    }

    void print_table(const vector<CompressionStats>& results) {
        ios old_state(nullptr);
        old_state.copyfmt(cout);

        cout << left
             << setw(18) << "File"
             << setw(10) << "Algo"
             << setw(10) << "Variant"
             << right
             << setw(12) << "Original"
             << setw(12) << "Compressed"
             << setw(10) << "Ratio"
             << setw(10) << "Header"
             << setw(12) << "Comp ms"
             << setw(12) << "Decomp ms"
             << setw(11) << "Verified"
             << '\n';
        cout << string(117, '-') << '\n';

        for (const CompressionStats& stats : results) {
            fs::path path(stats.input_path);
            cout << left
                 << setw(18) << path.filename().string()
                 << setw(10) << Header::algorithm_name(stats.algorithm)
                 << setw(10) << stats.variant_name
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

        csv << "file,input_kind,algorithm,variant,original_bytes,compressed_bytes,"
               "header_bytes,payload_bytes,compression_ratio,space_saved_percent,"
               "compression_ms,decompression_ms,verified\n";

        for (const CompressionStats& stats : results) {
            csv << stats.input_path << ','
                << Stats::input_kind_name(stats.input_kind) << ','
                << Header::algorithm_name(stats.algorithm) << ','
                << stats.variant_name << ','
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

void Benchmark::run(
    const string& input_directory,
    const string& csv_path,
    const vector<string>& algorithm_filters,
    const vector<string>& variant_filters
) {
    fs::path input_dir(input_directory);
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        error("Benchmark input path must be an existing directory");
    }

    vector<BenchmarkConfig> configs = build_configs(algorithm_filters, variant_filters);

    fs::path temp_dir = "benchmarks/tmp";
    fs::create_directories(temp_dir);

    vector<CompressionStats> results;
    for (const fs::directory_entry& entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;

        for (const BenchmarkConfig& config : configs) {
            string stem = entry.path().filename().string() + "_" + config.algorithm + "_" + config.variant;
            fs::path compressed_path = temp_dir / (stem + ".cbz");
            fs::path restored_path = temp_dir / (stem + ".out");
            results.push_back(run_trial(entry.path(), config, compressed_path, restored_path));
        }
    }

    print_table(results);

    if (!csv_path.empty()) {
        write_csv(results, csv_path);
        cout << "\nCSV written to " << csv_path << '\n';
    }
}
