#ifndef RLECompressor_HPP
#define RLECompressor_HPP

#include "Compressor.hpp"
#include "Helper.hpp"

using namespace std;

/**
 * Run-length encoding compressor.
 *
 * Publicly, RLE exposes the same small compressor contract as every other
 * algorithm. Internally, it can choose different RLE encodings based on the
 * header mode without leaking those details to callers.
 */
class RLECompressor : public Compressor {
    private:
        static constexpr uint8_t MAX_NAIVE_RUN_LENGTH = 255;
        static constexpr uint8_t MAX_CHUNK_LENGTH = 128;
        static constexpr uint8_t RUN_CHUNK_MASK = 0x80;
        static constexpr uint8_t LENGTH_MASK = 0x7F;

        enum class RLEVariant : uint8_t {
            NAIVE = 0,
            CHUNKED = 1
        };

        static RLEVariant parse_variant(const string &variant);
    


    public:
        AlgorithmId algorithm_id() const override;

        CompressionStats compress(
            const string& input_path,
            const string& output_path,
            const string& variant
        ) const override;

        CompressionStats decompress(
            const string& input_path,
            const string& output_path
        ) const override;
};

#endif // RLECompressor_HPP
