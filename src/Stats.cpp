#include "Stats.hpp"

#include <iomanip>
#include <iostream>

namespace {
    constexpr double BYTES_PER_MB = 1024.0 * 1024.0;

    const char* algorithm_name(AlgorithmId algorithm) {
        switch (algorithm) {
        case AlgorithmId::HUFFMAN:
            return "Huffman";
        case AlgorithmId::RLE:
            return "RLE";
        case AlgorithmId::LZSS:
            return "LZSS";
        default:
            return "Unknown";
        }
    }

}

double CompressionStats::compression_ratio() const {
    if (original_bytes == 0) return 0.0;
    return static_cast<double>(compressed_bytes) / original_bytes;
}

double CompressionStats::space_saved_percent() const {
    if (original_bytes == 0) return 0.0;
    return (1.0 - compression_ratio()) * 100.0;
}

double CompressionStats::compression_throughput_mb_s() const {
    if (compression_ms <= 0.0) return 0.0;
    return (static_cast<double>(original_bytes) / BYTES_PER_MB) / (compression_ms / 1000.0);
}

double CompressionStats::decompression_throughput_mb_s() const {
    if (decompression_ms <= 0.0) return 0.0;
    return (static_cast<double>(original_bytes) / BYTES_PER_MB) / (decompression_ms / 1000.0);
}

const char* Stats::input_kind_name(InputKind kind) {
    switch (kind) {
    case InputKind::UNKNOWN:
        return "unknown";
    case InputKind::TEXT:
        return "text";
    case InputKind::BINARY:
        return "binary";
    case InputKind::DNA:
        return "dna";
    case InputKind::IMAGE:
        return "image";
    case InputKind::VIDEO:
        return "video";
    default:
        return "unsupported";
    }
}

void Stats::print_summary(const CompressionStats& stats) {
    ios old_state(nullptr);
    old_state.copyfmt(cout);

    cout << "\nCompression summary\n";
    cout << "-------------------\n";
    cout << "Algorithm:              " << algorithm_name(stats.algorithm) << '\n';
    cout << "Variant:                " << stats.variant_name << '\n';
    cout << "Input kind:             " << input_kind_name(stats.input_kind) << '\n';
    cout << "Original size:          " << stats.original_bytes << " bytes\n";
    cout << "Compressed size:        " << stats.compressed_bytes << " bytes\n";
    cout << "Header size:            " << stats.header_bytes << " bytes\n";
    cout << "Payload size:           " << stats.payload_bytes << " bytes\n";

    if (stats.original_bytes == 0) {
        cout << "Compression ratio:      N/A\n";
        cout << "Space saved:            N/A\n";
    } else {
        cout << fixed << setprecision(2);
        cout << "Compression ratio:      " << stats.compression_ratio() * 100.0 << "%\n";
        cout << "Space saved:            " << stats.space_saved_percent() << "%\n";
    }

    cout << fixed << setprecision(3);
    if (stats.compression_ms > 0.0) {
        cout << "Compression time:       " << stats.compression_ms << " ms\n";
    } else {
        cout << "Compression time:       N/A\n";
    }
    if (stats.decompression_ms > 0.0) {
        cout << "Decompression time:     " << stats.decompression_ms << " ms\n";
    } else {
        cout << "Decompression time:     N/A\n";
    }

    cout << fixed << setprecision(2);
    if (stats.compression_ms > 0.0) {
        cout << "Compression throughput: " << stats.compression_throughput_mb_s() << " MB/s\n";
    } else {
        cout << "Compression throughput: N/A\n";
    }
    if (stats.decompression_ms > 0.0) {
        cout << "Decomp throughput:      " << stats.decompression_throughput_mb_s() << " MB/s\n";
    } else {
        cout << "Decomp throughput:      N/A\n";
    }
    cout << "Verified:               " << (stats.verified ? "yes" : "no") << '\n';

    cout.copyfmt(old_state);
}
