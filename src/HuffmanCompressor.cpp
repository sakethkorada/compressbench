#include "HuffmanCompressor.hpp"
#include <chrono>

using namespace std::chrono;




AlgorithmId HuffmanCompressor::algorithm_id() const {
    return AlgorithmId::HUFFMAN;
}

HuffmanCompressor::HuffmanVariant HuffmanCompressor::parse_variant(const string& variant) {
    if (variant == Config::DEFAULT) return HuffmanVariant::SPARSE;
    if (variant == "naive") return HuffmanVariant::NAIVE;
    if (variant == "sparse") return HuffmanVariant::SPARSE;
    if (variant == "bitmask") return HuffmanVariant::BITMASK;
    error("Invalid Huffman variant");
    return HuffmanVariant::SPARSE;
}

HuffmanCompressor::HuffmanVariant HuffmanCompressor::parse_variant_id(uint8_t variant_id) {
    switch (variant_id) {
    case static_cast<uint8_t>(HuffmanVariant::NAIVE):
        return HuffmanVariant::NAIVE;
    case static_cast<uint8_t>(HuffmanVariant::SPARSE):
        return HuffmanVariant::SPARSE;
    case static_cast<uint8_t>(HuffmanVariant::BITMASK):
        return HuffmanVariant::BITMASK;
    default:
        error("Invalid Huffman variant id");
        return HuffmanVariant::SPARSE;
    }
}

const char* HuffmanCompressor::variant_name(HuffmanVariant variant) {
    switch (variant) {
    case HuffmanVariant::NAIVE:
        return "naive";
    case HuffmanVariant::SPARSE:
        return "sparse";
    case HuffmanVariant::BITMASK:
        return "bitmask";
    default:
        return "unknown";
    }
}

CompressionStats HuffmanCompressor::compress(
    const string& input_path,
    const string& output_path,
    const string& variant
) const {
    HuffmanVariant huffman_variant = parse_variant(variant);

    CompressionStats stats;
    FancyInputStream read(input_path.c_str());
    FancyOutputStream write(output_path.c_str());
    HCTree hc_tree;
    vector<int> freqs(256,0);
    
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::HUFFMAN;
    stats.variant_name = variant_name(huffman_variant);
    stats.variant_id = static_cast<uint8_t>(huffman_variant);
    stats.original_bytes = read.filesize();
    auto start = steady_clock::now();

    if (stats.original_bytes == 0) {
        Header::write_file_header(write, AlgorithmId::HUFFMAN, stats.variant_id, stats.original_bytes);
        write.flush();
        auto end = steady_clock::now();
        stats.compression_ms = duration<double, std::milli>(end - start).count();
        stats.header_bytes = write.byte_count();
        stats.compressed_bytes = write.byte_count();
        stats.payload_bytes = 0;
        return stats;
    }

    int nextByte = read.read_byte();
    while (nextByte != -1) {
        freqs[nextByte] += 1;
        nextByte = read.read_byte();
    }

    hc_tree.build(freqs);
    Header::write_file_header(write, AlgorithmId::HUFFMAN, stats.variant_id, stats.original_bytes);
    write_metadata(write, huffman_variant, freqs);

    stats.header_bytes = write.byte_count();

    // Encode data using Huffman codes.
    read.reset();
    nextByte = read.read_byte();
    while (nextByte != -1) {
        hc_tree.encode(nextByte, write);
        nextByte = read.read_byte();
    }

    write.flush();

    auto end = steady_clock::now();
    stats.compression_ms = duration<double, std::milli>(end - start).count();
    stats.compressed_bytes = write.byte_count();
    stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;

    return stats;
}

CompressionStats HuffmanCompressor::decompress(
    const string& input_path,
    const string& output_path
) const {

    CompressionStats stats;
    FancyInputStream read(input_path.c_str());
    FancyOutputStream write(output_path.c_str());
    HCTree hc_tree;
    vector<int> freqs(256,0);
    int total_bytes;
    HuffmanVariant variant;

    stats.input_path = input_path;
    stats.output_path = output_path;

    stats.algorithm = AlgorithmId::HUFFMAN;
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

    freqs = read_metadata(read, variant);
    stats.header_bytes = header_bytes(variant, freqs);
    stats.payload_bytes = stats.compressed_bytes - stats.header_bytes;
   
    hc_tree.build(freqs);

    for (int i = 0; i < total_bytes; ++i) {
        int next_char = hc_tree.decode(read);
        write.write_byte(next_char);
    }
    write.flush();

    auto end = steady_clock::now();
    stats.decompression_ms = duration<double, std::milli>(end - start).count();
    return stats;
}

void HuffmanCompressor::write_naive_metadata(FancyOutputStream& out, const vector<int>& freqs) {
    for (int i = 0; i < 256; ++i) out.write_int(freqs[i]);
}

vector<int> HuffmanCompressor::read_naive_metadata(FancyInputStream& in) {
    vector<int> freqs(256, 0);
    for (int i = 0; i < 256; ++i) freqs[i] = in.read_int();
    return freqs;
}

void HuffmanCompressor::write_sparse_metadata(FancyOutputStream& out, const vector<int>& freqs) {
    int unique_symbols = 0;
    for (int freq : freqs) if (freq != 0) unique_symbols++;
    out.write_byte((unique_symbols >> 8) & 0xFF);
    out.write_byte(unique_symbols & 0xFF);
    for (size_t i = 0; i < freqs.size(); ++i) {
        int freq = freqs[i];
        if (freq == 0) continue;
        int num_bytes;
        if (freq >> 8 == 0) num_bytes = 1;
        else if (freq >> 16 == 0) num_bytes = 2;
        else if (freq >> 24 == 0) num_bytes = 3;
        else num_bytes = 4;
        out.write_byte(static_cast<unsigned char>(i));
        out.write_byte(num_bytes);
        for (int j = 0; j < num_bytes; ++j) {
            int shift = 8 * (num_bytes - j - 1);
            out.write_byte((freq >> shift) & 0xFF);
        }
    }
}

vector<int> HuffmanCompressor::read_sparse_metadata(FancyInputStream& in) {
    vector<int> freqs(256, 0);
    int unique_symbols = (in.read_byte() << 8) | in.read_byte();
    for (int i = 0; i < unique_symbols; ++i) {
        int symbol = in.read_byte();
        int num_bytes = in.read_byte();
        int freq = 0;
        for (int j = 0; j < num_bytes; ++j) freq = (freq << 8) | in.read_byte();
        freqs[symbol] = freq;
    }
    return freqs;
}

void HuffmanCompressor::write_bitmask_metadata(FancyOutputStream& out, const vector<int>& freqs) {
    vector<unsigned char> mask(32, 0);
    for (int symbol = 0; symbol < 256; ++symbol) {
        if (freqs[symbol] != 0) mask[symbol / 8] |= (1 << (7 - (symbol % 8)));
    }
    for (unsigned char byte : mask) out.write_byte(byte);
    for (int freq : freqs) if (freq != 0) out.write_int(freq);
}

vector<int> HuffmanCompressor::read_bitmask_metadata(FancyInputStream& in) {
    vector<unsigned char> mask(32, 0);
    for (int i = 0; i < 32; ++i) mask[i] = in.read_byte();
    vector<int> freqs(256, 0);
    for (int symbol = 0; symbol < 256; ++symbol) {
        bool present = (mask[symbol / 8] >> (7 - (symbol % 8))) & 1;
        if (present) freqs[symbol] = in.read_int();
    }
    return freqs;
}

void HuffmanCompressor::write_metadata(
    FancyOutputStream& out,
    HuffmanVariant variant,
    const vector<int>& freqs
) {
    switch (variant) {
    case HuffmanVariant::NAIVE:
        write_naive_metadata(out, freqs); break;
    case HuffmanVariant::SPARSE:
        write_sparse_metadata(out, freqs); break;
    case HuffmanVariant::BITMASK:
        write_bitmask_metadata(out, freqs); break;
    }
}

vector<int> HuffmanCompressor::read_metadata(FancyInputStream& in, HuffmanVariant variant) {
    switch (variant) {
    case HuffmanVariant::NAIVE:
        return read_naive_metadata(in);
    case HuffmanVariant::SPARSE:
        return read_sparse_metadata(in);
    case HuffmanVariant::BITMASK:
        return read_bitmask_metadata(in);
    }
    return vector<int>();
}

uint64_t HuffmanCompressor::metadata_bytes(HuffmanVariant variant, const vector<int>& freqs) {
    switch (variant) {
    case HuffmanVariant::NAIVE:
        return 256 * sizeof(int);
    case HuffmanVariant::SPARSE: {
        uint64_t bytes = 2;
        for (int freq : freqs) {
            if (freq == 0) continue;
            int n = (freq >> 8 == 0) ? 1 : (freq >> 16 == 0) ? 2 : (freq >> 24 == 0) ? 3 : 4;
            bytes += 2 + n;
        }
        return bytes;
    }
    case HuffmanVariant::BITMASK: {
        uint64_t present = 0;
        for (int freq : freqs) if (freq != 0) present++;
        return 32 + present * sizeof(int);
    }
    }
    return 0;
}

uint64_t HuffmanCompressor::header_bytes(HuffmanVariant variant, const vector<int>& freqs) {
    return Header::shared_header_bytes() + metadata_bytes(variant, freqs);
}
