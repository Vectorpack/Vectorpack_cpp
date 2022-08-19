#ifndef ALGOS_BINC_HPP
#define ALGOS_BINC_HPP

#include "base_algo.hpp"
#include "weights_measures_scores.hpp"

using namespace vectorpack;

// Base class of Bin-centric algorithm for vector packing
// With placeholder functions to compute scores
class AlgoBinCentric : public BaseAlgo
{
public:
    AlgoBinCentric(const std::string& algo_name, const Instance &instance,
                   const SCORE score, const WEIGHT weight,
                   const bool dynamic_weights,
                   const bool use_bin_weights);

    virtual int solveInstance(int hint_nb_bins = 0);
    virtual int solveInstanceMultiBin(int LB, int UB); // Not used for BinCentric algos

protected:
    virtual Bin* createNewBin();
    virtual void addItemToBin(Item* item, Bin* bin);
    virtual float computeItemBinScore(Item* item, Bin* bin);

protected:
    const SCORE score;
    const WEIGHT weight;
    bool dynamic_weights;
    bool is_ratio_weight;
    bool use_bin_weights; // Whether the weights are based on bin residual capacities

    FloatList weights_list; // The list of computed weights
    FloatList total_norm_size; // The list of total normalized size of items
    FloatList total_norm_residual_capacity; // The list of residual capacity of all bins (normalized values)
};

#endif // ALGOS_BINC_HPP
