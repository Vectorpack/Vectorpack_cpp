#include "algos_BinCentric.hpp"

#include <stdexcept> // For throwing stuff
#include <cmath>
#include <limits> // For infinity value

#include <iostream> // TODO remove, for fast debugging

AlgoBinCentric::AlgoBinCentric(const std::string &algo_name, const Instance &instance,
                               const SCORE score, const WEIGHT weight,
                               const bool dynamic_weights,
                               const bool use_bin_weights):
    BaseAlgo(algo_name, instance),
    weight(weight),
    score(score),
    dynamic_weights(dynamic_weights),
    weights_list(FloatList(instance.getDimensions(), 1.0)),
    is_ratio_weight(false),
    use_bin_weights(use_bin_weights)
{
    // Compute total normalized size of all items
    total_norm_size = FloatList(dimensions, 0.0);
    for(Item * item : items)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] += item->getNormSizeDim(h);
        }
    }

    if ((weight == WEIGHT::RESIDUAL_RATIO) or (weight == WEIGHT::UTILISATION_RATIO))
    {
        is_ratio_weight = true;
    }

    if (is_ratio_weight or use_bin_weights)
    {
        // So that weights are re-computed after each packing of an item
        this->dynamic_weights = true;
    }

    if ((score == SCORE::NORM_DOT_PRODUCT) or this->dynamic_weights)
    {
        // Will contain normalized values
        total_norm_residual_capacity = FloatList(dimensions, 0.0);
    }

    if (score == SCORE::DOT_PRODUCT2)
    {
        // Initialize the inverse of Norm2 of items in their measure placeholder
        // to ease computation of scaling factor for DP2 scores
        for (Item * item : items)
        {
            item->setMeasure(1.0 / utilComputeNorm2(item->getSizes()));
        }
    }
}

Bin* AlgoBinCentric::createNewBin()
{
    Bin* bin = BaseAlgo::createNewBin();

    if ((score == SCORE::NORM_DOT_PRODUCT) or dynamic_weights)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_residual_capacity[h] += 1.0; // The normalized residual capacity of an empty bin is 1
        }
    }
    if ((score == SCORE::DOT_PRODUCT2) or (score == SCORE::DOT_PRODUCT3))
    {
        // Set the norm2 of residual capacity of the bin
        bin->setMeasure(utilComputeNorm2(bin->getAvailableCaps(), bin->getMaxCaps()));
    }

    return bin;
}

void AlgoBinCentric::addItemToBin(Item *item, Bin *bin)
{
    bin->addItem(item);

    if ((score == SCORE::NORM_DOT_PRODUCT) or dynamic_weights)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_residual_capacity[h] -= item->getNormSizeDim(h);
        }
    }
    if ((score == SCORE::DOT_PRODUCT2) or (score == SCORE::DOT_PRODUCT3))
    {
        // Update norm2 of the bin
        bin->setMeasure(utilComputeNorm2(bin->getAvailableCaps(), bin->getMaxCaps()));
    }

    if (dynamic_weights)
    {
        // Update the total normalized size for dynamic weights
        for (int h = 0; h < dimensions; ++h)
        {
            total_norm_size[h] -= item->getNormSizeDim(h);
        }
    }
}


float AlgoBinCentric::computeItemBinScore(Item* item, Bin* bin)
{
    float score_value = 0.0;
    switch(score)
    {
    case SCORE::DOT_PRODUCT1:
        for (int h = 0; h < dimensions; ++h)
        {
            // Use normalized values of item size and bin residual capacity
            score_value += weights_list[h] * item->getNormSizeDim(h) * bin->getAvailableCapDim(h) / bin_max_capacities[h];
        }
        break;
    case SCORE::DOT_PRODUCT2:
        for (int h = 0; h < dimensions; ++h)
        {
            // Use normalized values of item size and bin residual capacity
            score_value += weights_list[h] * item->getNormSizeDim(h) * bin->getAvailableCapDim(h) / bin_max_capacities[h];
        }
        // Scaling factor 1/(norm2 item size * norm2 bin capacity)
        // item measure holds 1/norm2 size; bin measure holds norm2 bin capacity
        score_value = score_value * item->getMeasure() / bin->getMeasure();
        break;
    case SCORE::DOT_PRODUCT3:
        for (int h = 0; h < dimensions; ++h)
        {
            // Use normalized values of item size and bin residual capacity
            score_value += weights_list[h] * item->getNormSizeDim(h) * bin->getAvailableCapDim(h) / bin_max_capacities[h];
        }
        // Scaling factor 1/(norm2 bin capacity * norm2 bin capacity)
        // bin measure holds norm2 bin capacity
        score_value = score_value / (bin->getMeasure() * bin->getMeasure());
        break;
    case SCORE::NORM_DOT_PRODUCT:
        for (int h = 0; h < dimensions; ++h)
        {
            if ((total_norm_size[h] > ZERO_THRESHOLD) and (total_norm_residual_capacity[h] > ZERO_THRESHOLD))
            {
                score_value += weights_list[h] * (item->getNormSizeDim(h) * bin->getAvailableCapDim(h))/ (total_norm_size[h] * bin_max_capacities[h] * total_norm_residual_capacity[h]);
            }
            // Else, desactivate that dimension for the computation of score
        }
        break;
    case SCORE::L2NORM:
        for (int h = 0; h < dimensions; ++h)
        {
            float f = (bin->getAvailableCapDim(h) / bin_max_capacities[h]) - item->getNormSizeDim(h);
            score_value -= weights_list[h] * f*f;
        }
        break;
    case SCORE::TIGHT_FILL_SUM:
        for (int h = 0; h < dimensions; ++h)
        {
            // Use normalized values of item size and bin residual capacity
            score_value += weights_list[h] * item->getNormSizeDim(h) * bin_max_capacities[h] / bin->getAvailableCapDim(h);
        }
        break;
    case SCORE::TIGHT_FILL_MIN:
        {
            bool inited = false;
            score_value = std::numeric_limits<float>::max(); // +infinity

            for (int h = 1; h < dimensions; ++h)
            {
                // Use normalized values of item size and bin residual capacity
                if (weights_list[h] != 0.0)
                {
                    score_value = std::min(score_value, weights_list[h] * item->getNormSizeDim(h) * bin_max_capacities[h] / bin->getAvailableCapDim(h));
                    inited = true;
                }
                // Otherwise, the dimension has been deactivated by setting its weight to 0
            }

            if (!inited)
            {
                score_value = 0; // If all dimensions were deactivated, set the score to 0
            }
        }
        break;
    }

    return score_value;
}

int AlgoBinCentric::solveInstance(int hint_nb_bins)
{
    if(isSolved())
    {
        return getSolution(); // No need to solve it twice
    }
    if (hint_nb_bins > 0)
    {
        bins.reserve(hint_nb_bins);
    }

    int total_items = instance.getNbItems();
    Bin* curr_bin = createNewBin();
    auto first_item_it = items.begin();
    auto end_items_it = items.end();
    int remaining_items = end_items_it - first_item_it;

    if (!dynamic_weights)
    {
        // Compute weights only once
        if (is_ratio_weight)
        {
            // This should never be reached, as ratio weights are dynamic
            utilComputeWeightsRatio(weight, dimensions, weights_list, total_norm_size, total_norm_residual_capacity);
        }
        else
        {
            if (use_bin_weights)
            {
                // This should never be reached, as bin-based weights are dynamic
                utilComputeWeights(weight, dimensions, bins.size(), weights_list, total_norm_residual_capacity);
            }
            else
            {
                utilComputeWeights(weight, dimensions, remaining_items, weights_list, total_norm_size);
            }
        }
    } // Otherwise, weights will be re-computed before computing scores

    while(first_item_it != end_items_it) // While there are items to pack
    {
        // Compute maximum score and get the item
        auto max_score_it = end_items_it;
        float max_score_val = std::numeric_limits<float>::lowest(); // -infinity

        if (dynamic_weights)
        {
            // Update weights
            if (is_ratio_weight)
            {
                utilComputeWeightsRatio(weight, dimensions, weights_list, total_norm_size, total_norm_residual_capacity);
            }
            else
            {
                if (use_bin_weights)
                {
                    // This should never be reached, as bin-based weights are dynamic
                    utilComputeWeights(weight, dimensions, bins.size(), weights_list, total_norm_residual_capacity);
                }
                else
                {
                    utilComputeWeights(weight, dimensions, remaining_items, weights_list, total_norm_size);
                }
            }
        }

        // For each item, if it is feasible, compute its score
        for(auto curr_item_it = first_item_it; curr_item_it != end_items_it; ++curr_item_it)
        {
            if (curr_bin->doesItemFit((*curr_item_it)->getSizes()))
            {
                float score = computeItemBinScore((*curr_item_it), curr_bin);
                if (score > max_score_val)
                {
                    max_score_val = score;
                    max_score_it = curr_item_it;
                }
            }
        }

        if (max_score_it != end_items_it)
        {
            // There is a feasible item
            addItemToBin(*max_score_it, curr_bin);
            --remaining_items;

            // Put this item at beginning of the list and advance the first iterator
            std::iter_swap(max_score_it, first_item_it);
            first_item_it++;
        }
        else
        {
            // There is no feasible item, create a new bin
            curr_bin = createNewBin();

            // This is a quick safe guard to avoid infinite loops and running out of memory
            if (bins.size() > total_items)
            {
                std::string s = "There seem to be a problem with algo " + name + " and instance " + instance.getName() + ", created more bins than items (" + std::to_string(bins.size()) + ").";
                throw std::runtime_error(s);
            }
        }
    }

    solved = true;
    return getSolution();
}


int AlgoBinCentric::solveInstanceMultiBin(int LB, int UB)
{
    std::string s = "With BinCentric-type algorithm please call 'solveInstance' instead.";
    throw std::runtime_error(s);
}
