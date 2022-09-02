#include "utils.hpp"
#include "weights_measures_scores.hpp"

#include <sstream>
#include <stdexcept> // For throwing stuff

// For prioritizing dimensions
const static std::map<std::string, WEIGHT> map_str_to_weight = {
    { "Unit", WEIGHT::UNIT },
    { "Avg", WEIGHT::AVERAGE },
    { "Expo", WEIGHT::EXPONENTIAL },
    { "DivAvg", WEIGHT::DIVIDED_AVERAGE },
    { "Residual", WEIGHT::RESIDUAL_RATIO },
    { "Utilisation", WEIGHT::UTILISATION_RATIO },
};

// For ItemCentric algos
const static std::map<std::string, COMBINATION> map_str_to_combination = {
    { "Sum", COMBINATION::SUM },
    { "Max", COMBINATION::MAX },
    { "SumSq", COMBINATION::SUM_SQ },
    { "SumSqLoad", COMBINATION::SUM_SQ_LOAD },
};

// For BinCentric and Item-Bin Matching algos
const static std::map<std::string, SCORE> map_str_to_score = {
    { "DP1", SCORE::DOT_PRODUCT1 },
    { "DP2", SCORE::DOT_PRODUCT2 },
    { "DP3", SCORE::DOT_PRODUCT3 },
    { "NormDP", SCORE::NORM_DOT_PRODUCT },
    { "L2Norm", SCORE::L2NORM },
    { "TFSum", SCORE::TIGHT_FILL_SUM },
    { "TFMin", SCORE::TIGHT_FILL_MIN },
};

std::vector<std::string> splitString(const std::string & str)
{
    std::istringstream splitStr(str);
    std::vector<std::string> v;
    std::string s;
    while(std::getline(splitStr, s, '-'))
    {
        v.push_back(s);
    }
    return v;
}

BaseAlgo* createAlgoCentric(const std::string &algo_name, const Instance &instance)
{
    std::vector<std::string> v = splitString(algo_name);
    const std::string& s = v[0];

    // Item-centric algos
    if ((s == "FF") and (v.size() == 1))
    {
        return new AlgoFF(algo_name, instance);
    }
    else if ((v.size() == 2) and (v[1] == "Lexico")) // Lexicographic order
    {
        if(s == "FFD")
        {
            return new AlgoFFD_Lexico(algo_name, instance);
        }
        else if(s == "BFD")
        {
            return new AlgoBFD_Lexico(algo_name, instance);
        }
        if(s == "WFD")
        {
            return new AlgoWFD_Lexico(algo_name, instance);
        }
    }
    else if ((v.size() == 2) and (v[1] == "Rank")) // Aggregated Rank order
    {
        if(s == "FFD")
        {
            return new AlgoFFD_Rank(algo_name, instance, false);
        }
        else if(s == "BFD")
        {
            return new AlgoBFD_Rank(algo_name, instance, false);
        }
        else if(s == "WFD")
        {
            return new AlgoWFD_Rank(algo_name, instance, false);
        }
    }
    else if ((v.size() == 3) and (v[1] == "Rank") and (v[2] == "Dyn")) // Aggregated Rank order
    {
        if(s == "FFD")
        {
            return new AlgoFFD_Rank(algo_name, instance, true);
        }
        else if(s == "BFD")
        {
            return new AlgoBFD_Rank(algo_name, instance, true);
        }
        else if(s == "WFD")
        {
            return new AlgoWFD_Rank(algo_name, instance, true);
        }
    }
    else if (s == "FFD")
    {
        if (v.size() == 3)
        {
            // FFD with static weights
            return new AlgoFFD(algo_name, instance,
                               map_str_to_combination.at(v[1]),
                               map_str_to_weight.at(v[2]),
                               false);
        }
        else if ((v.size() == 4) and (v[3] == "Dyn"))
        {
            // FFD with dynamic weights
            return new AlgoFFD(algo_name, instance,
                               map_str_to_combination.at(v[1]),
                               map_str_to_weight.at(v[2]),
                               true);
        }
        else if ((v.size() == 4) and (v[3] == "Ratio"))
        {
            // FFD with ratio weights
            return new AlgoFFD(algo_name, instance,
                               map_str_to_combination.at(v[1]),
                               map_str_to_weight.at(v[2]),
                               true);
        }
    }
    else if (s == "BFD")
    {
        if (v[1] == "T1")
        {
            if (v.size() == 4)
            {
                // BFD Type 1 with static item weights
                return new AlgoBFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                // BFD Type 1 with dynamic item weights
                return new AlgoBFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
            else if ((v.size() == 5) and (v[4] == "Ratio"))
            {
                // BFD Type 1 with ratio weights
                return new AlgoBFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
        }
        else if (v[1] == "T2")
        {
            if (v.size() == 4)
            {
                return new AlgoBFD_T2(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                return new AlgoBFD_T2(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
        }
        else if (v[1] == "T3")
        {
            if (v.size() == 5)
            {
                return new AlgoBFD_T3(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      map_str_to_weight.at(v[4]),
                                      false);
            }
            else if ((v.size() == 6) and (v[5] == "Dyn"))
            {
                return new AlgoBFD_T3(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      map_str_to_weight.at(v[4]),
                                      true);
            }
        }
    }
    else if (s=="BF")
    {
        if (v.size() == 3)
        {
            return new AlgoBF(algo_name, instance,
                                 map_str_to_combination.at(v[1]),
                                 map_str_to_weight.at(v[2]));
        }
        else if ((v.size() == 4) and (v[3] == "Ratio"))
        {
            return new AlgoBF(algo_name, instance,
                                 map_str_to_combination.at(v[1]),
                                 map_str_to_weight.at(v[2]));
        }
    }
    else if (s == "WFD")
    {
        if (v[1] == "T1")
        {
            if (v.size() == 4)
            {
                // WFD Type 1 with static item weights
                return new AlgoWFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                // WFD Type 1 with dynamic item weights
                return new AlgoWFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
            else if ((v.size() == 5) and (v[4] == "Ratio"))
            {
                // WFD Type 1 with ratio weights
                return new AlgoWFD_T1(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
        }
        else if (v[1] == "T2")
        {
            if (v.size() == 4)
            {
                return new AlgoWFD_T2(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                return new AlgoWFD_T2(algo_name, instance,
                                      map_str_to_combination.at(v[2]),
                                      map_str_to_weight.at(v[3]),
                                      true);
            }
        }
    }
    else if (s=="WF")
    {
        if (v.size() == 3)
        {
            return new AlgoWF(algo_name, instance,
                                 map_str_to_combination.at(v[1]),
                                 map_str_to_weight.at(v[2]));
        }
        else if ((v.size() == 4) and (v[3] == "Ratio"))
        {
            return new AlgoWF(algo_name, instance,
                                 map_str_to_combination.at(v[1]),
                                 map_str_to_weight.at(v[2]));
        }
    }
    else if(s == "BCS") // Bin-centric with Score algos
    {
        if(v.size() == 3)
        {
            // with static weights
            return new AlgoBinCentric(algo_name, instance,
                                      map_str_to_score.at(v[1]),
                                      map_str_to_weight.at(v[2]),
                                      false,
                                      false);
        }
        else if ((v.size() == 4) and (v[3] == "Dyn"))
        {
            // with dynamic weights
            return new AlgoBinCentric(algo_name, instance,
                                      map_str_to_score.at(v[1]),
                                      map_str_to_weight.at(v[2]),
                                      true,
                                      false);
        }
        else if ((v.size() == 4) and (v[3] == "Ratio"))
        {
            // with dynamic Ratio weights
            return new AlgoBinCentric(algo_name, instance,
                                      map_str_to_score.at(v[1]),
                                      map_str_to_weight.at(v[2]),
                                      true,
                                      false);
        }
        else if ((v.size() == 4) and (v[3] == "Bin"))
        {
            // with dynamic Ratio weights
            return new AlgoBinCentric(algo_name, instance,
                                      map_str_to_score.at(v[1]),
                                      map_str_to_weight.at(v[2]),
                                      true,
                                      true);
        }
    }

    std::string err_string = "Could not create algo \'" + algo_name + "\'";
    throw std::runtime_error(err_string);
}


AlgoPairing* createAlgoPairing(const std::string& algo_name, const Instance &instance)
{
    std::vector<std::string> v = splitString(algo_name);
    const std::string& s = v[1];

    if (v[0] == "BIM")
    {
        if (s == "BS")
        {
            // Bin-Item Pairing with Relaxed Binary Search
            if (v.size() == 4)
            {
                return new AlgoPairing_BinSearch(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false, false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                return new AlgoPairing_BinSearch(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, false);
            }
            else if ((v.size() == 5) and (v[4] == "Ratio"))
            {
                return new AlgoPairing_BinSearch(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, false);
            }
            else if ((v.size() == 5) and (v[4] == "Bin"))
            {
                return new AlgoPairing_BinSearch(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, true);
            }
        }
        else if (s == "Incr")
        {
            // Bin-Item Pairing with increment bins
            if (v.size() == 5)
            {
                return new AlgoPairing_Increment(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false, false,
                                        std::stoi(v[4]));
            }
            else if ((v.size() == 6) and (v[4] == "Dyn"))
            {
                return new AlgoPairing_Increment(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, false,
                                        std::stoi(v[5]));
            }
            else if ((v.size() == 6) and (v[4] == "Ratio"))
            {
                return new AlgoPairing_Increment(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, false,
                                        std::stoi(v[5]));
            }
            else if ((v.size() == 6) and (v[4] == "Bin"))
            {
                return new AlgoPairing_Increment(algo_name, instance,
                                        map_str_to_score.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true, true,
                                        std::stoi(v[5]));
            }
        }
    }

    std::string err_string = "Could not create algo " + algo_name;
    throw std::runtime_error(err_string);
}


AlgoWFDm* createAlgoWFDm(const std::string& algo_name, const Instance &instance)
{
    std::vector<std::string> v = splitString(algo_name);
    const std::string& s = v[1];

    if (v[0] == "WFDm")
    {
        if (s == "BS")
        {
            // WFDm with Binary Search
            if (v.size() == 4)
            {
                return new AlgoWFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                return new AlgoWFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true);
            }
            else if ((v.size() == 5) and (v[4] == "Ratio"))
            {
                return new AlgoWFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true);
            }
        }
        else if (s == "Incr")
        {
            // WFDm with increment bins
            if (v.size() == 5)
            {
                return new AlgoWFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false,
                                        std::stoi(v[4]));
            }
            else if ((v.size() == 6) and (v[4] == "Dyn"))
            {
                return new AlgoWFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true,
                                        std::stoi(v[5]));
            }
            else if ((v.size() == 6) and (v[4] == "Ratio"))
            {
                return new AlgoWFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true,
                                        std::stoi(v[5]));
            }
        }
    }
    else if (v[0] == "BFDm")
    {
        if (s == "BS")
        {
            // BFDm with Binary Search
            if (v.size() == 4)
            {
                return new AlgoBFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false);
            }
            else if ((v.size() == 5) and (v[4] == "Dyn"))
            {
                return new AlgoBFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true);
            }
            else if ((v.size() == 5) and (v[4] == "Ratio"))
            {
                return new AlgoBFDm_BinSearch(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true);
            }
        }
        else if (s == "Incr")
        {
            // BFDm with increment bins
            if (v.size() == 5)
            {
                return new AlgoBFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        false,
                                        std::stoi(v[4]));
            }
            else if ((v.size() == 6) and (v[4] == "Dyn"))
            {
                return new AlgoBFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true,
                                        std::stoi(v[5]));
            }
            else if ((v.size() == 6) and (v[4] == "Ratio"))
            {
                return new AlgoBFDm_Increment(algo_name, instance,
                                        map_str_to_combination.at(v[2]),
                                        map_str_to_weight.at(v[3]),
                                        true,
                                        std::stoi(v[5]));
            }
        }
    }

    std::string err_string = "Could not create algo " + algo_name;
    throw std::runtime_error(err_string);
}

