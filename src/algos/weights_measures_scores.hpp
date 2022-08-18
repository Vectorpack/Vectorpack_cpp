#ifndef ALGOS_WEIGHTS_MEASURES_SCORES
#define ALGOS_WEIGHTS_MEASURES_SCORES

#include "item.hpp"

#define ZERO_THRESHOLD 1e-5

using namespace vectorpack;

// For prioritizing dimensions
enum class WEIGHT { // Weighing for combined size measure
     UNIT
    ,AVERAGE
    ,EXPONENTIAL
    ,DIVIDED_AVERAGE // (previously called ExtendedSum)
    ,RESIDUAL_RATIO    // From Gabay and Zaourar 2016
    ,UTILISATION_RATIO // From Gabay and Zaourar 2016
};

// For ItemCentric algos
enum class COMBINATION { // To compute the combined size measure
     SUM
    ,MAX
    ,SUM_SQ             // From Shi, Furlong, Wang 2013
    ,SUM_SQ_LOAD        // From Shi, Furlong, Wang 2013
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
// normalization_list is the list to normalize each coefficient of list before computing the norm2
float utilComputeNorm2(const SizeList &list, const SizeList &normalization_list);

#endif //ALGOS_WEIGHTS_MEASURES_SCORES
