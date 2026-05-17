#include "Compressor.hpp"
#include "HCTree.hpp"

class HuffmanCompressor : public Compressor {
public:
    AlgorithmId algorithm_id() const override;
    

    CompressionStats compress(
        const string& input_path,
        const string& output_path,
        HeaderMode header_mode
    ) const override;

    CompressionStats decompress(
        const string& input_path,
        const string& output_path
    ) const override;
};

