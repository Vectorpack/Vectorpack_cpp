#include "bin.hpp"

#include <iostream>
#include <sstream>

using namespace std;
using namespace vectorpack;

Bin::Bin(int id, const SizeList& max_capacity):
    id(id),
    dimensions(max_capacity.size()),
    max_capacities(max_capacity),
    available_capacities(SizeList(max_capacity)),
    measure(0.0)
{ }

const int Bin::getId() const
{
    return id;
}

const SizeList& Bin::getMaxCaps() const
{
    return max_capacities;
}

const int Bin::getMaxCapDim(const int dim) const
{
    return max_capacities[dim];
}

const SizeList& Bin::getAvailableCaps() const
{
    return available_capacities;
}

const int Bin::getAvailableCapDim(const int dim) const
{
    return available_capacities[dim];
}


const AllocList& Bin::getAllocList() const
{
    return alloc_list;
}


void Bin::addItem(Item* item)
{
    // We do NOT check if the item fits
    // it should have been done before calling addItem
    alloc_list.push_back(item->getId());

    const SizeList& item_sizes = item->getSizes();
    for (int i = 0; i < dimensions; ++i)
    {
        available_capacities[i] -= item_sizes[i];
    }
}

bool Bin::doesItemFit(const SizeList& item_sizes) const
{
    for (int i = 0; i < dimensions; ++i)
    {
        if (available_capacities[i] < item_sizes[i])
        {
            return false;
        }
    }
    return true;
}

const std::string Bin::formatAlloc(bool verbose) const
{
    stringstream ss;
    ss << "Bin_" << id;

    if(verbose)
    {
        ss << " (";

        for (int h = 0; h < dimensions; h++)
        {
            ss << to_string(getAvailableCapDim(h)) << "/" << to_string(getMaxCapDim(h)) << " ";
        }
        ss << to_string(getMeasure()) << ")";
    }

    ss << ":";
    for (int item_id : alloc_list)
    {
        ss << " " << to_string(item_id);
    }
    return ss.str();
}

void Bin::printAlloc(bool verbose) const
{
    std::cout << formatAlloc() << std::endl;
}


void Bin::setMeasure(float measure)
{
    this->measure = measure;
}

const float Bin::getMeasure() const
{
    return measure;
}


const int Bin::getNbDimensions() const
{
    return dimensions;
}

bool vectorpack::bin_comparator_id_increasing(Bin* bina, Bin* binb)
{
    return(bina->getId() < binb->getId());
}

bool vectorpack::bin_comparator_measure_increasing(Bin* bina, Bin* binb)
{
    return(bina->getMeasure() < binb->getMeasure());
}

bool vectorpack::bin_comparator_measure_decreasing(Bin* bina, Bin* binb)
{
    return(bina->getMeasure() > binb->getMeasure());
}

bool vectorpack::bin_comparator_lexicographic_increasing(Bin* bina, Bin* binb)
{
    for (int h = 0; h < bina->getNbDimensions(); h++)
    {
        if (bina->getAvailableCapDim(h) > binb->getAvailableCapDim(h))
        {
            return false;
        }
        else if (bina->getAvailableCapDim(h) < binb->getAvailableCapDim(h))
        {
            return true;
        }
        // else try with next dimension
    }
    return false; // if a == b
}

bool vectorpack::bin_comparator_lexicographic_decreasing(Bin* bina, Bin* binb)
{
    for (int h = 0; h < bina->getNbDimensions(); h++)
    {
        if (bina->getAvailableCapDim(h) < binb->getAvailableCapDim(h))
        {
            return false;
        }
        else if (bina->getAvailableCapDim(h) > binb->getAvailableCapDim(h))
        {
            return true;
        }
        // else try with next dimension
    }
    return false; // if a == b
}

bool vectorpack::bin_comparator_capacity_dim_increasing(Bin* bina, Bin* binb, int dim)
{
    return (bina->getAvailableCapDim(dim) < binb->getAvailableCapDim(dim));
}

// Perform one round of bubble upwards
void vectorpack::bubble_bin_up(BinList::iterator first, BinList::iterator last, bool comp(Bin*, Bin*))
{
    if (first == last)
        return; // Maybe first is also at the end
    --last; // last MUST point to the end of the vector
    // In case only one element
    if (first == last)
        return;

    auto current = last;
    auto previous = last-1;
    while(first != previous)
    {
        if (comp(*current, *previous))
        {
            std::iter_swap(current, previous);
        }
        --current;
        --previous;
    }
    // One last time at the head of the vector
    if (comp(*current, *previous))
    {
        std::iter_swap(current, previous);
    }
}



// Perform one round of bubble downwards
void vectorpack::bubble_bin_down(BinList::iterator first, BinList::iterator last, bool comp(Bin*, Bin*))
{
    if (first == last)
        return; // Empty list...

    auto next = first;
    next++;
    if (next == last) // There is only one element in the list
        return;

    auto current = first;
    while(next != last)
    {
        if (comp(*next, *current))
        {
            std::iter_swap(current, next);
        }
        ++current;
        ++next;
    }
}
