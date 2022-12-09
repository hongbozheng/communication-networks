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

    Node(int backoff, int num_coll, int num_tx, int coll_cnt, int num_drop) {
        this->backoff = backoff;
        this->num_coll = num_coll;
        this->num_tx = num_tx;
        this->coll_cnt = coll_cnt;
        this->num_drop = num_drop;
    }
};

#endif