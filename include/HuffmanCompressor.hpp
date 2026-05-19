#include "Compressor.hpp"
#include "HCTree.hpp"

class HuffmanCompressor : public Compressor {
private:
    enum class HuffmanVariant : uint8_t {
        NAIVE = 0,
        SPARSE = 1,
        BITMASK = 2
    };

    static HuffmanVariant parse_variant(const string& variant);
    static HuffmanVariant parse_variant_id(uint8_t variant_id);
    static const char* variant_name(HuffmanVariant variant);

    static void write_naive_metadata(FancyOutputStream& out, const vector<int>& freqs);
    static vector<int> read_naive_metadata(FancyInputStream& in);
    static void write_sparse_metadata(FancyOutputStream& out, const vector<int>& freqs);
    static vector<int> read_sparse_metadata(FancyInputStream& in);
    static void write_bitmask_metadata(FancyOutputStream& out, const vector<int>& freqs);
    static vector<int> read_bitmask_metadata(FancyInputStream& in);

    static void write_metadata(
        FancyOutputStream& out,
        HuffmanVariant variant,
        const vector<int>& freqs
    );
    static vector<int> read_metadata(FancyInputStream& in, HuffmanVariant variant);
    static uint64_t metadata_bytes(HuffmanVariant variant, const vector<int>& freqs);
    static uint64_t header_bytes(HuffmanVariant variant, const vector<int>& freqs);

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

