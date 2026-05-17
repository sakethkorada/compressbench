CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude

SRC = src/main.cpp src/HCTree.cpp src/Helper.cpp src/Header.cpp src/Stats.cpp src/Benchmark.cpp src/HuffmanCompressor.cpp
TARGET = compressbench

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o *.huf *.out output.txt restored.txt
