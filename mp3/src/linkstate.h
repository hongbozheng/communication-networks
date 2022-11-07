/**
 * FILENAME: linkstate.h
 *
 * DESCRIPTION: linkstate header file
 *              map topo       ----- topology map
 *              vector msg_vec ----- message vector
 *
 * DATE: Saturday, Nov 5, 2022
 *
 * AUTHOR:
 *
 */

#ifndef LINKSTATE_H
#define LINKSTATE_H

#include <map>
#include <fstream>
#include "route.h"

std::map<int, node_t*> topo;
std::vector<msg_t*> msg_vec;

#endif