#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>
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
 * Shared metadata present at the front of every CompressBench file.
 *
 * This is the algorithm-neutral envelope. Algorithm-specific metadata comes
 * immediately after this shared header.
 */
struct FileHeader {
    uint8_t version;
    AlgorithmId algorithm;
    uint8_t variant_id;
    uint64_t original_size;
};

/**
 * Read and write CompressBench file metadata.
 *
 * Current shape:
 *   [magic bytes]
 *   [version]
 *   [algorithm id]
 *   [variant id]
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
            uint8_t variant_id,
            uint64_t original_size
        );

        /**
         * Read and validate the shared algorithm-neutral file header.
         */
        static FileHeader read_file_header(FancyInputStream& in);

        static uint64_t shared_header_bytes();

        /**
         * Human-readable names for CLI output, stats, and inspect mode.
         */
        static const char* algorithm_name(AlgorithmId algorithm);
};

#endif // HEADER_HPP
