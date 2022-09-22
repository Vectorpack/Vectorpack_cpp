#ifndef ALGOS_WEIGHTS_MEASURES_SCORES
#define ALGOS_WEIGHTS_MEASURES_SCORES

#include "item.hpp"
#include <map>

#define ZERO_THRESHOLD 1e-5

using namespace vectorpack;

// For prioritizing dimensions
enum class WEIGHT { // Weighing for combined size measure
     UNIT
    ,AVERAGE
    ,EXPONENTIAL
    ,RECIPROCAL_AVERAGE
    ,UTILIZATION_RATIO // From Gabay and Zaourar 2016
};

// For ItemCentric algos
enum class MEASURE { // To compute the combined size measure
     LINF           // Linf norm, corresponds to a max
    ,L1             // L1 norm, corresponds to a sum
    ,L2             // L2 norm, corresponds to a sum of squares. From Shi, Furlong, Wang 2013
    ,L2_LOAD        // L2 norm, corresponds to a sum of squares w.r.t. bin load. From Shi, Furlong, Wang 2013
};

// For BinCentric and Item-Bin Matching algos
enum class SCORE { // Item-bin scores
     DOT_PRODUCT1
    ,DOT_PRODUCT2
    ,DOT_PRODUCT3
    ,NORM_DOT_PRODUCT
    ,L2NORM
    ,TIGHT_FILL_SUM
    ,TIGHT_FILL_MIN
};


// For prioritizing dimensions
const static std::map<std::string, WEIGHT> map_str_to_weight = {
    { "Unit", WEIGHT::UNIT },
    { "Avg", WEIGHT::AVERAGE },
    { "Expo", WEIGHT::EXPONENTIAL },
    { "ReciprocAvg", WEIGHT::RECIPROCAL_AVERAGE },
    { "UtilRatio", WEIGHT::UTILIZATION_RATIO },
};

// For ItemCentric algos
const static std::map<std::string, MEASURE> map_str_to_measure = {
    { "Linf", MEASURE::LINF },
    { "L1", MEASURE::L1 },
    { "L2", MEASURE::L2 },
    { "L2Load", MEASURE::L2_LOAD },
};

// For BinCentric and Item-Bin Matching algos
const static std::map<std::string, SCORE> map_str_to_score = {
    { "DP1", SCORE::DOT_PRODUCT1 },
    { "DP2", SCORE::DOT_PRODUCT2 },
    { "DP3", SCORE::DOT_PRODUCT3 },
    { "NormDP", SCORE::NORM_DOT_PRODUCT },
    { "L2Norm", SCORE::L2NORM },
    { "TFSum", SCORE::TIGHT_FILL_SUM },
    { "TFMin", SCORE::TIGHT_FILL_MIN },
};


// Utility function, to avoid code duplication across algorithm source files
void utilComputeWeights(const WEIGHT weight,
                    const int dimensions,
                    const int n,
                    FloatList& weights_list,
                    const FloatList& total_norm_size);

// Utility function, to avoid code duplication across algorithm source files
void utilComputeWeightsRatio(const WEIGHT weight,
                             const int dimensions,
                             FloatList& weights_list,
                             const FloatList& total_norm_size,
                             const FloatList& total_norm_residual_capacity);

// Utility function to compute the Norm2 of item sizes or bin capacities
float utilComputeNorm2(const SizeList &list);

// Utility function to compute the Norm2 of normalized item sizes or bin capacities
// normalization_list is the list to normalize each coefficient of list before computing the norm2
float utilComputeNorm2(const SizeList &list, const SizeList &normalization_list);

#endif //ALGOS_WEIGHTS_MEASURES_SCORES
