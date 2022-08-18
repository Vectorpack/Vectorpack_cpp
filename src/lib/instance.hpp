#ifndef VECTORPACK_INSTANCE_HPP
#define VECTORPACK_INSTANCE_HPP

#include "item.hpp"

#include <vector>

namespace vectorpack {
class Instance
{
public:
    Instance(const std::string instance_name,
             const std::string& filename,
             const bool shuffle_items = true);

    virtual ~Instance();

    const std::string& getName() const;
    const int getDimensions() const;
    const int getNbItems() const;
    const SizeList& getBinCapacities() const;
    const ItemList& getItems() const;
    const bool getItemsShuffled() const;
private:
    const std::string name;    // The instance name
    const bool items_shuffled; // Whether the items were shuffled
    int nb_items;              // The number of items
    int dimensions;            // The number of dimensions

    SizeList capacity_list;// The list of bin capacities
    ItemList item_list;   // The list of Items of this instance
};

SizeList retrieveCapacityList(std::string resource_str);
void retrieveSizeLists(std::string resource_str, SizeList& capacity_list,
                       SizeList& sizes, FloatList& norm_sizes);

} // namespace vectorpack
#endif // VECTORPACK_INSTANCE_HPP
