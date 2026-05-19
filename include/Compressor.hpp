#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <string>
#include "Header.hpp"
#include "Stats.hpp"

using namespace std;

namespace Config {
            inline constexpr std::string_view DEFAULT = "default";
}
/**
 * Common interface for compression algorithms supported by CompressBench.
 *
 * Each concrete compressor owns its algorithm-specific workflow internally:
 * reading any metadata it needs, writing any metadata it needs, and performing
 * the actual compression/decompression work. Callers should not need to know
 * whether an algorithm uses Huffman trees, run lengths, sliding windows, etc.
 */
class Compressor {
    public:
        virtual ~Compressor() = default;

        /**
         * Return the algorithm id written into the shared file header.
         */
        virtual AlgorithmId algorithm_id() const = 0;

        /**
         * Compress one input file into one output file.
         *
         * variant is a user-facing algorithm-local name such as "sparse" for
         * Huffman or "chunked" for a future RLE compressor. Each concrete
         * compressor owns validation and interpretation of its own variants.
         */
        virtual CompressionStats compress(
            const string& input_path,
            const string& output_path,
            const string& variant
        ) const = 0;

        /**
         * Decompress one encoded file into one restored output file.
         *
         * Implementations should read the shared file header plus any
         * algorithm-specific metadata they need from the encoded stream.
         */
        virtual CompressionStats decompress(
            const string& input_path,
            const string& output_path
        ) const = 0;


        
};

#endif // COMPRESSOR_HPP
