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
    int num_coll;
    int num_tx;
    int coll_cnt;
    int num_drop;

    Node();

    Node(int backoff, int num_coll, int num_tx, int coll_cnt, int num_drop) {
        backoff = backoff;
        num_coll = num_coll;
        num_tx = num_tx;
        coll_cnt = coll_cnt;
        num_drop = num_drop;
    }
};

#endif