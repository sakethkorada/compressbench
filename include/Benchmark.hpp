#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <string>
#include <vector>

using namespace std;

class Benchmark {
    public:
        static void run(
            const string& input_directory,
            const string& csv_path = "",
            const vector<string>& algorithm_filters = {},
            const vector<string>& variant_filters = {}
        );
};

#endif // BENCHMARK_HPP
