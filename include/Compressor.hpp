#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <string>
#include <chrono>
#include "Header.hpp"
#include "Stats.hpp"

using namespace std;
using namespace std::chrono;

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
         * header_mode is kept here because Huffman currently supports multiple
         * metadata formats. Algorithms that do not need modes can require NONE.
         */
        virtual CompressionStats compress(
            const string& input_path,
            const string& output_path,
            HeaderMode header_mode
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
