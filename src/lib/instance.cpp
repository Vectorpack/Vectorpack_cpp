#include "instance.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm> // For std::shuffle
#include <random> // For the random generator
#include <bits/stdc++.h>

using namespace std;
using namespace vectorpack;

auto RANDOM_SEED = 23; // For deterministic shuffles

Instance::Instance(const std::string instance_name,
                   const std::string& filename,
                   const bool shuffle_items):
    name(instance_name),
    items_shuffled(shuffle_items)
{
    ifstream ifile(filename.c_str());
    if(!ifile.is_open())
    {
        string s = "Could not open instance file " + filename;
        throw runtime_error(s);
    }
    try {
        string line;

        // Retrieve the number of dimensions
        getline(ifile, line);
        this->dimensions = stoi(line);

        // Retrieve the list of bin capacities
        getline(ifile, line);
        this->capacity_list = retrieveCapacityList(line);

        // Retrieve number of items
        getline(ifile, line);
        this->nb_items = stoi(line);

        // Retrieve the item list
        item_list.reserve(this->nb_items);

        // For each row create one Item
        for(int internal_id = 0; internal_id < this->nb_items; internal_id++)
        {
            getline(ifile, line);
            SizeList sizes;
            FloatList norm_sizes;
            retrieveSizeLists(line, this->capacity_list, sizes, norm_sizes);

            item_list.push_back(new Item(internal_id, sizes, norm_sizes));
        }

        // If needed, shuffle the list of items
        // Just to break optimality on handcrafted instances (such as Falkenauer's triplets)
        if (shuffle_items)
        {
            std::shuffle(item_list.begin(), item_list.end(), std::default_random_engine(RANDOM_SEED));
        }
    }
    catch (exception& e)
    {
        string s = "Problem while loading instance file " + filename + " (" + e.what() + ")";
        throw runtime_error(s);
    }
}


Instance::~Instance()
{
    for (Item* item : item_list)
    {
        if (item != nullptr)
        {
            delete item;
        }
    }
}

const std::string& Instance::getName() const
{
    return name;
}

const int Instance::getDimensions() const
{
    return dimensions;
}

const int Instance::getNbItems() const
{
    return nb_items;
}

const SizeList& Instance::getBinCapacities() const
{
    return capacity_list;
}

const ItemList& Instance::getItems() const
{
    return item_list;
}

const bool Instance::getItemsShuffled() const
{
    return items_shuffled;
}


SizeList vectorpack::retrieveCapacityList(std::string resource_str)
{
    SizeList vect;
    int val;
    std::string str_val;
    std::istringstream splitStream(resource_str);

    while(std::getline(splitStream, str_val, ' '))
    {
        val = std::stoi(str_val);
        vect.push_back(val);
    }
    return vect;
}

void vectorpack::retrieveSizeLists(std::string resource_str, SizeList& capacity_list,
                       SizeList& sizes, FloatList& norm_sizes)
{
    int val;
    std::string str_val;
    std::istringstream splitStream(resource_str);

    int d = capacity_list.size();
    sizes.reserve(d);
    norm_sizes.reserve(d);

    int i = 0;
    while(std::getline(splitStream, str_val, ' ') and (i < d))
    {
        val = std::stoi(str_val);
        sizes.push_back(val);
        norm_sizes.push_back((float)val / (float)capacity_list[i]);
        i++;
    }
}
