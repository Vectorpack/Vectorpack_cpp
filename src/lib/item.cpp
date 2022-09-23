#include "item.hpp"

using namespace vectorpack;

Item::Item(int id, SizeList& sizes, FloatList& norm_sizes):
    id(id),
    sizes(sizes),
    norm_sizes(norm_sizes),
    measure(0.0)
{ }

const int Item::getId() const
{
    return id;
}

const SizeList& Item::getSizes() const
{
    return sizes;
}

const int Item::getSizeDim(const int dim) const
{
    return sizes.at(dim);
}

const FloatList& Item::getNormSizes() const
{
    return norm_sizes;
}

const float Item::getNormSizeDim(const int dim) const
{
    return norm_sizes.at(dim);
}

std::string Item::toString(const bool full) const
{
    std::string s = std::to_string(id) + " (" + std::to_string(measure) + ")";
    if (full)
    {
        s+=":";
        for (const int size : sizes)
        {
            s+= " " + std::to_string(size);
        }
    }
    return s;
}

void Item::setMeasure(float measure)
{
    this->measure = measure;
}

const float Item::getMeasure() const
{
    return measure;
}

const int Item::getNbDimensions() const
{
    return sizes.size();
}


bool vectorpack::item_comparator_measure_increasing(Item* itema, Item* itemb)
{
    return (itema->getMeasure() < itemb->getMeasure());
}

bool vectorpack::item_comparator_measure_decreasing(Item* itema, Item* itemb)
{
    return (itema->getMeasure() > itemb->getMeasure());
}

bool vectorpack::item_comparator_lexicographic_decreasing(Item* itema, Item* itemb)
{
    for (int h = 0; h < itema->getNbDimensions(); h++)
    {
        if (itema->getSizeDim(h) < itemb->getSizeDim(h))
        {
            return false;
        }
        else if (itema->getSizeDim(h) > itemb->getSizeDim(h))
        {
            return true;
        }
        // else try with next dimension
    }
    return false; // if a == b
}

bool vectorpack::item_comparator_size_dim_increasing(Item* itema, Item* itemb, int dim)
{
    return (itema->getSizeDim(dim) < itemb->getSizeDim(dim));
}


// Perform one round of bubble downwards
void vectorpack::bubble_items_down(ItemList::iterator first, ItemList::iterator last, bool comp (Item*, Item*))
{
    if (first == last) // The vector is empty...
        return;

    auto next = first;
    next++;
    if (next == last) // there is only one element in the vector
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

// Perform one round of bubble upwards
void vectorpack::bubble_items_up(ItemList::iterator first, ItemList::iterator last, bool comp (Item*, Item*))
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
