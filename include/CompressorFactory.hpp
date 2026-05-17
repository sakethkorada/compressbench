#ifndef COMPRESSOR_FACTORY_HPP
#define COMPRESSOR_FACTORY_HPP

#include <memory>
#include <string>

#include "Compressor.hpp"

using namespace std;

unique_ptr<Compressor> create_compressor(const string& algorithm_name);
unique_ptr<Compressor> create_compressor(AlgorithmId algorithm_id);

#endif // COMPRESSOR_FACTORY_HPP
