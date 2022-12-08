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

#define DEBUG

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

void run_sim() {
    int time_to_finish = 0;
    Node *node = new Node(0, 0, 0, 0, 0);
    printf("%d\n", node->backoff);
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

    printf("[INFO]: Start Simulation...\n");
    run_sim();
    printf("[INFO]: Finish Simulation\n");

    FILE *output_fp = fopen("output.txt", "w");
    if (output_fp == NULL) {
        printf("[ERROR]: Failed to open output file output.txt\n");
        exit(EXIT_FAILURE);
    }
    fclose(output_fp);

    return 0;
}