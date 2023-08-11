#ifndef UTILS_VECTORPACK_HPP
#define UTILS_VECTORPACK_HPP

#include "base_algo.hpp"
#include "algos_ItemCentric.hpp"
#include "algos_BinCentric.hpp"
#include "algos_MultiBin.hpp"
#include "lower_bounds.hpp"

#include <string>

using namespace vectorpack;

std::vector<std::string> splitString(const std::string& str);
bool isRatioWeight(const std::string& str);

// Creator of BaseAlgo variant w.r.t. given algo_name
// Only for ItemCentric and BinCentric algorithms
BaseAlgo* createAlgoCentric(const std::string& algo_name, const Instance &instance);

// Creator of multi-bin algorithms
AlgoPairing* createAlgoPairing(const std::string& algo_name, const Instance &instance);
AlgoWFDm* createAlgoWFDm(const std::string& algo_name, const Instance &instance);

#endif // UTILS_VECTORPACK_HPP
 
