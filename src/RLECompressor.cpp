#include "RLECompressor.hpp"

AlgorithmId RLECompressor::algorithm_id() const {
    return AlgorithmId::RLE;
}

RLECompressor::RLEVariant RLECompressor::parse_variant(const string &variant){
    if(variant == Config::DEFAULT) return RLEVariant::CHUNKED;
    if(variant == "chunked") return RLEVariant::CHUNKED;
    if(variant == "naive") return RLEVariant::NAIVE;

    error("Invalid RLE variant");
    return RLEVariant::CHUNKED;
}

RLECompressor::RLEVariant RLECompressor::parse_variant_id(uint8_t variant_id){
    switch (variant_id)
    {
    case static_cast<uint8_t>(RLEVariant::CHUNKED):
        return RLEVariant::CHUNKED;
    case static_cast<uint8_t>(RLEVariant::NAIVE):
        return RLEVariant::NAIVE;
    default:
        error("Invalid RLE variant id");
        return RLEVariant::CHUNKED;
    }

}

const char* RLECompressor::variant_name(RLEVariant variant) {
    switch (variant) {
    case RLEVariant::NAIVE:
        return "naive";
    case RLEVariant::CHUNKED:
        return "chunked";
    default:
        return "unknown";
    }
}


CompressionStats RLECompressor::compress(
    const string& input_path,
    const string& output_path,
    const string& variant
) const {
    CompressionStats stats;
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::RLE;
    stats.variant_name = variant;
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

void encode_naive(FancyInputStream& in, FancyOutputStream& out) {

}

void decode_naive(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {

}

void encode_chunked(FancyInputStream& in, FancyOutputStream& out) {

}

void decode_chunked(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {

}
