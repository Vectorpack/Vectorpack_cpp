#ifndef ALGOS_MULTIBIN_HPP
#define ALGOS_MULTIBIN_HPP

#include "base_algo.hpp"
#include "weights_measures_scores.hpp"
#include "algos_ItemCentric.hpp"
#include "algos_BinCentric.hpp"

#include <vector>

using namespace vectorpack;

/*
 * 2 main algorithms:
 * - WFDm: repeated calls of ItemCentric WFD algorithm
 *   with initial set of bins
 * - PairCentric: repeated calls of a pairing algo
 *   (variant of bin-centric with scores, considering all bins)
 *
 * 2 variants to select the initial number of bins:
 * - iterative increasing steps from LB to UB, with steps of X bins
 *   X can be an input parameter of the algorithm, or computed
 *   from the values of UB and LB (for example, 10% or 5% of (UB-LB))
 * - modified binary search with 1 bin neightborhood
 *   (because of non-monotonicity, if failure at the target nb bins
 *    check also with +-1 bin)
 *
 * Other variant: incremental bin activation:
 *   start with LB and try to pack items
 *   then, while there are remaining items, activate X new bins
 *   and resume the algorithm with the current partial solution
 *   X is chosen as above
 */


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoPairing: public AlgoBinCentric
{
public:
    AlgoPairing(const std::string& algo_name, const Instance &instance,
                const SCORE score, const WEIGHT weight,
                const bool dynamic_weights,
                const bool use_bin_weights);

    //virtual int solveInstanceMultiBin(int LB, int UB) = 0; // Virtual pure
    virtual bool trySolve(int nb_bins); // Try to solve the instance with given number of bins

protected:
    virtual int solveInstance(int hint_nb_bins = 0);

    virtual void resetAlgo();
    virtual void createNewBins(int nb_bins);
    bool packItems(BinList::iterator start_bin_it);
    void updateScores(Bin* bin, ItemList::iterator first_item, ItemList::iterator end_it);

    std::vector<std::vector<float>> bin_item_scores;
    bool store_scores;
    ItemList::iterator first_remaining_item;
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoPairing_Increment: public AlgoPairing
{
public:
    AlgoPairing_Increment(const std::string& algo_name, const Instance &instance,
                          const SCORE score, const WEIGHT weight,
                          const bool dynamic_weights,
                          const bool use_bin_weights,
                          const int bin_increment_percent);

    virtual int solveInstanceMultiBin(int LB, int UB);

protected:
    int bin_increment_percent;
};

/* ================================================ */
class AlgoPairing_BinSearch: public AlgoPairing
{
public:
    AlgoPairing_BinSearch(const std::string& algo_name, const Instance &instance,
                          const SCORE score, const WEIGHT weight,
                          const bool dynamic_weights,
                          const bool use_bin_weights);

    virtual int solveInstanceMultiBin(int LB, int UB);

protected:
    void updateBestBins(const BinList& new_bins);

    BinList best_bins;
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoWFDm : public AlgoWFD_T1
{
public:
    AlgoWFDm(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               const bool dynamic_weights);

    //virtual int solveInstanceMultiBin(int LB, int UB) = 0; // Virtual pure
    virtual bool trySolve(int nb_bins);

protected:
    virtual int solveInstance(int hint_nb_bins = 0);

    virtual void resetAlgo();
    virtual void createNewBins(int nb_bins);
    bool packItems(BinList::iterator start_bin_it);

    virtual void sortBins(BinList::iterator first_bin, BinList::iterator last_bin);

    ItemList::iterator first_remaining_item;
};



/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoWFDm_Increment : public AlgoWFDm
{
public:
    AlgoWFDm_Increment(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               const bool dynamic_weights,
               const int bin_increment_percent);

    virtual int solveInstanceMultiBin(int LB, int UB);

protected:
    int bin_increment_percent;
};

/* ================================================ */
class AlgoWFDm_BinSearch : public AlgoWFDm
{
public:
    AlgoWFDm_BinSearch(const std::string& algo_name, const Instance &instance,
                 const COMBINATION combination, const WEIGHT weight,
                 const bool dynamic_weights);

    virtual int solveInstanceMultiBin(int LB, int UB);

protected:
    void updateBestBins(const BinList& new_bins);

    BinList best_bins;
};



/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoBFDm_Increment : public AlgoWFDm_Increment
{
public:
    AlgoBFDm_Increment(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               const bool dynamic_weights,
               const int bin_increment_percent);
protected:
    virtual void sortBins(BinList::iterator first_bin, BinList::iterator last_bin);
};

/* ================================================ */
class AlgoBFDm_BinSearch : public AlgoWFDm_BinSearch
{
public:
    AlgoBFDm_BinSearch(const std::string& algo_name, const Instance &instance,
                 const COMBINATION combination, const WEIGHT weight,
                 const bool dynamic_weights);
protected:
    virtual void sortBins(BinList::iterator first_bin, BinList::iterator last_bin);
};

#endif // ALGOS_MULTIBIN_HPP
