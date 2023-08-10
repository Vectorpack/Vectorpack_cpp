#include <utils.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <stdexcept>

using namespace std;
using namespace vectorpack;
using namespace std::chrono;

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
              << "\t-o <filename>, --output <filename>: Writes the solution and allocation into <filename>. Disables usual output to stdout\n"
              << "\t--no-time: Do not output algorithm running time\n"
              << "\t--no-shuffle: Disables shuffling of items during load of the instance\n"
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

    bool print_alloc = false;
    string output_alloc_file;
    bool shuffle_items = true;
    bool show_time = true;

    // Parsing options from CLI greatly inspired by
    // https://cplusplus.com/articles/DEN36Up4/
    for (int i = 3; i < argc; ++i)
    {
        std::string arg = argv[i];
        if ((arg == "-o") || (arg == "--output"))
        {
            if (i+1 < argc) // Make sure we aren't at the end of argv
            {
                print_alloc = true;
                output_alloc_file = argv[i+1];
                ++i; // Because we consumed argument i+1
            }
            else
            {
                std::cerr << "Filename missing for option '--output'" << std::endl;
                return 1;
            }
        }
        else if (arg == "--no-shuffle")
        {
            shuffle_items = false;
        }
        else if (arg == "--no-time")
        {
            show_time = false;
        }
        else
        {
            std::cerr << "Unknow option: " << arg << std::endl;
        }
    }
    // End of parsing options

    Instance inst(instance_name, instance_file, shuffle_items);

    time_point<high_resolution_clock> start;
    time_point<high_resolution_clock> stop;

    // First check if lower bound is asked
    bool only_LB = false;
    int sol;
    BaseAlgo * algo;

    if (algo_name == "LB_clique")
    {
        only_LB = true;
        start = high_resolution_clock::now();
        sol = LB_clique(inst);
        stop = high_resolution_clock::now();
    }
    else if (algo_name == "LB_BPP")
    {
        only_LB = true;
        start = high_resolution_clock::now();
        sol = LB_BPP(inst);
        stop = high_resolution_clock::now();
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

            start = high_resolution_clock::now();
            sol = algo->solveInstanceMultiBin(LB, UB);
            stop = high_resolution_clock::now();
        }
        else
        {
            start = high_resolution_clock::now();
            sol = algo->solveInstance(LB);
            stop = high_resolution_clock::now();
        }
    }



    string s = algo_name + ": " + to_string(sol) + " bins";
    if (show_time)
    {
        int dur = (duration_cast<milliseconds>(stop - start)).count();
        if (dur < 1000.0)
        {
            s+= " in " + to_string(dur) + " millisecond(s)";
        }
        else
        {
            dur = (duration_cast<seconds>(stop - start)).count();
            s+= " in " + to_string(dur) + " second(s)";
        }
    }

    if (!print_alloc) // Print solution to stdout
    {
        std::cout << s << std::endl;
    }
    else
    {
        ofstream f(output_alloc_file, ios_base::trunc);
        if (!f.is_open())
        {
            string s("Cannot write file " + output_alloc_file);
            throw std::runtime_error(s);
        }

        f << s << "\n";
        f.flush();

        if (!only_LB)
        {
            algo->orderBinsId();
            for (const Bin* bin : algo->getBins())
            {
                f << bin->formatAlloc() << "\n";
            }
            f.flush();
        }
        f.close();
    }

    //if (!only_LB)
    if (algo != nullptr)
    {
        delete algo;
    }

    return 0;
}
