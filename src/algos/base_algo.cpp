#include "base_algo.hpp"

#include <algorithm> // For stable_sort
#include <fstream>

BaseAlgo::BaseAlgo(const std::string& algo_name, const Instance &instance):
    name(algo_name),
    items(ItemList(instance.getItems())),
    bins(BinList(0)),
    bin_max_capacities(instance.getBinCapacities()),
    instance(instance),
    dimensions(instance.getDimensions()),
    next_bin_index(0),
    solved(false),
    create_bins_at_end(true)
{ }

BaseAlgo::~BaseAlgo()
{
    for (Bin* bin : bins)
    {
        if (bin != nullptr)
            delete bin;
    }
}

bool BaseAlgo::isSolved() const
{
    return solved;
}

int BaseAlgo::getSolution() const
{
    return bins.size();
}

const BinList& BaseAlgo::getBins() const
{
    return bins;
}

BinList BaseAlgo::getBinsCopy() const
{
    BinList new_bins;
    new_bins.reserve(bins.size());
    for (Bin* bin : bins)
    {
        new_bins.push_back(new Bin(*bin));
    }
    return new_bins;
}

const ItemList& BaseAlgo::getItems() const
{
    return items;
}

void BaseAlgo::orderBinsId()
{
    std::stable_sort(bins.begin(), bins.end(), bin_comparator_measure_increasing);
}

void BaseAlgo::writeSolution(const std::string& filename, const bool orderBins, const bool itemIdOneBased)
{
    std::ofstream f(filename, std::ios_base::trunc);
    if (!f.is_open())
    {
        std::string s("Cannot write solution to file " + filename);
        throw std::runtime_error(s);
    }

    if (orderBins)
    {
        orderBinsId();
    }

    // Write the number of bins in the first line
    f << bins.size() << "\n";

    // Then for each bin, one bin per line, write the number of items in the bin and the list of item ids
    for (Bin* bin : bins)
    {
        std::vector<int> allocList = bin->getAllocList();
        std::string s(std::to_string(allocList.size()));
        for (int i : allocList)
        {
            if (itemIdOneBased)
            {
                i += 1;
            }
            s += " " + std::to_string(i);
        }
        f << s << "\n";
    }

    f.flush();
    f.close();
}


void BaseAlgo::setSolution(BinList& bins)
{
    clearSolution();
    this->bins = bins;
    solved = true;
}

void BaseAlgo::clearSolution()
{
    solved = false;
    for (Bin* bin : bins)
    {
        if (bin != nullptr)
        {
            delete bin;
        }
    }
    bins.clear();
    next_bin_index = 0;
}

Bin* BaseAlgo::createNewBin()
{
    Bin* bin = new Bin(next_bin_index, bin_max_capacities);
    if (create_bins_at_end)
    {
        bins.push_back(bin);
    }
    else
    {
        // More costly, but necessary for the Worst Fit algorithms for example
        bins.insert(bins.begin(), bin);
    }
    next_bin_index += 1;

    return bin;
}

bool BaseAlgo::checkItemToBin(Item* item, Bin* bin) const
{
    return (bin->doesItemFit(item->getSizes()));
}

void BaseAlgo::addItemToBin(Item* item, Bin* bin)
{
    bin->addItem(item);
}
