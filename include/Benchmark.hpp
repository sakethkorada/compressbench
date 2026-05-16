#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <string>

using namespace std;

class Benchmark {
    public:
        static void run(const string& input_directory, const string& csv_path = "");
};

#endif // BENCHMARK_HPP
