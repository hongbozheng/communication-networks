#include "linkstate.h"

void create_topo(FILE *fp) {
    int src, dst, cost;
    while (fscanf(fp, "%d", &src) != EOF) {
        fscanf(fp, "%d %d", &dst, &cost);
        if (src != dst || cost != 0) {
            topo[src][dst] = cost;
            topo[dst][src] = cost;
        }
        num_node = max(num_node, max(src, dst));
    }
}

void create_msg_vec(std::ifstream &msg_file) {
    string line;
    int src, dst;

    while (getline(msg_file, line)) {
        sscanf(line.c_str(), "%d %d %*s", &src, &dst);
        line = line.substr(line.find(' ')+1);
        line = line.substr(line.find(' ')+1);
        msg_vec.push_back(make_pair(src, make_pair(dst, line)));
    }
}

void get_chg(FILE *fp) {
    int src, dst, cost;
    while (fscanf(fp, "%d", &src) != EOF) {
        fscanf(fp, "%d %d", &dst, &cost);
        chg_vec.push_back(make_pair(src, make_pair(dst, cost)));
    }
}

void init() {
    for (int i = 0; i < num_node + 1; i++) {
        for (int k = 0; k < num_node + 1; k++) {
            if (i == k) {
                dist[i][k] = 0;
            } else {
                if (topo[i].find(k) != topo[i].end()) {
                    dist[i][k] = topo[i][k];
                } else {
                    dist[i][k] = INFINITY;
                }
            }
        }
    }
    for (int i = 0; i < num_node + 1; i++) {
        for (int k = 0; k < num_node + 1; k++) {
            if (i == k) {
                next_hop[i][k] = i;
            } else {
                if (topo[i].find(k) != topo[i].end()) {
                    next_hop[i][k] = k;
                } else {
                    next_hop[i][k] = INFINITY;
                }
            }
        }
    }
    for (int i = 0; i < num_node + 1; i++) {
        for (int k = 0; k < num_node + 1; k++) {
            if (i == k) {
                prev_hop[i][k] = i;
            } else {
                if (topo[i].find(k) != topo[i].end()) {
                    prev_hop[i][k] = i;
                } else {
                    prev_hop[i][k] = INFINITY;
                }
            }
        }
    }
}

void upd_pq(priority_queue<int, vector<int>, cmp_dist>& pq) {
    vector<int> tmp;
    while (!pq.empty()) {
        tmp.push_back(pq.top());
        pq.pop();
    }
    for (int & i : tmp) {
        pq.push(i);
    }
}

void dijsktra() {
    for (int node = 1; node <= num_node; node++) {
        priority_queue<int, vector<int>, cmp_dist> pq;
        tmp_dist.clear();
        for (int i = 1; i <= num_node; i++) {
            if (i == node) {
                continue;
            }
            if (topo[node].find(i) != topo[node].end() && topo[node][i] != -999) {
                tmp_dist[i] = topo[node][i];
                pq.push(i);

            } else {
                tmp_dist[i] = INFINITY;
                pq.push(i);
            }
        }

        while (!pq.empty()) {
            int w = pq.top();
            pq.pop();

            for (int v = 1; v < num_node + 1; v++) {
                if (topo[w].find(v) != topo[w].end()) {
                    if (dist[node][w] + topo[w][v] == dist[node][v]) {
                        next_hop[node][v] = w < prev_hop[node][v] ? next_hop[node][w] : next_hop[node][v];

                    } else {
                        next_hop[node][v] = dist[node][w] + topo[w][v] > dist[node][v] ? next_hop[node][v] : next_hop[node][w];
                        prev_hop[node][v] = dist[node][w] + topo[w][v] > dist[node][v] ? prev_hop[node][v] : w;
                    }
                    dist[node][v] = min(dist[node][w] + topo[w][v], dist[node][v]);
                    tmp_dist[v] = min(dist[node][w] + topo[w][v], dist[node][v]);
                    upd_pq(pq);
                }
            }
        }
    }
}

void w_topo(FILE * fp) {
    for (int src = 1; src <= num_node; src++) {
        for (int dst = 1; dst <= num_node; dst++) {
            if (dist[src][dst] == INFINITY) {
                continue;
            }
            string topo_str = to_string(dst) + ' ' + to_string(next_hop[src][dst]) + ' ' + to_string(dist[src][dst]) + '\n';
            const char* topo_ptr = topo_str.data();
            fputs(topo_ptr, fp);
        }
    }

}

string get_route(int src, int dst) {
    string route = to_string(src);
    int cur = src;
    while (next_hop[cur][dst] != dst) {
        cur = next_hop[cur][dst];
        route += ' ' + to_string(cur);
    }
    return route;
}

void w_msg(FILE * fp) {
    for (auto & i : msg_vec) {
        int src = i.first;
        int dst = i.second.first;
        string msg = i.second.second;
        if (dist[src][dst] == INFINITY) {
            string message = "from " + to_string(src) + " to " + to_string(dst) + " cost infinite hops unreachable message " + msg + '\n';
            const char* msg_ptr = message.data();
            fputs(msg_ptr, fp);
        } else {
            string message = "from " + to_string(src) + " to " + to_string(dst) + " cost " + to_string(dist[src][dst]) + " hops " + get_route(src, dst) + " message " + msg + '\n';
            const char* msg_ptr = message.data();
            fputs(msg_ptr, fp);
        }
    }
}

void w_output_file(FILE * fp) {
    init();
    dijsktra();
    w_topo(fp);
    w_msg(fp);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("[USAGE]: ./linkstate <topology_file> <message_file> <changes_file>\n");
        return -1;
    }

    FILE *topo_fp = fopen(argv[1], "r");
    create_topo(topo_fp);
    fclose(topo_fp);
    ifstream msg_file;
    msg_file.open(argv[2], ifstream::in);
    create_msg_vec(msg_file);
    msg_file.close();
    FILE *chg_fp = fopen(argv[3], "r");
    get_chg(chg_fp);
    fclose(chg_fp);

    FILE *fpOut;
    fpOut = fopen(OUTPUT_FILENAME, "w");
    w_output_file(fpOut);
    for (auto & i : chg_vec) {
        if (i.second.second == -999) {
            topo[i.first][i.second.first] = INFINITY;
            topo[i.second.first][i.first] = INFINITY;
    
        } else {
            topo[i.first][i.second.first] = i.second.second;
            topo[i.second.first][i.first] = i.second.second;
        }
        w_output_file(fpOut);
    }
    fclose(fpOut);
    return 0;
}