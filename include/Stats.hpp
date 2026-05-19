#ifndef STATS_HPP
#define STATS_HPP

#include <cstdint>
#include <string>

#include "Header.hpp"

using namespace std;

/**
 * File/content categories used when comparing compression behavior across
 * different kinds of inputs. Start broad; add more only when benchmarks need it.
 */
enum class InputKind : uint8_t {
    UNKNOWN = 0,
    TEXT = 1,
    BINARY = 2,
    DNA = 3,
    IMAGE = 4,
    VIDEO = 5
};

/**
 * Measurements for one compression run.
 *
 * This struct is intentionally algorithm-neutral so future codecs such as RLE,
 * LZSS, image transforms, or video-oriented experiments can report through the
 * same shape.
 */
struct CompressionStats {
    // Identity / experiment context
    string input_path;
    string output_path;
    InputKind input_kind = InputKind::UNKNOWN;
    AlgorithmId algorithm = AlgorithmId::HUFFMAN;
    string variant_name = "unknown";
    uint8_t variant_id = 0;

    // Size accounting
    uint64_t original_bytes = 0;
    uint64_t compressed_bytes = 0;
    uint64_t header_bytes = 0;
    uint64_t payload_bytes = 0;

    // Timing
    double compression_ms = 0.0;
    double decompression_ms = 0.0;

    // Correctness
    bool verified = false;

    /**
     * compressed_bytes / original_bytes.
     * Example: 0.35 means the output is 35% of the original size.
     */
    double compression_ratio() const;

    /**
     * Percent reduction from the original size.
     * Example: 65.0 means 65% smaller than the original.
     */
    double space_saved_percent() const;

    /**
     * Throughput during compression, measured against original input size.
     */
    double compression_throughput_mb_s() const;

    /**
     * Throughput during decompression, measured against restored output size.
     */
    double decompression_throughput_mb_s() const;
};

/**
 * Human-readable helpers for CLI output, reports, and later CSV/JSON export.
 */
class Stats {
    public:
        static const char* input_kind_name(InputKind kind);
        static void print_summary(const CompressionStats& stats);
};

#endif // STATS_HPP
