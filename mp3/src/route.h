/**
 * FILENAME: route.h
 *
 * DESCRIPTION: route header file
 *              struct rtentry ----- routing table entry
 *              struct node    ----- node
 *              struct msg     ----- message
 *
 * DATE: Saturday, Nov 5, 2022
 *
 * AUTHOR:
 *
 */

#ifndef _ROUTE_H
#define _ROUTE_H

#include <unordered_map>
#include <vector>

typedef struct rtentry {
    int rt_dst;         // routing table dest
    int nxt_hop;        // next hop
    int path_cost;      // cost of path
} rtentry_t;

typedef struct node {
    int id;                                 // node id
    std::unordered_map<int, int> neighbor;  // <neighbor id, cost>
    std::vector<std::vector<int>> cost_tbl; // matrix of node cost
} node_t;

typedef struct msg {
    int src_id;         // src node
    int dst_id;         // dest node
    std::string m;      // message
} msg_t;

#endif