#ifndef ALGOS_ITEMC_HPP
#define ALGOS_ITEMC_HPP

#include "base_algo.hpp"
#include "weights_measures_scores.hpp"

using namespace vectorpack;

// Base class of AlgoFit tailored for vector bin packing
// With placeholder functions to sort the items, sort the bins
// and determine whether an item can be placed in a bin
class AlgoFit : public BaseAlgo
{
public:
    AlgoFit(const std::string& algo_name, const Instance &instance);

    virtual int solveInstance(int hint_nb_bins = 0);
    virtual int solveInstanceMultiBin(int LB, int UB); // Not used for ItemCentric algos

protected:
    // These are the methods each variant of the Fit algo can re-implement
    virtual void sortBins();
    virtual void sortItems(ItemList::iterator first_item, ItemList::iterator end_it);
    virtual void computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it);

protected:
    bool is_FFD_type; // Whether to compute item measures and sort items
    bool is_FFD_dynamic; // Whether to re-compute weights, item measures and re-order items after each packing
    bool is_BF_type;  // Whether to compute bin measures and sort bins
    bool is_ratio_weight; // Whether weight is of type ratio
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoFF : public AlgoFit
{
public:
    AlgoFF(const std::string& algo_name, const Instance &instance);
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoFFD : public AlgoFit
{
public:
    AlgoFFD(const std::string& algo_name, const Instance &instance,
            const COMBINATION combination, const WEIGHT weight,
            bool dynamic_weights);

protected:
    virtual void sortItems(ItemList::iterator first_item, ItemList::iterator end_it);
    virtual void computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it);
    virtual void addItemToBin(Item* item, Bin* bin);
    virtual Bin* createNewBin(); // Open a new empty bin

    const COMBINATION size_combination;
    const WEIGHT weight;
    FloatList weights_list; // The list of computed weights
    FloatList total_norm_size; // The list of total normalized size of items
    FloatList total_norm_residual_capacity; // The list of residual capacity of all bins, only for ratio weights
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoBFD_T1 : public AlgoFFD
{
public:
    AlgoBFD_T1(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               bool dynamic_weights);

protected:
    virtual void sortBins();
    virtual void addItemToBin(Item* item, Bin* bin);

    void updateBinMeasure(Bin* bin);
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoBFD_T2 : public AlgoFFD
{
public:
    AlgoBFD_T2(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               bool dynamic_item_weights);

protected:
    virtual Bin* createNewBin(); // Open a new empty bin
    virtual void sortBins();
    virtual void addItemToBin(Item* item, Bin* bin);

    void updateBinMeasures();

    FloatList total_norm_residual_capacity; // The list of residual capacity of all bins
    FloatList bin_weights_list; // The list of computed weights for bins
    WEIGHT bin_weight;
};


class AlgoBFD_T3 : public AlgoBFD_T2
{
public:
    AlgoBFD_T3(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT item_weight,
               const WEIGHT bin_weight, bool dynamic_item_weights);
};

/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoBF : public AlgoBFD_T2
{
public:
    AlgoBF(const std::string& algo_name, const Instance &instance,
           const COMBINATION combination, const WEIGHT weight);
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoWFD_T1 : public AlgoBFD_T1
{
public:
    AlgoWFD_T1(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               bool dynamic_weights);

protected:
    virtual void sortBins();
};

class AlgoWFD_T2 : public AlgoBFD_T2
{
public:
    AlgoWFD_T2(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight,
               bool dynamic_item_weights);

protected:
    virtual void sortBins();
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoWF : public AlgoWFD_T2
{
public:
    AlgoWF(const std::string& algo_name, const Instance &instance,
           const COMBINATION combination, const WEIGHT weight);
};




/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoFFD_Lexico : public AlgoFit
{
public:
    AlgoFFD_Lexico(const std::string& algo_name, const Instance &instance);

protected:
    virtual void sortItems(ItemList::iterator first_item, ItemList::iterator end_it);
};

class AlgoBFD_Lexico : public AlgoFFD_Lexico
{
public:
    AlgoBFD_Lexico(const std::string& algo_name, const Instance &instance);

protected:
    virtual void sortBins();
};

class AlgoWFD_Lexico : public AlgoFFD_Lexico
{
public:
    AlgoWFD_Lexico(const std::string& algo_name, const Instance &instance);

protected:
    virtual void sortBins();
};


/* ================================================ */
/* ================================================ */
/* ================================================ */
class AlgoFFD_Rank : public AlgoFit
{
public:
    AlgoFFD_Rank(const std::string& algo_name, const Instance &instance, const bool dynamic_items);

protected:
    virtual void sortItems(ItemList::iterator first_item, ItemList::iterator end_it);
    virtual void computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it);
};

class AlgoBFD_Rank : public AlgoFFD_Rank
{
public:
    AlgoBFD_Rank(const std::string& algo_name, const Instance &instance, const bool dynamic_items);

protected:
    virtual void sortBins();
    void computeBinRanks();
};

class AlgoWFD_Rank : public AlgoBFD_Rank
{
public:
    AlgoWFD_Rank(const std::string& algo_name, const Instance &instance, const bool dynamic_items);

protected:
    virtual void sortBins();
};



#endif // ALGOS_ITEMC_HPP
