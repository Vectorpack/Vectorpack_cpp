#include <algo_utils.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
//#include <chrono> // TODO running time disabled for now
#include <stdexcept>

using namespace std;
using namespace vectorpack;
//using namespace std::chrono;

/*
 * Simple program to solve a Vector Bin Packing instance
 * with the provided algorithms
 * Inputs:
 *  - the instance file (.vbp format)
 *  - the name of the packing algorithm to run (or a lower bound)
 * Output:
 *  - prints the solution (number of bins)
 * Options:
 *  - to write the solution allocation in a file
 */

void show_usage(std::string prog_name)
{
    // TODO need to add an option to show a list of possible algorithms

    std::cerr << "Usages: " << prog_name << " <instance_file.vbp> <algorithm_name> [<options>]\n"
              << "Options:\n"
              << "\t-o <filename>, --output <filename>: Writes the solution and allocation into <filename>. Disables usual output to stdout.\n"
              << "\t\tThe first line of the output contains the number of bins in the solution.\n"
              << "\t\tThen each subsequent line represents the content of a bin\n"
              << "\t\twith the number of items in it, and the identifier of each item in that bin\n"
              << "\t--order-bins-output: Outputs bins in their order of creation\n"
              << "\t--offset-item-ids: Makes item identifiers start at 1 instead of 0 in the output\n"
              << "\t--no-shuffle: Disables shuffling of items during loading of the instance\n"
              << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        show_usage(argv[0]);
        return 1;
    }

    string instance_file(argv[1]);
    string instance_name = instance_file.substr(instance_file.find_last_of("/\\") + 1);
    string algo_name(argv[2]);

    bool write_alloc = false;
    string output_file;
    bool order_bins_output = false;
    bool offset_item_ids = false;
    bool shuffle_items = true;

    // Parsing options from CLI greatly inspired by
    // https://cplusplus.com/articles/DEN36Up4/
    for (int i = 3; i < argc; ++i)
    {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help"))
        {
            show_usage(argv[0]);
            return 0;
        }
        else if ((arg == "-o") || (arg == "--output"))
        {
            if (i+1 < argc) // Make sure we aren't at the end of argv
            {
                write_alloc = true;
                output_file = argv[i+1];
                ++i; // Because we consumed argument i+1
            }
            else
            {
                std::cerr << "Filename missing for option '--output'" << std::endl;
                return 1;
            }
        }
        else if (arg == "--order-bins-output")
        {
            order_bins_output = true;
        }
        else if (arg == "--offset-item-ids")
        {
            offset_item_ids = true;
        }
        else if (arg == "--no-shuffle")
        {
            shuffle_items = false;
        }
        else
        {
            std::cerr << "Unknow option: " << arg << std::endl;
            return 1;
        }
    }
    // End of parsing options

    Instance inst(instance_name, instance_file, shuffle_items);

    //time_point<high_resolution_clock> start;
    //time_point<high_resolution_clock> stop;

    // First check if lower bound is asked
    bool only_LB = false;
    int sol;
    BaseAlgo * algo;

    if (algo_name == "LB_clique")
    {
        only_LB = true;
        //start = high_resolution_clock::now();
        sol = LB_clique(inst);
        //stop = high_resolution_clock::now();
    }
    else if (algo_name == "LB_BPP")
    {
        only_LB = true;
        //start = high_resolution_clock::now();
        sol = LB_BPP(inst);
        //stop = high_resolution_clock::now();
    }
    else // This is an algorithm
    {
        int LB = LB_BPP(inst);
        string type_algo = algo_name.substr(0, 4);

        bool is_multibin = false; // Whether it is a multi-bin type algorithm
        if (type_algo == "BIM-")
        {
            algo = createAlgoPairing(algo_name, inst);
            is_multibin = true;
        }
        else if ( (type_algo == "WFDm") || (type_algo == "BFDm"))
        {
            algo = createAlgoWFDm(algo_name, inst);
            is_multibin = true;
        }
        else
        {
            algo = createAlgoCentric(algo_name, inst);
        }

        if (is_multibin)
        {
            BaseAlgo * algoFF = createAlgoCentric("FF", inst);
            int UB = algoFF->solveInstance(LB);
            delete algoFF;

            //start = high_resolution_clock::now();
            sol = algo->solveInstanceMultiBin(LB, UB);
            //stop = high_resolution_clock::now();
        }
        else
        {
            //start = high_resolution_clock::now();
            sol = algo->solveInstance(LB);
            //stop = high_resolution_clock::now();
        }
    }

    if (write_alloc)
    {
        // Write the algorithm solution in a file
        if (!only_LB)
        {
            algo->writeSolution(output_file, order_bins_output, offset_item_ids);
        }
        else
        {
            std::ofstream f(output_file, std::ios_base::trunc);
            if (!f.is_open())
            {
                std::string s("Cannot write solution to file " + output_file);
                throw std::runtime_error(s);
            }
            f << sol << "\n";
            f.close();
        }
    }
    else
    {
        // Print the solution to standard output
        cout << sol << "\n";

        if (!only_LB)
        {
            if (order_bins_output)
            {
                algo->orderBinsId();
            }

            // Then for each bin, one bin per line, write the number of items in the bin and the list of item ids
            for (Bin* bin : algo->getBins())
            {
                std::vector<int> allocList = bin->getAllocList();
                std::string s(std::to_string(allocList.size()));
                for (int i : allocList)
                {
                    if (offset_item_ids)
                    {
                        i += 1;
                    }
                    s += " " + std::to_string(i);
                }
                cout << s << "\n";
            }
        }

    }

    if (!only_LB)
    {
        delete algo;
    }

    return 0;
}
