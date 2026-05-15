#include <iostream>
#include <string>
#include <vector>

#include "HCTree.hpp"
#include "Helper.hpp"
#include "Header.hpp"
#include "Stats.hpp"

using namespace std;

void printFreqs(const vector<int> &freqs);


int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage:\n";
        cerr << "  ./compressbench compress <input> <output>\n";
        cerr << "  ./compressbench decompress <input> <output>\n";
        return 1;
    }

    string command = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    HCTree hc_tree;
    vector<int> freqs(256,0);
    FancyInputStream read(argv[2]);
    FancyOutputStream write(argv[3]);


    if (command == "compress") {
        cout << "Compressing " << inputFile << " -> " << outputFile << endl;
        
        int unique_symbols = 0;
        if(read.filesize() == 0) return 0;
        
        //read data and update freq of symbols from input
        int nextByte = read.read_byte();
        while(nextByte != -1){
            freqs[nextByte] += 1;
            nextByte = read.read_byte();
        }

        for(const auto &freq: freqs){
            if(freq != 0) unique_symbols++;
        }

        hc_tree.build(freqs);


        /**
            Header/Medadata Aproach:
            First two bytes contain number of symbosl 
            Then for next n symbols
                -first byte is symbol
                -second byte is how long it is (1-4)
                -next 1-4 bytes is the freq
        */
        int byte_number_symbols1 = (unique_symbols >> 8) & 0xFF; 
        int byte_number_symbols2 = unique_symbols & 0xFF;       

        write.write_byte(byte_number_symbols1);
        write.write_byte(byte_number_symbols2);

        for(std::size_t i = 0; i < freqs.size(); ++i){
            int freq = freqs[i];
            if(freq == 0) continue;
            int num_bytes;

            if(freq >> 8 == 0) num_bytes = 1;
            else if(freq >> 16 == 0) num_bytes = 2;
            else if(freq >> 24 == 0) num_bytes = 3;
            else num_bytes = 4;

            write.write_byte((int)i); //write symbol
            write.write_byte(num_bytes); //write num expected bytes
            
            for(int j = 0; j < num_bytes; ++j){
                int shift = 8 * (num_bytes - j - 1);
                int byte_to_write = (freq >> shift) & 0xFF;
                write.write_byte(byte_to_write);
            }
            
        }

        //encode data using hc_codes
        read.reset();
        nextByte = read.read_byte();
        while(nextByte != -1){
            hc_tree.encode(nextByte,write);
            nextByte = read.read_byte();
        }
        
        return 0;
    }

    if (command == "decompress") {
        cout << "Decompressing " << inputFile << " -> " << outputFile << endl;
        
        int total_bytes = 0;
        if(read.filesize() == 0) return 0;
        
        int byte_number_symbols1 = read.read_byte();
        int byte_number_symbols2 = read.read_byte();
        int unique_symbols = (byte_number_symbols1 << 8) | byte_number_symbols2;

        for(int i = 0; i < unique_symbols; ++i){
            int symbol = read.read_byte();
            int num_bytes = read.read_byte();
            int freq = 0;
            
            for (int j = 0; j < num_bytes; ++j) {
                int cur_byte = read.read_byte();
                freq = (freq << 8) | cur_byte;
            }
            freqs[symbol] = freq;
            total_bytes += freq;
        }

        //cout << "total bytes: " << total_bytes;
        //printFreqs(freqs);
        
        HCTree hc_tree;
        hc_tree.build(freqs);
        //hc_tree.printTree();


        
        for(int i = 0; i < total_bytes; ++i){
            int next_char = hc_tree.decode(read);
            //cout << (char)next_char << " ";
            write.write_byte(next_char);
        }


        return 0;
    }

    cerr << "Unknown command: " << command << endl;
    return 1;
}






void printFreqs(const vector<int> &freqs){
    for(std::size_t i = 0; i < freqs.size();++i){
        cout << "Symbol: " << char(i) << " Freq: " << freqs[i] << endl;
    }
}