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
        if (node_set.find(dst) == node_set.end()) {
            node_set.insert(dst);
        }
    }
    #ifdef DEBUG
    printf("[DEBUG]: ---------- TOPOLOGY ----------\n");
    for (auto const &iter1: topo) {
        for (auto const &iter2: iter1.second) {
            src = iter1.first;
            dst = iter2.first;
            cost = iter2.second;
            printf("[TOPO]:  src %d, dst %d, cost %d\n", src, dst, cost);
        }
    }
    printf("[DEBUG]: ------------------------------\n");
    #endif
}

void dijkstra() {
    int num_node = node_set.size();
    fwd_tbl.clear();

    /* get fwd_tbl of all nodes in node_set */
    for (int cur_node: node_set) {
        #ifdef DEBUG
        printf("[DEBUG]: ********* CUR NODE %d *********\n", cur_node);
        #endif

        ckd_node.clear();
        ckd_node.insert(cur_node);

        for (const int node: node_set) {
            if (node == cur_node) {
                fwd_tbl[cur_node][node].first = 0;
                fwd_tbl[cur_node][node].second = cur_node;
            } else if (topo[cur_node].find(node) != topo[cur_node].end()){
                fwd_tbl[cur_node][node].first = topo[cur_node][node];
                fwd_tbl[cur_node][node].second = cur_node;
            } else {
                fwd_tbl[cur_node][node].first = INT_MAX;
                fwd_tbl[cur_node][node].second = INT_MAX;
            }
        }
        #ifdef DEBUG
        printf("[DEBUG]: ---------- FWD_TBL -----------\n");
        for (auto iter1 : fwd_tbl) {
            for (auto iter2 : iter1.second) {
                printf("[DEBUG]: src %d, dst %d, cost %d, next_hop %d\n", iter1.first, iter2.first, iter2.second.first, iter2.second.second);
            }
        }
        printf("\n");
        #endif

        for (int i = 0; i < num_node-1; i++) {
            int min_dist = INT_MAX;
            int min_node = INT_MAX;

            for (const int node : node_set) {
                if (ckd_node.find(node) == ckd_node.end()) {
                    if (fwd_tbl[cur_node][node].first < min_dist) {
                        min_dist = fwd_tbl[cur_node][node].first;
                        min_node = node;
                    } else if (fwd_tbl[cur_node][node].first == min_dist) {
                        min_node = std::min(min_node, node);
                    }
                }
            }
            #ifdef DEBUG
            printf("[DEBUG]: ------- CUR MIN_NODE %d -------\n", min_node);
            #endif

            ckd_node.insert(min_node);

            for (auto nbr : topo[min_node]) {
                int nbr_node = nbr.first;
                int nbr_cost = nbr.second;
                if (fwd_tbl[cur_node][min_node].first + nbr_cost < fwd_tbl[cur_node][nbr_node].first) {
                    fwd_tbl[cur_node][nbr_node].first = fwd_tbl[cur_node][min_node].first + nbr_cost;
                    fwd_tbl[cur_node][nbr_node].second = min_node;
                }
                else if (fwd_tbl[cur_node][min_node].first + nbr_cost == fwd_tbl[cur_node][nbr_node].first) {
                    fwd_tbl[cur_node][min_node].second = std::min(fwd_tbl[cur_node][min_node].second, min_node);
                }
            }
            #ifdef DEBUG
            printf("[DEBUG]: ---------- FWD_TBL -----------\n");
            for (auto iter1 : fwd_tbl) {
                for (auto iter2 : iter1.second) {
                    printf("[DEBUG]: src %d, dst %d, cost %d, next_hop %d\n", iter1.first, iter2.first, iter2.second.first, iter2.second.second);
                }
            }
            printf("\n");
            #endif
        }
    }
    #ifdef DEBUG
    printf("[DEBUG]: ------- FINAL FWD_TBL --------\n");
    for (auto iter1 : fwd_tbl) {
        for (auto iter2 : iter1.second) {
            printf("[DEBUG]: src %d, dst %d, cost %d, next_hop %d\n", iter1.first, iter2.first, iter2.second.first, iter2.second.second);
        }
    }
    printf("\n");
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

    printf("[INFO]: Start DIJKSTRA ALGORITHM...\n");
    dijkstra();
    printf("[INFO]: Finish DIJKSTRA ALGORITHM\n\n");

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