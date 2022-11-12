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

#include <unordered_set>    /* node_set_t   */
#include <unordered_map>    /* topo_t       */
#include <vector>           /* msg_vec_t    */
#include <limits.h>         /* INT_MAX      */
#include <queue>            /* */
#include <fstream>          /* message_file */

typedef std::unordered_set<int> node_set_t;                                                 /* node set */
typedef std::unordered_map<int, std::unordered_map<int, int>> topo_t;                       /* <src_id, <dst_id, cost>> */
typedef std::unordered_map<int, std::unordered_map<int, std::pair<int, int>>> fwd_tbl_t;    /* <src_id, <dest_id, <cost, next_hop>>> */
typedef struct msg {
    int src;                            /* src node id  */
    int dst;                            /* dest node id */
    std::string msg;                    /* message      */
} msg_t;
typedef std::vector<msg_t> msg_vec_t;   /* vec<msg.src_id, msg.dst_id, msg.m> */

node_set_t node_set;                    /* node set         */
topo_t topo;                            /* topology         */
fwd_tbl_t fwd_tbl;                      /* forward table    */
msg_vec_t msg_vec;                      /* message vector   */

#endif