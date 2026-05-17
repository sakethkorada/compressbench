#include "HuffmanCompressor.hpp"



AlgorithmId HuffmanCompressor::algorithm_id() const {
    return AlgorithmId::HUFFMAN;
}

CompressionStats HuffmanCompressor::compress(
    const string& input_path,
    const string& output_path,
    HeaderMode header_mode
) const {


    CompressionStats stats;
    FancyInputStream read(input_path.c_str());
    FancyOutputStream write(output_path.c_str());
    HCTree hc_tree;
    vector<int> freqs(256,0);
    
    stats.input_path = input_path;
    stats.output_path = output_path;
    stats.algorithm = AlgorithmId::HUFFMAN;
    stats.header_mode = header_mode;
    stats.original_bytes = read.filesize();
    auto start = steady_clock::now();

    if (stats.original_bytes == 0) {
        write.flush();
        auto end = steady_clock::now();
        stats.compression_ms = duration<double, std::milli>(end - start).count();
        stats.compressed_bytes = 0;
        return stats;
    }

    int nextByte = read.read_byte();
    while (nextByte != -1) {
        freqs[nextByte] += 1;
        nextByte = read.read_byte();
    }

    hc_tree.build(freqs);
    Header::write_file_header(write, AlgorithmId::HUFFMAN, header_mode, stats.original_bytes);
    Header::write_huffman_header(write, header_mode, freqs);

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
    HeaderMode header;

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
    header = fh.header_mode;
    stats.algorithm = fh.algorithm;
    stats.header_mode = fh.header_mode;
    stats.original_bytes = fh.original_size;
    stats.compressed_bytes = read.filesize();

    freqs = Header::read_huffman_header(read, header);
    stats.header_bytes = Header::huffman_header_bytes(header, freqs);
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