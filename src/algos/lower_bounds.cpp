 #include "lower_bounds.hpp"

#include <cmath>
#include <iostream> // For debugging, TODO remove it
#include <vector>
#include <algorithm>

int BPP_LB1(const Instance & instance)
{
    SizeList sums(instance.getDimensions(), 0);
    for (Item* item : instance.getItems())
    {
        const SizeList& item_sizes = item->getSizes();
        for (int i = 0; i < instance.getDimensions(); ++i)
        {
            sums[i] += item_sizes[i];
        }
    }

    int LB = 0;
    const SizeList& bin_caps = instance.getBinCapacities();
    for (int i = 0; i < instance.getDimensions(); ++i)
    {
        int val = std::ceil(((float)sums[i])/bin_caps[i]);
        if (LB < val )
        {
            LB = val;
        }
    }
    return LB;
}


/* The idea is to create a graph of incompatibility between
 * pairs of items.
 * Then the size of a large clique is a valid lower bound
 * as each item in the clique has to be packed into a different bin.
 * The large clique is computed by means of Johnson's algorithm
 * ("Approximation algorithms for combinatorial problems", 1974
 */
int LB_clique(const Instance & instance)
{
    // First build the adjacency matrix of items
    const ItemList& items = instance.getItems();
    const SizeList& capacities = instance.getBinCapacities();
    int nb_items = items.size();
    std::vector<std::vector<bool>> adj(items.size(), std::vector<bool>(nb_items, false));
    std::vector<int> degrees(nb_items, 0);

    for (int i = 0; i < nb_items; ++i)
    {
        const Item* itemi = items[i];
        for (int j = i+1; j < nb_items; ++j)
        {
            if (check_incompatibility(itemi, items[j], capacities))
            {
                // Items are incompatible, update the adjacency matrix
                adj[i][j] = true;
                adj[j][i] = true;

                // Update their degrees
                degrees[i]+=1;
                degrees[j]+=1;

                //std::cout << "Adjacent items of indices " << i << " " << j << std::endl;
            }
        }
    }

    // Johnson's algo
    std::vector<int> sub_list; // The clique list
    std::vector<int> rest_list(nb_items); // The list of remaining items

    // Rest list contains all items
    for (int i = 0; i < nb_items; ++i)
    {
        rest_list[i] = i;
    }

    // TODO: The complexity of this can probably be greatly improved!
    while(!rest_list.empty())
    {
        // Take the item of highest degree
        int y = argmax_degree(degrees);
        if (y == -1)
        {
            std::cout << "ERROR while computing the LB with clique" << std::endl;
            return -1;
        }

        // Add y in the clique list
        sub_list.push_back(y);
        // This item will be marked to be removed later as it is non adjacent to itself

        std::vector<int> to_remove; // y is non adjacent to itself, it will be removed
        for (const int item : rest_list)
        {
            if (!(adj[y][item]))
            {
                // Remove from rest_list all items NOT adjacent to y
                to_remove.push_back(item);
            }
        }

        for (const int item : to_remove)
        {
            auto it = std::find(rest_list.begin(), rest_list.end(), item);
            rest_list.erase(it);

            // Update degrees of adjacent items
            for (int j = 0; j < nb_items; ++j)
            {
                if (adj[item][j])
                {
                    degrees[j]-=1;
                }
            }

            // Deactivate this item
            degrees[item] = -1;
        }
    }

    return sub_list.size();
}


bool check_incompatibility(const Item* itema, const Item* itemb, const SizeList& capacities)
{
    for (int h = 0; h < capacities.size(); ++h)
    {
        // If cumulated size of both items does not fit the bin capacity in dimension h
        if ( (itema->getSizeDim(h) + itemb->getSizeDim(h)) > capacities[h] )
        {
            return true;
        }
    }
    return false;
}

int argmax_degree(const std::vector<int> degrees)
{
    int max = -1;
    int argmax = -1;
    for (int i = 0; i < degrees.size(); ++i)
    {
        if (max < degrees[i])
        {
            max = degrees[i];
            argmax = i;
        }
    }
    return argmax;
}
