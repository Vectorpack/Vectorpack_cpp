#include "algos_MultiBin.hpp"

#include <stdexcept> // For throwing stuff
#include <cmath>
#include <algorithm> // For stable_sort


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoPairing::AlgoPairing(const std::string &algo_name, const Instance &instance,
                         const SCORE score, const WEIGHT weight,
                         const bool dynamic_weights,
                         const bool use_bin_weights):
    AlgoBinCentric(algo_name, instance, score, weight, dynamic_weights, use_bin_weights)
{
    if ((score == SCORE::NORM_DOT_PRODUCT) or is_ratio_weight or this->use_bin_weights)
    {
        // Scores need to be re-computed every time
        store_scores = false;
    }
    else
    {
        // Scores can be lazilly updated only for the last bin that changed
        store_scores = true;
    }
}

int AlgoPairing::solveInstance(int hint_nb_bins)
{
    std::string s = "With AlgoPairing-type algorithm please call 'solveInstanceMultiBin' instead.";
    throw std::runtime_error(s);
}

void AlgoPairing::createNewBins(int nb_bins)
{
    bins.reserve(bins.size() + nb_bins);
    for (int i = 0; i < nb_bins; ++i)
    {
        createNewBin();
    }
}

void AlgoPairing::updateScores(Bin* bin, ItemList::iterator first_item, ItemList::iterator end_it)
{
    std::vector<float>& item_scores = bin_item_scores[bin->getId()];
    for (auto item_it = first_item; item_it != end_it; item_it++)
    {
        item_scores[(*item_it)->getId()] = computeItemBinScore(*item_it, bin);
    }
}

void AlgoPairing::resetAlgo()
{
    clearSolution();

    total_norm_size.clear();
    total_norm_size = FloatList(dimensions, 0.0);

    for(Item * item : items)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] += item->getNormSizeDim(h);
        }
    }

    if ((score == SCORE::NORM_DOT_PRODUCT) or is_ratio_weight or use_bin_weights)
    {
        total_norm_residual_capacity.clear();
        total_norm_residual_capacity = FloatList(dimensions, 0.0);
    }
}


bool AlgoPairing::trySolve(int nb_bins)
{
    if (nb_bins < 0)
    {
        std::string s("Trying to solve instance with negative number of bins");
        throw std::runtime_error(s);
    }

    resetAlgo();
    createNewBins(nb_bins);

    // Initialize weights, need to be computed after all bins were created
    int nb_items = items.size();
    if (is_ratio_weight)
    {
        utilComputeWeightsRatio(weight, dimensions, weights_list, total_norm_size, total_norm_residual_capacity);
    }
    else
    {
        if (use_bin_weights)
        {
            utilComputeWeights(weight, dimensions, bins.size(), weights_list, total_norm_residual_capacity);
        }
        else
        {
            utilComputeWeights(weight, dimensions, nb_items, weights_list, total_norm_size);
        }
    }

    if (store_scores)
    {
        // Initialize the score for each item-bin pair
        bin_item_scores.clear();
        bin_item_scores.resize(nb_bins, std::vector<float>(nb_items, 0.0));
        for (Bin* bin : bins)
        {
            updateScores(bin, items.begin(), items.end());
        }
    }

    first_remaining_item = items.begin();
    return packItems(bins.begin());
}


bool AlgoPairing::packItems(BinList::iterator start_bin_it)
{
    auto first_item_it = first_remaining_item; // TODO probably don't need this new variable
    auto end_items_it = items.end();
    auto end_bins_it = bins.end();
    int remaining_items = end_items_it - first_item_it;

    while(first_item_it != end_items_it) // While there are items to pack
    {
        // Compute maximum score for each pair of item-bin
        auto max_score_item_it = end_items_it;
        auto max_score_bin_it = end_bins_it;
        float max_score_val = std::numeric_limits<double>::lowest(); // -infinity

        // For each remaining item
        for(auto curr_item_it = first_item_it; curr_item_it != end_items_it; ++curr_item_it)
        {
            // For each bin
            for(auto curr_bin_it = start_bin_it; curr_bin_it != end_bins_it; ++curr_bin_it)
            {
                if ((*curr_bin_it)->doesItemFit((*curr_item_it)->getSizes()))
                {
                    float score;
                    if (!store_scores)
                    {
                        // Need to recompute all scores
                        score = computeItemBinScore((*curr_item_it), (*curr_bin_it));
                    }
                    else
                    {
                        // Scores were computed previously
                        score = bin_item_scores[(*curr_bin_it)->getId()][(*curr_item_it)->getId()];
                    }

                    if (score > max_score_val)
                    {
                        max_score_val = score;
                        max_score_item_it = curr_item_it;
                        max_score_bin_it = curr_bin_it;
                    }
                }
            }
        }

        if (max_score_item_it != end_items_it)
        {
            // There is a feasible item-bin pair
            addItemToBin(*max_score_item_it, *max_score_bin_it);
            --remaining_items;

            // Put this item at beginning of the list and advance the first iterator
            std::iter_swap(max_score_item_it, first_item_it);
            first_item_it++;

            if (dynamic_weights)
            {
                // Update the weights after packing that item
                if (use_bin_weights)
                {
                    utilComputeWeights(weight, dimensions, bins.size(), weights_list, total_norm_residual_capacity);
                }
                else
                {
                    utilComputeWeights(weight, dimensions, remaining_items, weights_list, total_norm_size);
                }
            }

            if (store_scores)
            {
                // Update score of remaining items for that bin
                updateScores(*max_score_bin_it, first_item_it, end_items_it);
            }
        }
        else
        {
            // There is no feasible item-bin pair, the solution is infeasible
            // Update the iterator on first remaining item before exitting
            first_remaining_item = first_item_it;
            return false;
        }
    }

    first_remaining_item = end_items_it;
    return true;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoPairing_Increment::AlgoPairing_Increment(
                    const std::string &algo_name, const Instance &instance,
                    const SCORE score, const WEIGHT weight,
                    const bool dynamic_weights,
                    const bool use_bin_weights,
                    const int bin_increment_percent):
    AlgoPairing(algo_name, instance, score, weight, dynamic_weights, use_bin_weights),
    bin_increment_percent(bin_increment_percent)
{ }

int AlgoPairing_Increment::solveInstanceMultiBin(int LB, int UB)
{
    int bin_increment = 1; // Default
    if (bin_increment_percent > 0)
    {
        bin_increment = std::floor( (UB - LB) * bin_increment_percent / 100.0);

        if (bin_increment < 1)
        {
            bin_increment = 1;
        }
    }

    int target_bins = LB;
    bool sol_found = trySolve(target_bins);
    bool last_try = false;
    while (!sol_found and !last_try)
    {
        // There are remaining items to pack
        // But no bin can accommodate an item anymore

        // Increment the number of bins
        target_bins += bin_increment;
        if (target_bins >= UB)
        {
            // Try with UB as a last resort
            target_bins = UB;
            last_try = true;
        }

        sol_found = trySolve(target_bins);
    }

    int answer = target_bins;
    if (!sol_found)
    {
        answer = -1;
    }
    return answer;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoPairing_BinSearch::AlgoPairing_BinSearch(
                    const std::string &algo_name, const Instance &instance,
                    const SCORE score, const WEIGHT weight,
                    const bool dynamic_weights,
                    const bool use_bin_weights):
    AlgoPairing(algo_name, instance, score, weight, dynamic_weights, use_bin_weights)
{ }

void AlgoPairing_BinSearch::updateBestBins(const BinList &new_bins)
{
    for (Bin* bin : best_bins)
    {
        if (bin != nullptr)
        {
            delete bin;
        }
    }
    best_bins.clear();
    best_bins = new_bins;
}

// Implements true Binary Search
int AlgoPairing_BinSearch::solveInstanceMultiBin(int LB, int UB)
{
    // First, try to find a solution with UB
    if (!trySolve(UB))
    {
        // If no solution found, no need to continue the search
        return -1;
    }

    // A solution with UB was found, store the current solution
    updateBestBins(getBinsCopy());
    int target_bins;

    // Then iteratively try to improve on the solution
    while (LB < UB)
    {
        target_bins = std::floor((float)(LB + UB) / 2.0);

        if (trySolve(target_bins))
        {
            // Better solution found, update UB
            UB = target_bins;
            updateBestBins(getBinsCopy());
        }
        else
        {
            // Target too low, update LB
            LB = target_bins + 1; // +1 to keep a potential feasible solution with LB
        }
    }

    setSolution(best_bins);
    return UB;
}


/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWFDm::AlgoWFDm(const std::string &algo_name, const Instance &instance,
                       const MEASURE measure, const WEIGHT weight,
                       const bool dynamic_weights):
    AlgoWFD_T1(algo_name, instance, measure, weight, dynamic_weights)
{
    create_bins_at_end = true; // Was set to false for regular WFD algorithm
    // but we don't need this as all bins are created at the init stage in multi-bin algorithms
}


int AlgoWFDm::solveInstance(int hint_nb_bins)
{
    std::string s = "With AlgoWFDm-type algorithm please call 'solveInstanceMultiBin' instead.";
    throw std::runtime_error(s);
}

void AlgoWFDm::createNewBins(int nb_bins)
{
    bins.reserve(bins.size() + nb_bins);
    for (int i = 0; i < nb_bins; ++i)
    {
        createNewBin();
    }
}


void AlgoWFDm::resetAlgo()
{
    clearSolution();

    total_norm_size.clear();
    total_norm_size = FloatList(dimensions, 0.0);

    for(Item * item : items)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] += item->getNormSizeDim(h);
        }
    }

    if (is_ratio_weight)
    {
        total_norm_residual_capacity.clear();
        total_norm_residual_capacity = FloatList(dimensions, 0.0);
    }
}

bool AlgoWFDm::trySolve(int nb_bins)
{
    if (nb_bins < 0)
    {
        std::string s("Trying to solve instance with negative number of bins");
        throw std::runtime_error(s);
    }

    resetAlgo();
    createNewBins(nb_bins);

    if (is_FFD_type)
    {
        // Weights are computed as well
        computeItemMeasures(items.begin(), items.end());
        sortItems(items.begin(), items.end());
    }
    else
    {
        // Need to initialize weights anyway, for bin measures
        int nb_items = items.size();
        if (is_ratio_weight)
        {
            utilComputeWeightsRatio(weight, dimensions, weights_list, total_norm_size, total_norm_residual_capacity);
        }
        else
        {
            utilComputeWeights(weight, dimensions, nb_items, weights_list, total_norm_size);
        }
    }

    // Initialize measure of all bins
    for (Bin* bin : bins)
    {
        updateBinMeasure(bin);
    }

    first_remaining_item = items.begin();
    return packItems(bins.begin());
}


bool AlgoWFDm::packItems(BinList::iterator start_bin_it)
{
    bool allocated = false;
    auto curr_item_it = first_remaining_item;
    auto end_items_it = items.end();
    while(curr_item_it != end_items_it)
    {
        Item * item = *curr_item_it;

        auto curr_bin_it = start_bin_it;
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
            // The item cannot be packed, unfeasible instance
            first_remaining_item = curr_item_it;
            return false;
        }

        // Advance next item to pack and re-order the bins
        ++curr_item_it;

        // Update weights, item measures and order of remaining items
        if (is_FFD_dynamic)
        {
            computeItemMeasures(curr_item_it, end_items_it);
            sortItems(curr_item_it, end_items_it);
        }

        sortBins(start_bin_it, bins.end());
    }

    first_remaining_item = end_items_it;
    return true;
}


void AlgoWFDm::sortBins(BinList::iterator first_bin, BinList::iterator last_bin)
{
    if (is_FFD_dynamic)
    {
        // The item weights have changed, need to re-compute all bin measures
        for (auto bin_it = first_bin; bin_it != last_bin; bin_it++)
        {
            updateBinMeasure(*bin_it);
        }

        // Then re-order the whole list of bins
        stable_sort(first_bin, last_bin, bin_comparator_measure_decreasing);
    }
    else
    {
        // The measure of only one bin has changed, no need to perform complete sort
        // but the bubbling in BOTH directions is required, as it depends on the size measures
        bubble_bin_down(first_bin, last_bin, bin_comparator_measure_decreasing);
        bubble_bin_up(first_bin, last_bin, bin_comparator_measure_increasing);
    }
}



/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWFDm_Increment::AlgoWFDm_Increment(const std::string &algo_name, const Instance &instance,
                       const MEASURE measure, const WEIGHT weight,
                       const bool dynamic_weights,
                       const int bin_increment_percent):
    AlgoWFDm(algo_name, instance, measure, weight, dynamic_weights),
    bin_increment_percent(bin_increment_percent)
{ }


int AlgoWFDm_Increment::solveInstanceMultiBin(int LB, int UB)
{
    //!\\
    //!\\ This should be a strict copy of the same method of AlgoPairing_Increment
    //!\\

    int bin_increment = 1; // Default
    if (bin_increment_percent > 0)
    {
        bin_increment = std::floor( (UB - LB) * bin_increment_percent / 100.0);

        if (bin_increment < 1)
        {
            bin_increment = 1;
        }
    }

    int target_bins = LB;
    bool sol_found = trySolve(target_bins);
    bool last_try = false;
    while (!sol_found and !last_try)
    {
        // There are remaining items to pack
        // But no bin can accommodate an item anymore

        // Increment the number of bins
        target_bins += bin_increment;
        if (target_bins >= UB)
        {
            // Try with UB as a last resort
            target_bins = UB;
            last_try = true;
        }

        sol_found = trySolve(target_bins);
    }

    int answer = target_bins;
    if (!sol_found)
    {
        answer = -1;
    }
    return answer;
}

/* ================================================ */
/* ================================================ */
/* ================================================ */
AlgoWFDm_BinSearch::AlgoWFDm_BinSearch(const std::string &algo_name, const Instance &instance,
                       const MEASURE measure, const WEIGHT weight,
                       const bool dynamic_weights):
    AlgoWFDm(algo_name, instance, measure, weight, dynamic_weights)
{ }


void AlgoWFDm_BinSearch::updateBestBins(const BinList &new_bins)
{
    for (Bin* bin : best_bins)
    {
        if (bin != nullptr)
        {
            delete bin;
        }
    }
    best_bins.clear();
    best_bins = new_bins;
}


// Implements true Binary Search
int AlgoWFDm_BinSearch::solveInstanceMultiBin(int LB, int UB)
{
    //!\\
    //!\\ This should be a strict copy of the same method of AlgoPairing_BinSearch
    //!\\

    // First, try to find a solution with UB
    if (!trySolve(UB))
    {
        // If no solution found, no need to continue the search
        return -1;
    }

    // A solution with UB was found, store the current solution
    updateBestBins(getBinsCopy());
    int target_bins;

    // Then iteratively try to improve on the solution
    while (LB < UB)
    {
        target_bins = std::floor((float)(LB + UB) / 2.0);

        if (trySolve(target_bins))
        {
            // Better solution found, update UB
            UB = target_bins;
            updateBestBins(getBinsCopy());
        }
        else
        {
            // Target too low, update LB
            LB = target_bins + 1; // +1 to keep a potential feasible solution with LB
        }
    }

    setSolution(best_bins);
    return UB;
}


AlgoBFDm_Increment::AlgoBFDm_Increment(const std::string &algo_name, const Instance &instance,
                       const MEASURE measure, const WEIGHT weight,
                       const bool dynamic_weights,
                       const int bin_increment_percent):
    AlgoWFDm_Increment(algo_name, instance,
                       measure, weight,
                       dynamic_weights,
                       bin_increment_percent)
{ }

void AlgoBFDm_Increment::sortBins(BinList::iterator first_bin, BinList::iterator last_bin)
{
    if (is_FFD_dynamic)
    {
        // The item weights have changed, need to re-compute all bin measures
        for (auto bin_it = first_bin; bin_it != last_bin; bin_it++)
        {
            updateBinMeasure(*bin_it);
        }

        // Then re-order the whole list of bins
        stable_sort(first_bin, last_bin, bin_comparator_measure_increasing);
    }
    else
    {
        // The measure of only one bin has changed, no need to perform complete sort
        // but the bubbling in BOTH directions is required, as it depends on the size measures
        bubble_bin_up(first_bin, last_bin, bin_comparator_measure_increasing);
        bubble_bin_down(first_bin, last_bin, bin_comparator_measure_decreasing);
    }
}




AlgoBFDm_BinSearch::AlgoBFDm_BinSearch(const std::string &algo_name, const Instance &instance,
                       const MEASURE measure, const WEIGHT weight,
                       const bool dynamic_weights):
    AlgoWFDm_BinSearch(algo_name, instance,
                       measure, weight,
                       dynamic_weights)
{ }

void AlgoBFDm_BinSearch::sortBins(BinList::iterator first_bin, BinList::iterator last_bin)
{
    if (is_FFD_dynamic)
    {
        // The item weights have changed, need to re-compute all bin measures
        for (auto bin_it = first_bin; bin_it != last_bin; bin_it++)
        {
            updateBinMeasure(*bin_it);
        }

        // Then re-order the whole list of bins
        stable_sort(first_bin, last_bin, bin_comparator_measure_increasing);
    }
    else
    {
        // The measure of only one bin has changed, no need to perform complete sort
        // but the bubbling in BOTH directions is required, as it depends on the size measures
        bubble_bin_up(first_bin, last_bin, bin_comparator_measure_increasing);
        bubble_bin_down(first_bin, last_bin, bin_comparator_measure_decreasing);
    }
}
