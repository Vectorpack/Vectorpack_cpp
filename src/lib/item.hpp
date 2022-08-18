#ifndef VECTORPACK_ITEM_HPP
#define VECTORPACK_ITEM_HPP

#include <vector>
#include <string>

namespace vectorpack {
class Item;

using ItemList = std::vector<Item*>;
using SizeList = std::vector<int>; // A list of size or bin capacity
using FloatList = std::vector<float>; // A list of float, used for weights or normalized size

class Item
{
public:
    Item(int id, SizeList& sizes, FloatList& norm_sizes);

    const int getId() const;
    const SizeList& getSizes() const;
    const int getSizeDim(const int dim) const;
    const FloatList& getNormSizes() const;
    const float getNormSizeDim(const int dim) const;
    std::string toString(const bool full = false) const;

    void setMeasure(float measure);
    const float getMeasure() const;
    const int getNbDimensions() const;

protected:
    int id;     // 0-based item id
    const SizeList sizes;// The list of size in each dimension
    const FloatList norm_sizes; // The list of normalized size in each dimension

    float measure; // Placeholder for a combined size measure
};

bool item_comparator_measure_increasing(Item* itema, Item* itemb);
bool item_comparator_measure_decreasing(Item* itema, Item* itemb);
bool item_comparator_lexicographic_decreasing(Item* itema, Item* itemb);
bool item_comparator_size_dim_increasing(Item* itema, Item* itemb, int dim);
void bubble_items_down(ItemList::iterator first, ItemList::iterator last, bool comp (Item*, Item*));
void bubble_items_up(ItemList::iterator first, ItemList::iterator last, bool comp (Item*, Item*));

} // namespace vectorpack
#endif // VECTORPACK_ITEM_HPP
