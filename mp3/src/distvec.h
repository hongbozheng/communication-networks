/**
 * FILENAME: distvec.h
 *
 * DESCRIPTION: distvec header file
 *
 * DATE: Saturday, Nov 5, 2022
 *
 * AUTHOR:
 *
 */

#ifndef DISTVEC_H
#define DISTVEC_H

#include <set>              /* node_set_t   */
#include <unordered_map>    /* topo_t       */
#include <vector>           /* msg_vec_t    */
#include <limits.h>         /* INT_MAX      */
#include <unordered_set>    /* ckd_node_t   */
#include <deque>            /* msg route    */
#include <fstream>          /* message_file */

#define OUTPUT_FILENAME "output.txt"

typedef std::set<int> node_set_t;                                                           /* node set */
typedef std::unordered_map<int, std::unordered_map<int, int>> topo_t;                       /* <src_id, <dst_id, cost>> */
typedef std::unordered_map<int, std::unordered_map<int, std::pair<int, int>>> fwd_tbl_t;    /* <src_id, <dst_id, <cost, next_hop>>> */
typedef std::unordered_set<int> ckd_node_t;                                                 /* checked node set */
typedef struct msg {
    int src;                            /* src node id  */
    int dst;                            /* dst node id  */
    std::string msg;                    /* message      */
} msg_t;
typedef std::vector<msg_t> msg_vec_t;   /* vec<msg.src_id, msg.dst_id, msg.m> */

node_set_t node_set;                    /* node set         */
topo_t topo;                            /* topology         */
fwd_tbl_t fwd_tbl;                      /* forward table    */
msg_vec_t msg_vec;                      /* message vector   */
ckd_node_t ckd_node;                    /* checked node set */

#endif