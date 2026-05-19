#include "CompressorFactory.hpp"

#include "Helper.hpp"
#include "HuffmanCompressor.hpp"
#include "RLECompressor.hpp"

unique_ptr<Compressor> create_compressor(const string& algorithm_name) {
    if(algorithm_name == Config::DEFAULT){
        return make_unique<HuffmanCompressor>();
    }
    if (algorithm_name == "huffman") {
        return make_unique<HuffmanCompressor>();
    }
    if(algorithm_name == "rle"){
        return make_unique<RLECompressor>();
    }

    error("Unknown algorithm");
    return nullptr;
}

unique_ptr<Compressor> create_compressor(AlgorithmId algorithm_id) {
    if (algorithm_id == AlgorithmId::HUFFMAN) {
        return make_unique<HuffmanCompressor>();
    }
    if(algorithm_id == AlgorithmId::RLE){
        return make_unique<RLECompressor>();
    }

    error("Unsupported algorithm in compressed file");
    return nullptr;
}
