#ifndef RLE_HPP
#define RLE_HPP

#include <cstdint>

#include "Helper.hpp"

using namespace std;

/**
 * Run-Length Encoding helpers.
 *
 * Supports two RLE strategies so they can be compared directly:
 *
 * 1. naive:
 *      [symbol][run_length]
 *
 * 2. packetized:
 *      0LLLLLLL [literal bytes...]
 *      1LLLLLLL [symbol]
 *
 *    The low 7 bits store (length - 1), so each packet represents 1..128 bytes.
 */
class RLE {
    public:
        static constexpr uint8_t MAX_NAIVE_RUN_LENGTH = 255;
        static constexpr uint8_t MAX_PACKET_LENGTH = 128;
        static constexpr uint8_t RUN_PACKET_MASK = 0x80;
        static constexpr uint8_t LENGTH_MASK = 0x7F;

        /**
         * Encode using simple symbol/run-length pairs:
         *
         *   [symbol: 1 byte][count: 1 byte]
         *
         * Runs longer than 255 are split into multiple pairs.
         */
        static void encode_naive(FancyInputStream& in, FancyOutputStream& out);

        /**
         * Decode data written by encode_naive().
         *
         * original_size is used as a safety bound and tells the decoder
         * exactly when the restored output is complete.
         */
        static void decode_naive(
            FancyInputStream& in,
            FancyOutputStream& out,
            uint64_t original_size
        );

        /**
         * Encode using mixed literal/run packets:
         *
         *   0LLLLLLL = literal packet, followed by L+1 raw bytes
         *   1LLLLLLL = run packet, followed by 1 symbol byte repeated L+1 times
         *
         * This avoids the expansion problem naive RLE has on ordinary text.
         */
        static void encode_packetized(FancyInputStream& in, FancyOutputStream& out);

        /**
         * Decode data written by encode_packetized().
         *
         * original_size is used as a safety bound and tells the decoder
         * exactly when the restored output is complete.
         */
        static void decode_packetized(
            FancyInputStream& in,
            FancyOutputStream& out,
            uint64_t original_size
        );
};

#endif // RLE_HPP
