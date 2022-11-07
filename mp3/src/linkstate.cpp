/**
 * FILENAME: linkstate.cpp
 *
 * DESCRIPTION:
 *
 * DATE: Saturday, Nov 5, 2022
 *
 * AUTHOR:
 *
 */

#include <stdio.h>

#include "linkstate.h"

#define DEBUG

void read_topo(FILE *fp) {
    printf("[INFO]: Reading topology file...\n");
    int src_id, dst_id, cost;
    while(fscanf(fp, "%d", &src_id) != EOF) {
        fscanf(fp, "%d %d", &dst_id, &cost);
        #ifdef DEBUG
        printf("[TOPO]: src_id %d, dst_id %d, cost %d\n", src_id, dst_id, cost);
        #endif
        if(topo.find(src_id) == topo.end()) {
            node_t *src_node = new node_t;
            src_node->id = src_id;
            topo[src_id] = src_node;
        }
        if (topo.find(dst_id) == topo.end()) {
            node_t *dst_node = new node_t;
            dst_node->id = dst_id;
            topo[dst_id] = dst_node;
        }
        topo[src_id]->neighbor[dst_id] = cost;
        topo[dst_id]->neighbor[src_id] = cost;
    }
    printf("[INFO]: Finish reading topology file\n");
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("[USAGE]: ./linkstate <topology_file> <message_file> <changes_file>\n");
        return -1;
    }
    printf("[INFO]: Creating topology map from %s file\n", argv[1]);
    FILE *topo_fp = fopen(argv[1], "r");
    read_topo(topo_fp);

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    fclose(fpOut);


    return 0;
}