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

bool check_src_dst(int src, int dst, const topo_t topo) {
    auto src_iter = topo.find(src);
    auto dst_iter = src_iter->second.find(dst);
    return !(dst_iter == src_iter->second.end());
}

void create_topo(FILE *fp, topo_t &topo, fwd_tbl_t &fwd_tbl) {
    printf("[INFO]: Creating topology map...\n");
    int src, dst, cost;
    while(fscanf(fp, "%d", &src) != EOF) {
        fscanf(fp, "%d %d", &dst, &cost);
        #ifdef DEBUG
        printf("[TOPO]: SRC %d, DST %d, COST %d\n", src, dst, cost);
        #endif
        topo[src][dst] = cost;
        topo[dst][src] = cost;
        if (node_set.find(src) == node_set.end()) {
            node_set.insert(src);
        }
        if (topo.find(dst) == topo.end()) {
            node_set.insert(dst);
        }
    }
    printf("[INFO]: Finish creating topology map\n");

    printf("[INFO]: Creating forward table...\n");
    for (auto i = node_set.begin(); i != node_set.end(); ++i) {
        src = *i;
        for (auto k = node_set.begin(); k != node_set.end(); ++k) {
            dst = *i;
            if (src == dst) {
                topo[src][dst] = 0;
            }
            if (!check_src_dst(src, dst, topo)) {
                topo[src][dst] = -999;
            }
            fwd_tbl[src][dst] = std::make_pair(src, topo[src][dst]);
        }
    }
    printf("[INFO]: Finish creating forward table...\n");
}

//void update_cost_tbl() {
//    for (auto n : topo) {
//        std::vector<rtentry_t> fwd_tbl;
//        /* IMPLEMENT DIJKSTRA ALGORITHM */
//    }
//}

void get_msg(std::ifstream &msg_file) {
    std::string line;
    int src, dst;
    msg_t msg;

    printf("[INFO]: Creating message vector..\n");
    while (getline(msg_file, line)) {
        sscanf(line.c_str(), "%d %d %*s", &src, &dst);
        line = line.substr(line.find(' ')+1);
        line = line.substr(line.find(' ')+1);
        #ifdef DEBUG
        const char *message = line.c_str();
        printf("[MSG]:  SRC_ID %d, DST_ID %d, MSG %s\n", src, dst, message);
        #endif
        msg.src = src;
        msg.dst = dst;
        msg.msg = line;
        msg_vec.push_back(msg);
    }
    printf("[INFO]: Finish creating message vector\n");
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("[USAGE]: ./linkstate <topology_file> <message_file> <changes_file>\n");
        return -1;
    }

    printf("[INFO]: Creating topology map & forward table from file %s...\n", argv[1]);
    FILE *topo_fp = fopen(argv[1], "r");
    if (topo_fp == NULL) {
        printf("[ERROR]: Failed to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    create_topo(topo_fp, topo, fwd_tbl);
    fclose(topo_fp);
    printf("[INFO]: Finish creating topology map & forward table from file %s\n\n", argv[1]);

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