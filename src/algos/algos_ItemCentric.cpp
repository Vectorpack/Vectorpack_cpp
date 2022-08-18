#include "algos_ItemCentric.hpp"

#include <algorithm> // For stable_sort
#include <cmath> // For exp
#include <stdexcept> // For throwing stuff

#include <iostream> // Simple debugging TODO remove it

AlgoFit::AlgoFit(const std::string& algo_name, const Instance &instance):
    BaseAlgo(algo_name, instance),
    is_FFD_type(false),
    is_FFD_dynamic(false),
    is_BF_type(false),
    is_ratio_weight(false)
{ }

void AlgoFit::sortBins()
{ }
void AlgoFit::sortItems(ItemList::iterator first_item, ItemList::iterator end_it)
{ }
void AlgoFit::computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it)
{ }


// Generic algorithm based on first fit
// The hint is an estimate on the number of bins to allocate
int AlgoFit::solveInstance(int hint_nb_bins)
{
    if(isSolved())
    {
        return getSolution(); // No need to solve twice
    }
    if(hint_nb_bins > 0)
    {
        bins.reserve(hint_nb_bins); // Small memory optimisation
    }

    if (is_FFD_type)
    {
        // Pre-processing to compute combined size measure of all items
        computeItemMeasures(items.begin(), items.end());

        // Renumber items in decreasing size
        sortItems(items.begin(), items.end());
    }

    bool allocated = false;
    int total_items = instance.getNbItems();

    // For all items in the list
    auto curr_item_it = items.begin();
    auto end_items_it = items.end();
    while(curr_item_it != end_items_it)
    {
        Item * item = *curr_item_it;

        auto curr_bin_it = bins.begin();
        allocated = false;
        while ((!allocated) and (curr_bin_it != bins.end()))
        {
            if (checkItemToBin(item, *curr_bin_it))
            {
                addItemToBin(item, *curr_bin_it);
                allocated = true;
            }
            else
            {
                ++curr_bin_it;
            }
        }

        if (!allocated)
        {
            // The item did not fit in any bin, create a new one
            Bin* bin = createNewBin();

            // This is a quick safe guard to avoid infinite loops and running out of memory
            if (bins.size() > total_items)
            {
                std::string s = "There seem to be a problem with algo " + name + " and instance " + instance.getName() + ", created more bins than items (" + std::to_string(bins.size()) + ").";
                throw std::runtime_error(s);
            }

            addItemToBin(item, bin);
        }

        // Advance to next item to pack
        ++curr_item_it;

        // Update weights, item measures and order of remaining items
        // (only for FFD-dynamic algos)
        if (is_FFD_dynamic)
        {

            computeItemMeasures(curr_item_it, end_items_it);
            sortItems(curr_item_it, end_items_it);
        }

        // Update bins order (only for BF-type algos)
        if (is_BF_type)
        {
            // Bin measures must have been updated when last
            // item was added to a bin
            sortBins();
        }
    }

    solved = true;
    return getSolution();
}



/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoFF::AlgoFF(const std::string& algo_name, const Instance &instance):
    AlgoFit(algo_name, instance)
{ }


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoFFD::AlgoFFD(const std::string& algo_name, const Instance &instance,
                 const COMBINATION combination, const WEIGHT weight,
                 bool dynamic_weights):
    AlgoFit(algo_name, instance),
    size_combination(combination),
    weights_list(instance.getDimensions(), 1.0),
    weight(weight)
{
    is_FFD_type = true;
    is_FFD_dynamic = dynamic_weights;

    if ((weight == WEIGHT::RESIDUAL_RATIO) or (weight == WEIGHT::UTILISATION_RATIO))
    {
        is_ratio_weight = true;
        is_FFD_dynamic = true;
        total_norm_residual_capacity = FloatList(dimensions, 0.0); // Contains normalized values
    }

    // The value of total_norm_size is kept and updated after each packing of item in case of dynamic weights
    total_norm_size = FloatList(dimensions, 0.0);
    for(Item * item : items)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] += item->getNormSizeDim(h);
        }
    }
}

void AlgoFFD::addItemToBin(Item* item, Bin* bin)
{
    bin->addItem(item);

    if (is_FFD_dynamic)
    {
        // Update the total normalized size for dynamic weights
        if (weight != WEIGHT::RESIDUAL_RATIO)
        {
            // Residual ratio weights do not depend on total_norm_size
            for (int h = 0; h < dimensions; ++h)
            {
                total_norm_size[h] -= item->getNormSizeDim(h);
            }
        }

        if (is_ratio_weight)
        {
            for (int h = 0; h < dimensions; ++h)
            {
                total_norm_residual_capacity[h] -= item->getNormSizeDim(h);
            }
        }
    }
}

Bin* AlgoFFD::createNewBin()
{
    Bin* bin = BaseAlgo::createNewBin();

    if (is_ratio_weight)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_residual_capacity[h] += 1.0; // The normalized residual capacity of an empty bin is 1
        }
    }
    return bin;
}


void AlgoFFD::sortItems(ItemList::iterator first_item, ItemList::iterator end_it)
{
    if (is_FFD_dynamic)
    {
        // Only need to bring forward the item of highest measure
        // as this function will be called again after the packing of each item
        bubble_items_up(first_item, end_it, item_comparator_measure_decreasing);
    }
    else
    {
        stable_sort(first_item, end_it, item_comparator_measure_decreasing);
    }
}

void AlgoFFD::computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it)
{
    if (is_ratio_weight)
    {
        utilComputeWeightsRatio(weight, dimensions, weights_list, total_norm_size, total_norm_residual_capacity);
    }
    else
    {
        utilComputeWeights(weight, dimensions, (end_it - first_item), weights_list, total_norm_size);
    }

    switch(size_combination)
    {
    case COMBINATION::SUM:
        for(auto item_it = first_item; item_it != end_it; ++item_it)
        {
            Item * item = *item_it;
            float item_size = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                item_size += weights_list[h] * item->getNormSizeDim(h);
            }
            item->setMeasure(item_size);
        }
        break;
    case COMBINATION::MAX:
        for(auto item_it = first_item; item_it != end_it; ++item_it)
        {
            Item * item = *item_it;
            float max_size = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                max_size = std::max(max_size, weights_list[h] * item->getNormSizeDim(h));
            }
            item->setMeasure(max_size);
        }
        break;
    case COMBINATION::SUM_SQ:
    case COMBINATION::SUM_SQ_LOAD:
        // For these 2 combinations, the item measure is computed the same
        for (auto item_it = first_item; item_it != end_it; ++item_it)
        {
            Item* item = *item_it;
            float value = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                value += weights_list[h] * item->getNormSizeDim(h) * item->getNormSizeDim(h);
            }
            item->setMeasure(std::sqrt(value));
        }
        break;
    }
}



/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoBFD_T1::AlgoBFD_T1(const std::string& algo_name, const Instance &instance,
                 const COMBINATION combination, const WEIGHT weight,
                       bool dynamic_weights):
    AlgoFFD(algo_name, instance, combination, weight, dynamic_weights)
{
    is_BF_type = true;
}

void AlgoBFD_T1::sortBins()
{
    if (is_FFD_dynamic)
    {
        // The item weights have changed, need to re-compute all bin measures
        for (Bin* bin : bins)
        {
            updateBinMeasure(bin);
        }

        // Then re-order the whole list of bins
        stable_sort(bins.begin(), bins.end(), bin_comparator_measure_increasing);
    }
    else
    {
        // The measure of only one bin has changed, no need to perform complete sort
        bubble_bin_up(bins.begin(), bins.end(), bin_comparator_measure_increasing);
    }
}


void AlgoBFD_T1::addItemToBin(Item *item, Bin *bin)
{
    bin->addItem(item);

    if (is_FFD_dynamic)
    {
        // Update the total normalized size for dynamic weights
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] -= item->getNormSizeDim(h);
        }
    }

    // If dynamic weights are used, all bin measures will be updated later (during call to sortbins)
    if (!is_FFD_dynamic)
    {
        updateBinMeasure(bin);
    }
}

void AlgoBFD_T1::updateBinMeasure(Bin *bin)
{
    float val_residual = 0.0;
    switch(size_combination)
    {
    case COMBINATION::SUM:
        for (int h = 0; h < dimensions; ++h)
        {
            // We need the normalized residual bin capacity
            val_residual += weights_list[h] * ((float)bin->getAvailableCapDim(h)) / bin_max_capacities[h];
        }
        bin->setMeasure(val_residual);
        break;
    case COMBINATION::MAX:
        for (int h = 0; h < dimensions; ++h)
        {
            // We need the normalized residual bin capacity
            val_residual = std::max(val_residual, weights_list[h] * ((float)bin->getAvailableCapDim(h)) / bin_max_capacities[h]);
        }
        bin->setMeasure(val_residual);
        break;
    case COMBINATION::SUM_SQ:
        for (int h = 0; h < dimensions; ++h)
        {
            // Need normalized values
            float f = ((float)bin->getAvailableCapDim(h)) / bin_max_capacities[h];
            val_residual += weights_list[h] * f*f;
        }
        bin->setMeasure(std::sqrt(val_residual));
        break;
    case COMBINATION::SUM_SQ_LOAD:
        for (int h = 0; h < dimensions; ++h)
        {
            // Normalized value of used capacity (normalized load of the bin)
            float f = ((float)(bin_max_capacities[h] - bin->getAvailableCapDim(h))) / bin_max_capacities[h];
            val_residual += weights_list[h] * f*f;
        }
        bin->setMeasure(std::sqrt(val_residual));
        break;
    }
}



/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoBFD_T2::AlgoBFD_T2(const std::string& algo_name, const Instance &instance,
                       const COMBINATION combination, const WEIGHT weight,
                       bool dynamic_item_weights):
    AlgoFFD(algo_name, instance, combination, weight, dynamic_item_weights)
{
    is_BF_type = true;
    this->bin_weight = weight;

    bin_weights_list = FloatList(dimensions, 1.0);
    total_norm_residual_capacity = FloatList(dimensions, 0.0); // Contains normalized values
}

Bin* AlgoBFD_T2::createNewBin()
{
    Bin* bin = BaseAlgo::createNewBin();

    for (int h = 0; h < dimensions; ++h)
    {
        total_norm_residual_capacity[h] += 1.0; // The normalized residual capacity of an empty bin is 1
    }
    return bin;
}

void AlgoBFD_T2::sortBins()
{
    // The measure of all bins have been updated, need to re-order the whole list
    stable_sort(bins.begin(), bins.end(), bin_comparator_measure_increasing);
}


void AlgoBFD_T2::addItemToBin(Item *item, Bin *bin)
{
    bin->addItem(item);

    if (is_FFD_dynamic)
    {
        // Update the total normalized size for dynamic item weights
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] -= item->getNormSizeDim(h);
        }
    }

    for (int h = 0; h < dimensions; ++h)
    {
        total_norm_residual_capacity[h] -= item->getNormSizeDim(h);
    }
    updateBinMeasures();
}


void AlgoBFD_T2::updateBinMeasures()
{
    // For type2 BFD, need to update measure of ALL bins
    if (is_ratio_weight)
    {
        utilComputeWeightsRatio(bin_weight, dimensions, bin_weights_list, total_norm_size, total_norm_residual_capacity);
    }
    else
    {
        utilComputeWeights(bin_weight, dimensions, bins.size(), bin_weights_list, total_norm_residual_capacity);
    }

    switch(size_combination)
    {
    case COMBINATION::SUM:
        for (Bin* b : bins)
        {
            float bin_residual = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                // We need the normalized residual bin capacity
                bin_residual += bin_weights_list[h] * ((float)b->getAvailableCapDim(h)) / bin_max_capacities[h];
            }
            b->setMeasure(bin_residual);
        }
        break;
    case COMBINATION::MAX:
        for (Bin* b : bins)
        {
            float max_residual = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                // We need the normalized residual bin capacity
                max_residual = std::max(max_residual, bin_weights_list[h] * ((float)b->getAvailableCapDim(h)) / bin_max_capacities[h]);
            }
            b->setMeasure(max_residual);
        }
        break;
    case COMBINATION::SUM_SQ:
        for (Bin* b : bins)
        {
            float value = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                // Need normalized values
                float f = ((float)b->getAvailableCapDim(h)) / bin_max_capacities[h];
                value += bin_weights_list[h] * f*f;
            }
            b->setMeasure(std::sqrt(value));
        }
        break;
    case COMBINATION::SUM_SQ_LOAD:
        for (Bin* b : bins)
        {
            float value = 0.0;
            for (int h = 0; h < dimensions; ++h)
            {
                // Normalized value of used capacity (normalized load of the bin)
                float f = ((float)(bin_max_capacities[h] - b->getAvailableCapDim(h))) / bin_max_capacities[h];
                value += bin_weights_list[h] * f*f;
            }
            b->setMeasure(std::sqrt(value));
        }
        break;
    }
}



/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoBFD_T3::AlgoBFD_T3(const std::string& algo_name, const Instance &instance,
                       const COMBINATION combination, const WEIGHT item_weight,
                       const WEIGHT bin_weight, bool dynamic_item_weights):
    AlgoBFD_T2(algo_name, instance, combination, item_weight, dynamic_item_weights)
{ 
    this->bin_weight = bin_weight;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoBF::AlgoBF(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight):
    AlgoBFD_T2(algo_name, instance, combination, weight, false)
{
    is_FFD_type = false;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWFD_T1::AlgoWFD_T1(const std::string& algo_name, const Instance &instance,
                       const COMBINATION combination, const WEIGHT weight,
                       bool dynamic_weights):
    AlgoBFD_T1(algo_name, instance, combination, weight, dynamic_weights)
{
    create_bins_at_end = false; // Necessary when bins are bubbled down
}

void AlgoWFD_T1::sortBins()
{
    if (is_FFD_dynamic)
    {
        // The item weights have changed, need to re-compute all bin measures
        for (Bin* bin : bins)
        {
            updateBinMeasure(bin);
        }

        // Then re-order the whole list of bins
        stable_sort(bins.begin(), bins.end(), bin_comparator_measure_decreasing);
    }
    else
    {
        // The measure of only one bin has changed, no need to perform complete sort
        bubble_bin_down(bins.begin(), bins.end(), bin_comparator_measure_decreasing);
    }
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWFD_T2::AlgoWFD_T2(const std::string& algo_name, const Instance &instance,
                       const COMBINATION combination, const WEIGHT weight,
                       bool dynamic_weights):
    AlgoBFD_T2(algo_name, instance, combination, weight, dynamic_weights)
{
    create_bins_at_end = false; // Necessary when bins are bubbled down
}

void AlgoWFD_T2::sortBins()
{
    // The measure of all bins have been updated, need to re-order the whole list
    stable_sort(bins.begin(), bins.end(), bin_comparator_measure_decreasing);
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWF::AlgoWF(const std::string& algo_name, const Instance &instance,
               const COMBINATION combination, const WEIGHT weight):
    AlgoWFD_T2(algo_name, instance, combination, weight, false)
{
    is_FFD_type = false;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoFFD_Lexico::AlgoFFD_Lexico(const std::string &algo_name, const Instance &instance):
    AlgoFit(algo_name, instance)
{
    is_FFD_type = true;
}

void AlgoFFD_Lexico::sortItems(ItemList::iterator first_item, ItemList::iterator end_it)
{
    stable_sort(first_item, end_it, item_comparator_lexicographic_decreasing);
}

/* ================================================ */
AlgoBFD_Lexico::AlgoBFD_Lexico(const std::string &algo_name, const Instance &instance):
    AlgoFFD_Lexico(algo_name, instance)
{
    is_BF_type = true;
}

void AlgoBFD_Lexico::sortBins()
{
    // Only one bin has changed, need to bubble it to the front of the list
    bubble_bin_up(bins.begin(), bins.end(), bin_comparator_lexicographic_increasing);
}

/* ================================================ */
AlgoWFD_Lexico::AlgoWFD_Lexico(const std::string &algo_name, const Instance &instance):
    AlgoFFD_Lexico(algo_name, instance)
{
    is_BF_type = true;
    create_bins_at_end = false; // Necessary when bins are bubbled down
}

void AlgoWFD_Lexico::sortBins()
{
    // Only one bin has changed, need to bubble it to the back of the list
    bubble_bin_down(bins.begin(), bins.end(), bin_comparator_lexicographic_decreasing);
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoFFD_Rank::AlgoFFD_Rank(const std::string &algo_name, const Instance &instance,
                           const bool dynamic_items):
    AlgoFit(algo_name, instance)
{
    is_FFD_type = true;
    is_FFD_dynamic = dynamic_items;
}

void AlgoFFD_Rank::sortItems(ItemList::iterator first_item, ItemList::iterator end_it)
{
    //computeItemRanks(first_item, end_it);
    stable_sort(first_item, end_it, item_comparator_measure_decreasing);
}

void AlgoFFD_Rank::computeItemMeasures(ItemList::iterator first_item, ItemList::iterator end_it)
{
    // Reset all item measures to 0
    for(auto item_it = first_item; item_it != end_it; ++item_it)
    {
        (*item_it)->setMeasure(0);
    }

    for (int h = 0; h < dimensions; h++)
    {
        // Sort items in increasing order of size in dimension h
        auto compare_method = [h](Item* itema, Item* itemb) {
            return item_comparator_size_dim_increasing(itema, itemb, h);
        };
        stable_sort(first_item, end_it, compare_method);

        // Then add to each item measure its rank in dimension h
        int rank = 0;
        for(auto item_it = first_item; item_it != end_it; ++item_it)
        {
            (*item_it)->setMeasure((*item_it)->getMeasure() + rank);
            rank++;
        }
    }
}

/* ================================================ */
AlgoBFD_Rank::AlgoBFD_Rank(const std::string &algo_name, const Instance &instance,
                           const bool dynamic_items):
    AlgoFFD_Rank(algo_name, instance, dynamic_items)
{
    is_BF_type = true;
}

void AlgoBFD_Rank::sortBins()
{
    computeBinRanks();
    // The aggregated rank of all bins may have changed, need to re-order the whole list
    stable_sort(bins.begin(), bins.end(), bin_comparator_measure_increasing);
}

void AlgoBFD_Rank::computeBinRanks()
{
    // Reset all bin measures to 0
    for(Bin* bin : bins)
    {
        bin->setMeasure(0);
    }

    for (int h = 0; h < dimensions; h++)
    {
        // Sort bins in increasing order of residual capacity in dimension h
        auto compare_method = [h](Bin* bina, Bin* binb) {
            return bin_comparator_capacity_dim_increasing(bina, binb, h);
        };
        stable_sort(bins.begin(), bins.end(), compare_method);

        // Then add to each bin measure its rank in dimension h
        int rank = 0;
        for(auto bin_it = bins.begin(); bin_it != bins.end(); ++bin_it)
        {
            (*bin_it)->setMeasure((*bin_it)->getMeasure() + rank);
            rank++;
        }
    }
}

/* ================================================ */
AlgoWFD_Rank::AlgoWFD_Rank(const std::string &algo_name, const Instance &instance,
                           const bool dynamic_items):
    AlgoBFD_Rank(algo_name, instance, dynamic_items)
{ }

void AlgoWFD_Rank::sortBins()
{
    computeBinRanks();
    // The aggregated rank of all bins may have changed, need to re-order the whole list
    stable_sort(bins.begin(), bins.end(), bin_comparator_measure_decreasing);
}





