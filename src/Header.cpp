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

    bool is_valid_header_mode(int value) {
        return value == static_cast<int>(HeaderMode::NONE)
            || value == static_cast<int>(HeaderMode::NAIVE)
            || value == static_cast<int>(HeaderMode::SPARSE)
            || value == static_cast<int>(HeaderMode::BITMASK)
            || value == static_cast<int>(HeaderMode::TREE)
            || value == static_cast<int>(HeaderMode::CANONICAL);
    }
}

void Header::write_file_header(
    FancyOutputStream& out,
    AlgorithmId algorithm,
    HeaderMode header_mode,
    uint64_t original_size
) {
    // Magic bytes identify this as a CompressBench file.
    for (int i = 0; i < MAGIC_LENGTH; ++i) {
        out.write_byte(MAGIC_BYTES[i]);
    }

    out.write_byte(CURRENT_VERSION);
    out.write_byte(static_cast<unsigned char>(algorithm));
    out.write_byte(static_cast<unsigned char>(header_mode));

    // Write original_size as 8 bytes, most-significant byte first.
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.write_byte(static_cast<unsigned char>((original_size >> shift) & 0xFF));
    }
}

FileHeader Header::read_file_header(FancyInputStream& in) {
    for (int i = 0; i < MAGIC_LENGTH; ++i) {
        int byte = read_required_byte(
            in,
            "Invalid CompressBench file: incomplete magic bytes"
        );

        if (byte != MAGIC_BYTES[i]) {
            error("Invalid CompressBench file: bad magic bytes");
        }
    }

    int version_num = read_required_byte(
        in,
        "Invalid CompressBench file: missing format version"
    );
    if (version_num != CURRENT_VERSION) {
        error("Unsupported CompressBench file version");
    }

    int algo_id = read_required_byte(
        in,
        "Invalid CompressBench file: missing algorithm id"
    );
    if (!is_valid_algorithm_id(algo_id)) {
        error("Invalid CompressBench file: unknown algorithm id");
    }

    int header_mode = read_required_byte(
        in,
        "Invalid CompressBench file: missing header mode"
    );
    if (!is_valid_header_mode(header_mode)) {
        error("Invalid CompressBench file: unknown header mode");
    }

    AlgorithmId algorithm = static_cast<AlgorithmId>(algo_id);
    HeaderMode mode = static_cast<HeaderMode>(header_mode);
    if (!supports(algorithm, mode)) {
        error("Invalid CompressBench file: header mode not supported for algorithm");
    }

    uint64_t original_size = 0;
    for (int i = 0; i < 8; ++i) {
        int byte = read_required_byte(
            in,
            "Invalid CompressBench file: incomplete original size"
        );
        original_size = (original_size << 8) | static_cast<uint64_t>(byte);
    }

    FileHeader fh;
    fh.version = static_cast<uint8_t>(version_num);
    fh.algorithm = algorithm;
    fh.header_mode = mode;
    fh.original_size = original_size;
    return fh;
}

bool Header::supports(AlgorithmId algorithm, HeaderMode header_mode) {
    if (algorithm == AlgorithmId::HUFFMAN) {
        return header_mode == HeaderMode::NAIVE
            || header_mode == HeaderMode::SPARSE
            || header_mode == HeaderMode::BITMASK
            || header_mode == HeaderMode::TREE
            || header_mode == HeaderMode::CANONICAL;
    }

    if (algorithm == AlgorithmId::RLE || algorithm == AlgorithmId::LZSS) {
        return header_mode == HeaderMode::NONE;
    }

    return false;
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

const char* Header::header_mode_name(HeaderMode header_mode) {
    switch (header_mode) {
    case HeaderMode::NONE:
        return "none";
    case HeaderMode::NAIVE:
        return "naive";
    case HeaderMode::SPARSE:
        return "sparse";
    case HeaderMode::BITMASK:
        return "bitmask";
    case HeaderMode::TREE:
        return "tree";
    case HeaderMode::CANONICAL:
        return "canonical";
    default:
        return "unknown";
    }
}

uint64_t Header::shared_header_bytes() {
    return MAGIC_LENGTH + 1 + 1 + 1 + 8;
}









 /**
    Header/Medadata Aproach:
    First two bytes contain number of symbosl 
    Then for next n symbols
        -first byte is symbol
        -second byte is how long it is (1-4)
        -next 1-4 bytes is the freq
*/
void Header::write_huffman_sparse_header(FancyOutputStream& out, const vector<int>& freqs){

    int unique_symbols = 0;
    for(int freq: freqs){
        if(freq != 0) unique_symbols ++;
    }

    int byte_number_symbols1 = (unique_symbols >> 8) & 0xFF; 
        int byte_number_symbols2 = unique_symbols & 0xFF;       

        out.write_byte(byte_number_symbols1);
        out.write_byte(byte_number_symbols2);

        for(std::size_t i = 0; i < freqs.size(); ++i){
            int freq = freqs[i];
            if(freq == 0) continue;
            int num_bytes;

            if(freq >> 8 == 0) num_bytes = 1;
            else if(freq >> 16 == 0) num_bytes = 2;
            else if(freq >> 24 == 0) num_bytes = 3;
            else num_bytes = 4;

            out.write_byte((int)i); //write symbol
            out.write_byte(num_bytes); //write num expected bytes
            
            for(int j = 0; j < num_bytes; ++j){
                int shift = 8 * (num_bytes - j - 1);
                int byte_to_write = (freq >> shift) & 0xFF;
                out.write_byte(byte_to_write);
            }
            
        }

}


vector<int> Header::read_huffman_sparse_header(FancyInputStream& in){

    vector<int> freqs(256,0);
    int byte_number_symbols1 = in.read_byte();
    int byte_number_symbols2 = in.read_byte();
    int unique_symbols = (byte_number_symbols1 << 8) | byte_number_symbols2;

    
    for(int i = 0; i < unique_symbols; ++i){
        int symbol = in.read_byte();
        int num_bytes = in.read_byte();
        int freq = 0;
        
        for (int j = 0; j < num_bytes; ++j) {
            int cur_byte = in.read_byte();
            freq = (freq << 8) | cur_byte;
        }
        freqs[symbol] = freq;

    }

    return freqs;
}


void Header::write_huffman_naive_header(FancyOutputStream& out, const vector<int>& freqs){
    for(int i = 0; i < 256; ++i){
        out.write_int(freqs[i]);
    }
}

vector<int> Header::read_huffman_naive_header(FancyInputStream& in){
    vector<int> freqs(256,0);
    for(int i = 0; i < 256; ++i){
        freqs[i] = in.read_int();
    }
    return freqs;
}


void Header::write_huffman_bitmask_header(FancyOutputStream& out, const vector<int>& freqs){

    vector<unsigned char> mask(32,0);
    int count = 0;
    for(int i = 0; i < 32; ++i)
    {
        for(int j = 0; j < 8; ++j){

            if(freqs[count]!= 0){
                mask[i] |= (1<<(7 - j));
            }
            count++;
        }
    }

    for(const auto &byt: mask){
        out.write_byte(byt);
    }
    for(const int &freq: freqs){
        if(freq != 0) out.write_int(freq);
    }
    return;
}

vector<int> Header::read_huffman_bitmask_header(FancyInputStream& in){

    vector<unsigned char> mask(32,0);
    for(int i = 0; i < 32; ++i){
        mask[i] = in.read_byte();
    }

    vector<bool> is_char_present(256,false);
    vector<int> freqs(256,0);

    int count = 0;
    for(int i = 0; i < 32; ++i){
        for(int j = 0; j < 8; ++j){

            is_char_present[count] = (mask[i] >> (7 - j)) & 1;
            count++;
        }
    }


    for(int i = 0; i < 256; ++i){
        if(is_char_present[i]) freqs[i] = in.read_int();
    }

    return freqs;


}

void Header::write_huffman_header(
    FancyOutputStream& out,
    HeaderMode header_mode,
    const vector<int>& freqs
) {
    switch (header_mode) {
    case HeaderMode::NAIVE:
        write_huffman_naive_header(out, freqs);
        break;
    case HeaderMode::SPARSE:
        write_huffman_sparse_header(out, freqs);
        break;
    case HeaderMode::BITMASK:
        write_huffman_bitmask_header(out, freqs);
        break;
    default:
        error("Unsupported Huffman header mode");
    }
}

vector<int> Header::read_huffman_header(
    FancyInputStream& in,
    HeaderMode header_mode
) {
    switch (header_mode) {
    case HeaderMode::NAIVE:
        return read_huffman_naive_header(in);
    case HeaderMode::SPARSE:
        return read_huffman_sparse_header(in);
    case HeaderMode::BITMASK:
        return read_huffman_bitmask_header(in);
    default:
        error("Unsupported Huffman header mode");
        return vector<int>();
    }
}

uint64_t Header::huffman_metadata_bytes(
    HeaderMode header_mode,
    const vector<int>& freqs
) {
    switch (header_mode) {
    case HeaderMode::NAIVE:
        return 256 * sizeof(int);
    case HeaderMode::SPARSE: {
        uint64_t bytes = 2;
        for (int freq : freqs) {
            if (freq == 0) continue;
            int num_bytes;
            if (freq >> 8 == 0) num_bytes = 1;
            else if (freq >> 16 == 0) num_bytes = 2;
            else if (freq >> 24 == 0) num_bytes = 3;
            else num_bytes = 4;
            bytes += 2 + num_bytes;
        }
        return bytes;
    }
    case HeaderMode::BITMASK: {
        uint64_t present_symbols = 0;
        for (int freq : freqs) {
            if (freq != 0) present_symbols++;
        }
        return 32 + present_symbols * sizeof(int);
    }
    default:
        error("Unsupported Huffman header mode");
        return 0;
    }
}

uint64_t Header::huffman_header_bytes(
    HeaderMode header_mode,
    const vector<int>& freqs
) {
    return shared_header_bytes() + huffman_metadata_bytes(header_mode, freqs);
}






