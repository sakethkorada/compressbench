#include "RLE.hpp"

AlgorithmId RLECompressor::algorithm_id() const {
    return AlgorithmId::RLE;
}

CompressionStats RLECompressor::compress(
    const string& input_path,
    const string& output_path,
    HeaderMode header_mode
) const {
    CompressionStats stats;
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::RLE;
    stats.header_mode = header_mode;
    return stats;
}

CompressionStats RLECompressor::decompress(
    const string& input_path,
    const string& output_path
) const {
    CompressionStats stats;
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::RLE;
    return stats;
}

void RLECompressor::encode_naive(FancyInputStream& in, FancyOutputStream& out) {

}

void RLECompressor::decode_naive(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {

}

void RLECompressor::encode_chunked(FancyInputStream& in, FancyOutputStream& out) {

}

void RLECompressor::decode_chunked(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {

}
