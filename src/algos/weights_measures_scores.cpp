#include "weights_measures_scores.hpp" 

#include <cmath> // For exp

void utilComputeWeights(const WEIGHT weight,
                    const int dimensions,
                    const int n,
                    FloatList& weights_list,
                    const FloatList& total_norm_size)
{
    // Initialize the vector of weights for computing the combined size measures
    if (weight != WEIGHT::UNIT)
    {
        switch(weight)
        {
        case WEIGHT::AVERAGE:
            for (int h = 0; h < dimensions; ++h)
            {
                weights_list[h] = total_norm_size[h];
            }
            break;
        case WEIGHT::EXPONENTIAL:
            for (int h=0; h < dimensions; ++h)
            {
                weights_list[h] = std::exp(0.01 * total_norm_size[h] / n);
            }
            break;
        case WEIGHT::DIVIDED_AVERAGE:
            for (int h = 0; h < dimensions; ++h)
            {
                if (total_norm_size[h] <= ZERO_THRESHOLD)
                {
                    // If the total size of items is "0" in that dimension, desactivate the dimension
                    // by setting the weight to 0 (+ avoids a division by 0 or weight of value close to +infinity)
                    weights_list[h] = 0.0;
                }
                else
                {
                    weights_list[h] = 1.0 / total_norm_size[h]; // It's the inverse of AVERAGE weight
                }
            }
            break;
        }
    }
}

void utilComputeWeightsRatio(const WEIGHT weight,
                             const int dimensions,
                             FloatList& weights_list,
                             const FloatList& total_norm_size,
                             const FloatList& total_norm_residual_capacity)
{
    // If total residual capacity of the bins is "0" in a dimension, desactivate the dimension
    // by setting the weight to 0 (+ avoids a division by 0 or weight of value close to +infinity)
    if (weight == WEIGHT::RESIDUAL_RATIO)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            if (total_norm_residual_capacity[h] <= ZERO_THRESHOLD)
            {
                weights_list[h] = 0.0;
            }
            else
            {
                weights_list[h] = 1.0 / total_norm_residual_capacity[h];
            }
        }
    }
    else if (weight == WEIGHT::UTILISATION_RATIO)
    {
        for (int h = 0; h < dimensions; ++h)
        {
            if (total_norm_residual_capacity[h] <= ZERO_THRESHOLD)
            {
                weights_list[h] = 0.0;
            }
            else
            {
                weights_list[h] = total_norm_size[h] / total_norm_residual_capacity[h];
            }
        }
    }
}

float utilComputeNorm2(const SizeList &list)
{
    if (list.size() > 1)
    {
        float val = 0.0;
        for (auto &i : list)
        {
            val += i*i;
        }
        return std::sqrt(val);
    }
    else
    {
        // Only one dimension
        return list[0];
    }
}

float utilComputeNorm2(const SizeList &list, const SizeList &normalization_list)
{
    if (list.size() > 1)
    {
        float val = 0.0;
        for (int i = 0; i < list.size(); i++)
        {
            float norm_val = ((float)list[i]) / normalization_list[i];
            val += norm_val * norm_val;
        }
        return std::sqrt(val);
    }
    else
    {
        // Only one dimension
        return ((float)list[0] / normalization_list[0]);
    }
}
