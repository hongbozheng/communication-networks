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
    while (fscanf(fp, "%d", &src) != EOF) {
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
    printf("[DEBUG]: ---------- NODE_SET ----------\n[DEBUG]: ");
    for (const auto node : node_set) {
        printf("%d ", node);
    }
    printf("\n[DEBUG]: ---------- TOPOLOGY ----------\n");
    for (auto const iter1: topo) {
        for (auto const iter2: iter1.second) {
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

    for (const int cur_node: node_set) {
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
        for (const auto iter1 : fwd_tbl) {
            for (const auto iter2 : iter1.second) {
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
                #ifdef DEBUG
                printf("[DEBUG]: nbr_node %d, cost %d\n", nbr_node, nbr_cost);
                #endif
                if (fwd_tbl[cur_node][min_node].first != INT_MAX &&
                    fwd_tbl[cur_node][min_node].first + nbr_cost < fwd_tbl[cur_node][nbr_node].first) {
                    fwd_tbl[cur_node][nbr_node].first = fwd_tbl[cur_node][min_node].first + nbr_cost;
                    fwd_tbl[cur_node][nbr_node].second = min_node;
                }
                else if (fwd_tbl[cur_node][min_node].first != INT_MAX &&
                         fwd_tbl[cur_node][min_node].first + nbr_cost == fwd_tbl[cur_node][nbr_node].first) {
                    fwd_tbl[cur_node][nbr_node].second = std::min(fwd_tbl[cur_node][nbr_node].second, min_node);
                }
            }
            #ifdef DEBUG
            printf("[DEBUG]: ---------- FWD_TBL -----------\n");
            for (const auto iter1 : fwd_tbl) {
                for (const auto iter2 : iter1.second) {
                    printf("[DEBUG]: src %d, dst %d, cost %d, next_hop %d\n", iter1.first, iter2.first, iter2.second.first, iter2.second.second);
                }
            }
            printf("\n");
            #endif
        }
    }
    #ifdef DEBUG
    printf("[DEBUG]: ------- FINAL FWD_TBL --------\n");
    for (const auto iter1 : fwd_tbl) {
        for (const auto iter2 : iter1.second) {
            printf("[DEBUG]: src %d, dst %d, cost %d, next_hop %d\n", iter1.first, iter2.first, iter2.second.first, iter2.second.second);
        }
    }
    printf("\n");
    #endif
}

void w_fwd_tbl(FILE *fp) {
    int nxt_hop;
    for (const auto src : node_set) {
        for (const auto dst : node_set) {
            if (fwd_tbl[src][dst].first != INT_MAX) {
                nxt_hop = fwd_tbl[dst][src].second;
                fprintf(fp, "%d %d %d\n", dst, nxt_hop, fwd_tbl[src][dst].first);
            }
        }
    }
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

void send_msg(FILE *fp) {
    int src, dst, nxt_hop;
    std::deque<int> route;
    const char *message;

    for (const auto msg : msg_vec) {
        route.clear();
        src = msg.src;
        dst = nxt_hop = msg.dst;
        message = msg.msg.c_str();

        if (fwd_tbl[src][dst].first == INT_MAX || node_set.find(src) == node_set.end() ||
            node_set.find(dst) == node_set.end()) {
            fprintf(fp, "from %d to %d cost infinite hops unreachable message %s\n", src, dst, message);
            continue;
        }

        do {
            nxt_hop = fwd_tbl[src][nxt_hop].second;
            route.push_front(nxt_hop);
        } while (nxt_hop != src);
        #ifdef DEBUG
        printf("[DEBUG]: ------------ ROUTE -----------\n[DEBUG]: ");
        for (const auto node : route) {
            printf("%d ", node);
        }
        printf("\n[DEBUG]: ------------------------------\n\n");
        #endif

        fprintf(fp, "from %d to %d cost %d hops ", src, dst, fwd_tbl[src][dst].first);
        for (const auto node : route) {
            fprintf(fp, "%d ", node);
        }
        fprintf(fp, "message %s\n", message);
    }
}

void update_fwd_tbl(FILE *chg_fp, FILE *output_fp) {
    int src, dst, cost;
    while (fscanf(chg_fp, "%d", &src) != EOF) {
        fscanf(chg_fp, "%d %d", &dst, &cost);
        if (src == 0 || dst == 0 || cost == 0) continue;
        if (cost == -999) {
            if (topo[src].find(dst) != topo[src].end()) {
                topo[src].erase(dst);
            }
            if (topo[dst].find(src) != topo[dst].end()) {
                topo[dst].erase(src);
            }
        } else {
            topo[src][dst] = cost;
            topo[dst][src] = cost;
        }
        #ifdef DEBUG
        printf("[DEBUG]: ---------- NODE_SET ----------\n[DEBUG]: ");
        for (const auto node : node_set) {
            printf("%d ", node);
        }
        printf("\n[DEBUG]: ---------- TOPOLOGY ----------\n");
        for (auto const iter1: topo) {
            for (auto const iter2: iter1.second) {
                src = iter1.first;
                dst = iter2.first;
                cost = iter2.second;
                printf("[TOPO]:  src %d, dst %d, cost %d\n", src, dst, cost);
            }
        }
        printf("\n");
        #endif
        dijkstra();
        w_fwd_tbl(output_fp);
        send_msg(output_fp);
    }
}

int main(int argc, char** argv) {
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
    printf("[INFO]: Finish creating message vector from file %s\n\n", argv[2]);

    FILE *output_fp = fopen(OUTPUT_FILENAME, "w");
    if (output_fp == NULL) {
        printf("[ERROR]: Failed to open file %s\n", OUTPUT_FILENAME);
        exit(EXIT_FAILURE);
    }
    printf("[INFO]: Start writing forward_table...\n");
    w_fwd_tbl(output_fp);
    printf("[INFO]: Finish writing forward_table...\n\n");

    printf("[INFO]: Start sending message(s)...\n");
    send_msg(output_fp);
    printf("[INFO]: Finish sending message(s)\n\n");

    printf("[INFO]: Apply changes from file %s\n", argv[3]);
    FILE *chg_fp = fopen(argv[3], "r");
    update_fwd_tbl(chg_fp, output_fp);
    printf("[INFO]: Finish applying changes from file %s\n", argv[3]);
    fclose(output_fp);
    return 0;
}