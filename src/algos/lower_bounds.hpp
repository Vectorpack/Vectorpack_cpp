#ifndef LOWER_BOUNDS_HPP
#define LOWER_BOUNDS_HPP

#include <item.hpp>
#include <instance.hpp>
using namespace vectorpack;

int LB_BPP(const Instance & instance);

int LB_clique(const Instance & instance);

bool check_incompatibility(const Item* itema, const Item* itemb, const SizeList& capacities);
int argmax_degree(const std::vector<int> degrees);

#endif // LOWER_BOUNDS_HPP
