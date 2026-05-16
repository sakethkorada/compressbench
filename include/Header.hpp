#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>
#include <vector>

#include "Helper.hpp"

using namespace std;

/**
 * Algorithms supported by the CompressBench file format.
 *
 * Keep the numeric values stable once files using them exist in the wild.
 */
enum class AlgorithmId : uint8_t {
    HUFFMAN = 0,
    RLE = 1,
    LZSS = 2
};

/**
 * Metadata/header strategies used after the shared file header.
 *
 * Not every algorithm needs every header mode. For now, the Huffman-specific
 * modes are still listed here so the outer file header can describe them.
 */
enum class HeaderMode : uint8_t {
    NONE = 0,
    NAIVE = 1,
    SPARSE = 2,
    BITMASK = 3,
    TREE = 4,
    CANONICAL = 5
};

/**
 * Shared metadata present at the front of every CompressBench file.
 *
 * This is the algorithm-neutral envelope. Algorithm-specific metadata comes
 * immediately after this shared header.
 */
struct FileHeader {
    uint8_t version;
    AlgorithmId algorithm;
    HeaderMode header_mode;
    uint64_t original_size;
};

/**
 * Read and write CompressBench file metadata.
 *
 * Current shape:
 *   [magic bytes]
 *   [version]
 *   [algorithm id]
 *   [header mode]
 *   [original size]
 *   [algorithm-specific metadata]
 *   [compressed payload]
 */
class Header {
    public:
        static constexpr uint8_t CURRENT_VERSION = 1;
        static constexpr const char* MAGIC_BYTES = "SK219";

        /**
         * Write the shared algorithm-neutral file header.
         */
        static void write_file_header(
            FancyOutputStream& out,
            AlgorithmId algorithm,
            HeaderMode header_mode,
            uint64_t original_size
        );

        /**
         * Read and validate the shared algorithm-neutral file header.
         */
        static FileHeader read_file_header(FancyInputStream& in);

        /**
         * Return whether a metadata/header mode is valid for an algorithm.
         * Useful for CLI validation before writing a file.
         */
        static bool supports(AlgorithmId algorithm, HeaderMode header_mode);
        static uint64_t shared_header_bytes();

        /**
         * Human-readable names for CLI output, stats, and inspect mode.
         */
        static const char* algorithm_name(AlgorithmId algorithm);
        static const char* header_mode_name(HeaderMode header_mode);

        /**
         * Huffman metadata strategies.
         *
         * These are separate from the shared file header because they are not
         * algorithm-neutral; they describe how Huffman should rebuild its tree.
         */
        static void write_huffman_naive_header(
            FancyOutputStream& out,
            const vector<int>& freqs
        );
        static vector<int> read_huffman_naive_header(FancyInputStream& in);

        static void write_huffman_sparse_header(
            FancyOutputStream& out,
            const vector<int>& freqs
        );
        static vector<int> read_huffman_sparse_header(FancyInputStream& in);

        static void write_huffman_bitmask_header(
            FancyOutputStream& out,
            const vector<int>& freqs
        );
        static vector<int> read_huffman_bitmask_header(FancyInputStream& in);

        /**
         * Dispatch helpers so callers do not need to know the concrete
         * Huffman metadata format for each mode.
         */
        static void write_huffman_header(
            FancyOutputStream& out,
            HeaderMode header_mode,
            const vector<int>& freqs
        );
        static vector<int> read_huffman_header(
            FancyInputStream& in,
            HeaderMode header_mode
        );
        static uint64_t huffman_metadata_bytes(
            HeaderMode header_mode,
            const vector<int>& freqs
        );
        static uint64_t huffman_header_bytes(
            HeaderMode header_mode,
            const vector<int>& freqs
        );
};

#endif // HEADER_HPP
