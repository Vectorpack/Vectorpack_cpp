#ifndef BASE_ALGO_HPP
#define BASE_ALGO_HPP

#include "item.hpp"
#include "instance.hpp"
#include "bin.hpp"

using namespace vectorpack;

// Base class of Algo tailored for vector bin packing
class BaseAlgo
{
public:
    BaseAlgo(const std::string& algo_name, const Instance &instance);
    virtual ~BaseAlgo();

    bool isSolved() const;
    int getSolution() const;
    const BinList& getBins() const;
    BinList getBinsCopy() const;
    const ItemList& getItems() const;

    void orderBinsId(); // Re-order bins in increasing id

    void setSolution(BinList& bins);
    void clearSolution();

    virtual int solveInstance(int hint_nb_bins = 0) = 0; // For Centric algorithms
    virtual int solveInstanceMultiBin(int LB, int UB) = 0; // For Multibin algorithms

protected:
    virtual Bin* createNewBin(); // Open a new empty bin
    virtual bool checkItemToBin(Item* item, Bin* bin) const;
    virtual void addItemToBin(Item* item, Bin* bin);

protected:
    const std::string& name;
    int next_bin_index;
    ItemList items;
    BinList bins;
    const SizeList& bin_max_capacities;
    const Instance& instance;
    const int dimensions;
    bool create_bins_at_end; // Whether a newly created bin should be put at the end of the list
    bool solved;
};

#endif // BASE_ALGO_HPP
