/**
 * FILENAME: csma.cpp
 *
 * DESCRIPTION:
 *
 * DATE: Wednesday, Dec 8, 2022
 *
 * AUTHOR:
 *
 */

// #define DEBUG

#include "csma.h"

void read_input_file(std::ifstream &input_file) {
    char type;
    std::string line;
    int val, start = 0;

    while (getline(input_file,line)) {
        type = line.at(TYPE_INDEX);

        if (type == NODE || type == PACKET_SIZE || type == MAX_RETX_ATTEMPT || type == TTL_TIME) {
            line = line.substr(NUM_START_POS);
            val = atoi(line.c_str());
        } else if (type == MAX_RETX) {
            line = line.substr(NUM_START_POS);
        } else {
            printf("[ERROR]: TYPE NOT FOUND\n");
        }

        switch (type) {
            case NODE:
                N = val;
                break;
            case PACKET_SIZE:
                L = val;
                break;
            case MAX_RETX:
                for (int i = 0; i < line.length(); ++i) {
                    if (line[i] == ' ') {
                        val = atoi(line.substr(start, i).c_str());
                        R.push_back(val);
                        start = i;
                    }
                }
                break;
            case MAX_RETX_ATTEMPT:
                M = val;
                break;
            case TTL_TIME:
                T = val;
                break;
            default:
                printf("[ERROR]: TYPE NOT FOUND\n");
        }
    }
    #ifdef DEBUG
    printf("[INFO]: ----- Input File -----\n");
    printf("[INFO]: N %d\n", N);
    printf("[INFO]: L %d\n", L);
    printf("[INFO]: R");
    for (int i = 0; i < R.size(); ++i) {
        printf(" %d", R[i]);
    }
    printf("\n");
    printf("[INFO]: M %d\n", M);
    printf("[INFO]: T %d\n", T);
    printf("[INFO]: ----------------------\n");
    #endif
}

void pre_sim() {
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        Node node(rand() % (R[0]+1), 0, 0, 0, 0);
        node_vec.push_back(node);
    }
    #ifdef DEBUG
    printf("[INFO]: -------------------- Node --------------------\n");
    for (int i = 0; i < node_vec.size(); ++i) {
        printf("[INFO]: BACKOFF %d #_TX %d #_COLL %d COLL_# %d #_DROP %d\n",
               node_vec[i].backoff, node_vec[i].num_tx, node_vec[i].num_coll,
               node_vec[i].coll_cnt, node_vec[i].num_drop);

    }
    printf("[INFO]: ----------------------------------------------\n");
    #endif
}

void run_sim() {
    int num_pkt = 0;

    for (int clk = 0; clk < T; ++clk) {
        std::vector<Node *> ready_node_vec;

        if (num_pkt == 0) chnl_avil = true;

        if (chnl_avil) {
            for (int i = 0; i < node_vec.size(); ++i) {
                if (node_vec[i].backoff == 0) {
                    ready_node_vec.push_back(&node_vec[i]);
                }
            }

            if (ready_node_vec.size() == 0) {
                for (int i = 0; i < node_vec.size(); ++i) {
                    --node_vec[i].backoff;
                }
            } else if (ready_node_vec.size() == 1){
                chnl_avil = false;
                num_pkt = L;
                Node *node = ready_node_vec[0];
                node->backoff = rand() % (R[0] + 1);
                node->num_tx++;
                node->coll_cnt = 0;
            } else {
                for (int i = 0; i < ready_node_vec.size(); ++i) {
                    Node *node = ready_node_vec[i];
                    node->coll_cnt++;
                    if (node->coll_cnt == M){
                        node->backoff =  rand() % (R[0] + 1);
                        node->num_coll++;
                        node->coll_cnt = 0;
                        node->num_drop++;
                    }
                    else {
                        if (node->coll_cnt <= R.size()){
                            node->backoff = rand() % (R[node->coll_cnt] + 1);
                        }
                        else{
                            node->backoff = rand() % (R[R.size()-1] + 1);
                        }
                    }
                }
            }
        } else {
            num_pkt--;
        }
        #ifdef DEBUG
        printf("[INFO]: -------------------- Node --------------------\n");
        for (int i = 0; i < node_vec.size(); ++i) {
            printf("[INFO]: BACKOFF %d #_TX %d #_COLL %d COLL_# %d #_DROP %d\n",
                   node_vec[i].backoff, node_vec[i].num_tx, node_vec[i].num_coll,
                   node_vec[i].coll_cnt, node_vec[i].num_drop);

        }
        printf("[INFO]: ----------------------------------------------\n");
        #endif
    }
}

float get_sim_result() {
    int ttl_tx = 0, ttl_coll = 0;

    for (std::vector<Node>::iterator iter = node_vec.begin(); iter != node_vec.end(); ++iter) {
        Node *node = &(*iter);
        ttl_coll += node->num_coll;
        ttl_tx += node->num_tx;
    }
    float link_util = float(ttl_tx) * float(L) / float(T);
    return link_util;
}

void w_link_util(FILE *output_fp, float link_util) {
    fprintf(output_fp, "%.2f", link_util);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("[USAGE]: ./csma <input file>\n");
        return -1;
    }

    printf("[INFO]: Reading input file %s...\n", argv[1]);
    std::ifstream input_file;
    input_file.open(argv[1], std::ifstream::in);
    if (!input_file) {
        printf("[ERROR]: Failed to open input file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    read_input_file(input_file);
    input_file.close();
    printf("[INFO]: Finish reading input file %s\n\n", argv[1]);

    printf("[INFO]: Prepare simulation...\n");
    pre_sim();
    printf("[INFO]: Finish preparing simulation\n\n");

    printf("[INFO]: Start Simulation...\n");
    run_sim();
    float link_util = get_sim_result();
    printf("[INFO]: Link Utilization %f\n", link_util);
    printf("[INFO]: Finish Simulation\n\n");

    printf("[INFO]: Write link utilization into output file %s\n", "output.txt");
    FILE *output_fp = fopen("output.txt", "w");
    if (output_fp == NULL) {
        printf("[ERROR]: Failed to open output file output.txt\n");
        exit(EXIT_FAILURE);
    }
    w_link_util(output_fp, link_util);
    fclose(output_fp);
    printf("[INFO]: Finish writing link utilization into output file %s", "output.txt");

    return 0;
}