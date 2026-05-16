#ifndef HELPER
#define HELPER
#include <cstdint>
#include <fstream>
#include <iostream>
using namespace std;

/**
 * Conveniently crash with error messages
 */
void error(const char* message);

/**
 * Handle reading from a file. You must never call read_byte or read_int after calling read_bit!!!
 */
class FancyInputStream {
    private:
        const char* FILENAME;
        ifstream input_file;
        unsigned char buffer;
        int buffer_index;

    public:
        FancyInputStream(const char* filename);
        bool good() const;
        int filesize() const;
        void reset();
        int read_int();
        int read_byte();
        int read_bit();
};

/**
 * Handle writing to a file. You must never call write_byte or write_int after calling write_bit!!!
 */
class FancyOutputStream {
    private:
        ofstream output_file;
        unsigned char buffer;
        int buffer_index;
        uint64_t bytes_written;

    public:
        FancyOutputStream(const char* filename);
        ~FancyOutputStream();
        bool good() const;
        uint64_t byte_count() const;
        void write_int(int const & num);
        void write_byte(unsigned char const & byte);
        void write_bit(int bit);
        void flush_bitwise();
        void flush();
};

class HCNode {
    public:
        int count;
        unsigned char symbol;
        HCNode* c0;
        HCNode* c1;
        HCNode* p;
        HCNode(int count, unsigned char symbol);
};

class HCNodePtrComp {
    public:
        bool operator()(HCNode*& lhs, HCNode*& rhs) const;
};
#endif // HELPER
