#include "RLECompressor.hpp"
#include <chrono>

using namespace std::chrono;

void encode_naive(FancyInputStream& in, FancyOutputStream& out);
void encode_chunked(FancyInputStream& in, FancyOutputStream& out);
void decode_chunked(FancyInputStream& in, FancyOutputStream& out,uint64_t original_size);
void decode_naive(FancyInputStream& in, FancyOutputStream& out,uint64_t original_size);


AlgorithmId RLECompressor::algorithm_id() const {
    return AlgorithmId::RLE;
}

RLECompressor::RLEVariant RLECompressor::parse_variant(const string &variant){
    if(variant == Config::DEFAULT) return RLEVariant::NAIVE;
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
    FancyInputStream read(input_path.c_str());
    FancyOutputStream write(output_path.c_str());
    RLEVariant rle_variant = parse_variant(variant);

    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::RLE;
    stats.variant_name = variant_name(rle_variant);
    stats.variant_id = static_cast<uint8_t>(rle_variant);
    stats.original_bytes = read.filesize();
    auto start = steady_clock::now();

    if(stats.original_bytes == 0){
        Header::write_file_header(write, AlgorithmId::RLE,stats.variant_id, stats.original_bytes);
        write.flush();
        auto end = steady_clock::now();
        stats.compression_ms = duration<double, std::milli>(end - start).count();
        stats.header_bytes = write.byte_count();
        stats.compressed_bytes = write.byte_count();
        stats.payload_bytes = 0;
        return stats;
    }

    Header::write_file_header(write, AlgorithmId::RLE, stats.variant_id, stats.original_bytes);
    stats.header_bytes = write.byte_count();
    switch (rle_variant)
    {
    case RLEVariant::NAIVE:
        encode_naive(read,write);
        break;
    case RLEVariant::CHUNKED:
        encode_chunked(read, write);
        break;
    default:
        break;
    }


    write.flush();
    stats.compressed_bytes = write.byte_count();
    stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;
    auto end = steady_clock::now();
    stats.compression_ms = duration<double, std::milli>(end - start).count();
    
    return stats;
}

CompressionStats RLECompressor::decompress(
    const string& input_path,
    const string& output_path
) const {
    CompressionStats stats;
    FancyInputStream read(input_path.c_str());
    FancyOutputStream write(output_path.c_str());
    RLEVariant variant;
    int total_bytes;
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::RLE;

    auto start = steady_clock::now();
    if (read.filesize() == 0){
        auto end = steady_clock::now();
        stats.compression_ms = duration<double, std::milli>(end - start).count();
        stats.compressed_bytes = 0;
        return stats;
    }

    FileHeader fh = Header::read_file_header(read);
    total_bytes = fh.original_size;
    variant = parse_variant_id(fh.variant_id);
    stats.algorithm = fh.algorithm;
    stats.variant_id = fh.variant_id;
    stats.variant_name = variant_name(variant);
    stats.original_bytes = fh.original_size;
    stats.compressed_bytes = read.filesize();

    if (stats.original_bytes == 0) {
        stats.header_bytes = Header::shared_header_bytes();
        stats.payload_bytes = 0;
        write.flush();
        auto end = steady_clock::now();
        stats.decompression_ms = duration<double, std::milli>(end - start).count();
        return stats;
    }

    stats.header_bytes = 0;
    stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;

    switch (variant)
    {
    case RLEVariant::NAIVE:
        decode_naive(read, write, total_bytes);
        break;
    case RLEVariant::CHUNKED:
        decode_chunked(read, write, total_bytes);
        break;
    default:
        break;
    }
    write.flush();
    auto end = steady_clock::now();
    stats.decompression_ms = duration<double, std::milli>(end - start).count();
    return stats;
}

void encode_naive(FancyInputStream& in, FancyOutputStream& out) {
    int cur = in.read_byte();
    if (cur == -1) return;

    int count = 1;

    while (true) {
        int next = in.read_byte();

        if (next == cur && count < 255) {
            count++;
        } else {
            out.write_byte(cur);
            out.write_byte(count);

            if (next == -1) break;

            cur = next;
            count = 1;
        }
    }

}

void decode_naive(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {
    uint64_t written = 0;
    while (written < original_size) {
        int byte = in.read_byte();
        int count = in.read_byte();

        for (int i = 0; i < count; ++i) {
            out.write_byte(byte);
            written++;
        }
}


}

void encode_chunked(FancyInputStream& in, FancyOutputStream& out) {

}

void decode_chunked(
    FancyInputStream& in,
    FancyOutputStream& out,
    uint64_t original_size
) {

}
