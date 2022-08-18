#include "instance.hpp"
#include "bin.hpp"
#include "item.hpp"

#include <iostream>

using namespace std;
using namespace vectorpack;


int main(int argc, char** argv)
{
    string base_path("/home/mommess/Documents/Vectorpack/Vectorpack_cpp/");
    string instance_name("test_20_3");

    Instance inst("test", base_path + instance_name + ".vbp", false);

    BinList bins;
    Bin * b0 = new Bin(0, inst.getBinCapacities());
    Bin * b1 = new Bin(1, inst.getBinCapacities());
    bins.push_back(b0);
    bins.push_back(b1);

    //ItemList& items = inst.getItems();
    auto item_it = inst.getItems().begin();
    b0->addItem(*item_it);
    item_it++;
    b1->addItem(*item_it);

    for (Bin* b : bins)
    {
        b->printAlloc();
    }

    return 0;
}
