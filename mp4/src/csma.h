/**
 * FILENAME: csma.h
 *
 * DESCRIPTION: csma header file
 *
 * DATE: Wednesday, Dec 8, 2022
 *
 * AUTHOR:
 *
 */

#ifndef CSMA_H
#define CSMA_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<vector>
#include <fstream>          /* message_file */
#include "node.h"

#define NODE                'N'
#define PACKET_SIZE         'L'
#define MAX_RETX            'R'
#define MAX_RETX_ATTEMPT    'M'
#define TTL_TIME            'T'
#define TYPE_INDEX          0
#define NUM_START_POS       2

int N = -1;
int L = -1;
std::vector<int> R;
int M = -1;
int T = -1;
std::vector<Node> node_vec;
bool chnl_avil = true;

#endif