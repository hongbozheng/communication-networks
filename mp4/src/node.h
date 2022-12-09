/**
 * FILENAME: node.h
 *
 * DESCRIPTION: node header file
 *
 * DATE: Wednesday, Dec 8, 2022
 *
 * AUTHOR:
 *
 */

#ifndef NODE_H
#define NODE_H

class Node {
public:
    int backoff;            /* need to refactor the following */
    int num_tx;
    int num_coll;
    int coll_cnt;
    int num_drop;

    Node();

    Node(int bo, int num_collision, int num_tran, int collision_cnt, int number_drop) {
        backoff = bo;
        num_coll = num_collision;
        num_tx = num_tran;
        coll_cnt = collision_cnt;
        num_drop = number_drop;
    }
};

#endif