#ifndef VECTORPACK_BIN_HPP
#define VECTORPACK_BIN_HPP

#include "item.hpp"

#include <string>
#include <vector>

namespace vectorpack {
class Bin;

using BinList = std::vector<Bin*>;

using AllocList = std::vector<int>;


class Bin
{
public:
    Bin(int id, const SizeList& max_capacity);
    Bin(const Bin& other) = default; // Copy ctor

    const int getId() const;
    const SizeList& getMaxCaps() const;
    const int getMaxCapDim(const int dim) const;
    const SizeList& getAvailableCaps() const;
    const int getAvailableCapDim(const int dim) const;

    const AllocList& getAllocList() const;

    void addItem(Item* item);
    bool doesItemFit(const SizeList& sizes) const;

    void printAlloc() const;

    void setMeasure(float measure);
    const float getMeasure() const;
    const int getNbDimensions() const;

protected:
    const int id;
    const int dimensions;
    const SizeList& max_capacities;
    SizeList available_capacities;

    // Vector of item id allocated to this bin
    AllocList alloc_list;

    float measure; // Placeholder for a combined residual capacity measure
};

bool bin_comparator_measure_increasing(Bin* bina, Bin* binb);
bool bin_comparator_measure_decreasing(Bin* bina, Bin* binb);
bool bin_comparator_lexicographic_increasing(Bin* bina, Bin* binb);
bool bin_comparator_lexicographic_decreasing(Bin* bina, Bin* binb);
bool bin_comparator_capacity_dim_increasing(Bin* bina, Bin* binb, int dim);

void bubble_bin_up(BinList::iterator first, BinList::iterator last, bool comp(Bin*, Bin*));
void bubble_bin_down(BinList::iterator first, BinList::iterator last, bool comp(Bin*, Bin*));

} // namespace vectorpack
#endif // VECTORPACK_BIN_HPP
