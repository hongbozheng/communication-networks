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
    int src, dst, cost;
    while(fscanf(fp, "%d", &src) != EOF) {
        fscanf(fp, "%d %d", &dst, &cost);
        topo[src][dst] = cost;
        topo[dst][src] = cost;
        if (node_set.find(src) == node_set.end()) {
            node_set.insert(src);
        }
        if (topo.find(dst) == topo.end()) {
            node_set.insert(dst);
        }
    }
    #ifdef DEBUG
    printf("[DEBUG]: ---------- Topology ----------\n");
    for (auto const &iter1: topo) {
        for (auto const &iter2: iter1.second) {
            src = iter1.first;
            dst = iter2.first;
            cost = iter2.second;
            printf("[TOPO]:  SRC %d, DST %d, COST %d\n", src, dst, cost);
        }
    }
    printf("[DEBUG]: ----------------------------\n");
    #endif
}

void get_msg(std::ifstream &msg_file) {
    std::string line;
    int src, dst;
    msg_t msg;

    while (getline(msg_file, line)) {
        sscanf(line.c_str(), "%d %d %*s", &src, &dst);
        line = line.substr(line.find(' ')+1);
        line = line.substr(line.find(' ')+1);
        msg.src = src;
        msg.dst = dst;
        msg.msg = line;
        msg_vec.push_back(msg);
    }
    #ifdef DEBUG
    printf("[DEBUG]: ---------- Message ----------\n");
    const char *message;
    for (auto msg : msg_vec) {
        src = msg.src;
        dst = msg.dst;
        message = msg.msg.c_str();
        printf("[MSG]:   SRC_ID %d, DST_ID %d, MSG %s\n", src, dst, message);
    }
    printf("[DEBUG]: -----------------------------\n");
    #endif
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("[USAGE]: ./linkstate <topology_file> <message_file> <changes_file>\n");
        return -1;
    }

    printf("[INFO]: Creating topology from file %s...\n", argv[1]);
    FILE *topo_fp = fopen(argv[1], "r");
    if (topo_fp == NULL) {
        printf("[ERROR]: Failed to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    create_topo(topo_fp);
    fclose(topo_fp);
    printf("[INFO]: Finish creating topology from file %s\n\n", argv[1]);

    printf("[INFO]: Creating message vector from file %s...\n", argv[2]);
    std::ifstream msg_file;
    msg_file.open(argv[2], std::ifstream::in);
    if (!msg_file) {
        printf("[ERROR]: Failed to open file %s", argv[2]);
        exit(EXIT_FAILURE);
    }
    get_msg(msg_file);
    msg_file.close();
    printf("[INFO]: Finish creating message vector from file %s\n", argv[2]);

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    fclose(fpOut);


    return 0;
}