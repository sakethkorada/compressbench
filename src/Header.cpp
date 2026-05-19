#include "Header.hpp"

namespace {
    constexpr unsigned char MAGIC_BYTES[] = {'S', 'K', '2', '1', '9'};
    constexpr int MAGIC_LENGTH = sizeof(MAGIC_BYTES) / sizeof(MAGIC_BYTES[0]);

    int read_required_byte(FancyInputStream& in, const char* error_message) {
        int byte = in.read_byte();
        if (byte == -1) {
            error(error_message);
        }
        return byte;
    }

    bool is_valid_algorithm_id(int value) {
        return value == static_cast<int>(AlgorithmId::HUFFMAN)
            || value == static_cast<int>(AlgorithmId::RLE)
            || value == static_cast<int>(AlgorithmId::LZSS);
    }
}

void Header::write_file_header(
    FancyOutputStream& out,
    AlgorithmId algorithm,
    uint8_t variant_id,
    uint64_t original_size
) {
    for (int i = 0; i < MAGIC_LENGTH; ++i) {
        out.write_byte(MAGIC_BYTES[i]);
    }

    out.write_byte(CURRENT_VERSION);
    out.write_byte(static_cast<unsigned char>(algorithm));
    out.write_byte(variant_id);

    for (int shift = 56; shift >= 0; shift -= 8) {
        out.write_byte(static_cast<unsigned char>((original_size >> shift) & 0xFF));
    }
}

FileHeader Header::read_file_header(FancyInputStream& in) {
    for (int i = 0; i < MAGIC_LENGTH; ++i) {
        int byte = read_required_byte(in, "Invalid CompressBench file: incomplete magic bytes");
        if (byte != MAGIC_BYTES[i]) {
            error("Invalid CompressBench file: bad magic bytes");
        }
    }

    int version_num = read_required_byte(in, "Invalid CompressBench file: missing format version");
    if (version_num != CURRENT_VERSION) {
        error("Unsupported CompressBench file version");
    }

    int algo_id = read_required_byte(in, "Invalid CompressBench file: missing algorithm id");
    if (!is_valid_algorithm_id(algo_id)) {
        error("Invalid CompressBench file: unknown algorithm id");
    }

    int variant_id = read_required_byte(in, "Invalid CompressBench file: missing variant id");

    uint64_t original_size = 0;
    for (int i = 0; i < 8; ++i) {
        int byte = read_required_byte(in, "Invalid CompressBench file: incomplete original size");
        original_size = (original_size << 8) | static_cast<uint64_t>(byte);
    }

    FileHeader fh;
    fh.version = static_cast<uint8_t>(version_num);
    fh.algorithm = static_cast<AlgorithmId>(algo_id);
    fh.variant_id = static_cast<uint8_t>(variant_id);
    fh.original_size = original_size;
    return fh;
}

uint64_t Header::shared_header_bytes() {
    return MAGIC_LENGTH + 1 + 1 + 1 + 8;
}

const char* Header::algorithm_name(AlgorithmId algorithm) {
    switch (algorithm) {
    case AlgorithmId::HUFFMAN:
        return "huffman";
    case AlgorithmId::RLE:
        return "rle";
    case AlgorithmId::LZSS:
        return "lzss";
    default:
        return "unknown";
    }
}
