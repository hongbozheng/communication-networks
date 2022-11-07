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

#include "linkstate.h"

#define DEBUG

void create_topo(FILE *fp) {
    printf("[INFO]: Reading topology file...\n");
    int src_id, dst_id, cost;
    while(fscanf(fp, "%d", &src_id) != EOF) {
        fscanf(fp, "%d %d", &dst_id, &cost);
        #ifdef DEBUG
        printf("[TOPO]: SRC_ID %d, DST_ID %d, COST %d\n", src_id, dst_id, cost);
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

void update_cost_tbl() {
    for (auto n : topo) {
        std::vector<rtentry_t> fwd_tbl;
        /* IMPLEMENT DIJKSTRA ALGORITHM */
    }
}

void parse_msg(std::ifstream &msg_file) {
    std::string line;
    int src_id, dst_id;
    printf("[INFO]: Reading message file..\n");
    while(getline(msg_file, line)) {
        sscanf(line.c_str(), "%d %d %*s", &src_id, &dst_id);
        line = line.substr(line.find(' ')+1);
        line = line.substr(line.find(' ')+1);
        #ifdef DEBUG
        const char *message = line.c_str();
        printf("[MSG]:  SRC_ID %d, DST_ID %d, MSG %s\n", src_id, dst_id, message);
        #endif
        msg_t *msg = new msg_t;
        msg->src_id = src_id;
        msg->dst_id = dst_id;
        msg->m = line;
        msg_vec.push_back(msg);
    }
    printf("[INFO]: Finish reading message file\n");
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("[USAGE]: ./linkstate <topology_file> <message_file> <changes_file>\n");
        return -1;
    }
    printf("[INFO]: Creating topology map from file %s...\n", argv[1]);
    FILE *topo_fp = fopen(argv[1], "r");
    if (topo_fp == NULL) {
        printf("[ERROR]: Failed to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    create_topo(topo_fp);
    fclose(topo_fp);
    printf("[INFO]: Finish creating topology map from file %s\n\n", argv[1]);

    printf("[INFO]: Creating message vector from file %s...\n", argv[2]);
    std::ifstream msg_file;
    msg_file.open(argv[2], std::ifstream::in);
    if (!msg_file) {
        printf("[ERROR]: Failed to open file %s", argv[2]);
        exit(EXIT_FAILURE);
    }
    parse_msg(msg_file);
    msg_file.close();
    printf("[INFO]: Finish creating message vector from file %s\n", argv[2]);

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    fclose(fpOut);


    return 0;
}