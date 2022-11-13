/**
 * FILENAME: linkstate.h
 *
 * DESCRIPTION: linkstate header file
 *
 * DATE: Saturday, Nov 5, 2022
 *
 * AUTHOR:
 *
 */

#ifndef LINKSTATE_H
#define LINKSTATE_H

#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;

#define INFINITY 99999
#define OUTPUT_FILENAME "output.txt"

map<int, map<int, int>> topo;
map<int, map<int, int>> dist;
map<int, map<int, int>> next_hop;
map<int, map<int, int>> prev_hop;
vector<pair<int, pair<int, string>>> msg_vec;
vector<pair<int, pair<int, int>>> chg_vec;
map<int, int> tmp_dist;
int num_node = 0;

struct cmp_dist {
    bool operator() (int node1, int node2) const {
        return tmp_dist[node1] == tmp_dist[node2]? node1 > node2 : tmp_dist[node1] > tmp_dist[node2];
    }
};

#endif