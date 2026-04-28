#include "graph.hpp"
#include <iostream>
using namespace std;

extern "C" int get_num_nodes(graph* g) { return g->num_nodes(); }

int main() {
    graph g("./test_graph.txt");
    g.parseGraph();
    cout << "num_nodes = " << g.num_nodes() << endl;
    cout << "get_num_nodes = " << get_num_nodes(&g) << endl;
    
    int count = 0;
    for (int v = 0; v < g.num_nodes(); v++) {
        count += 1;
    }
    cout << "Manual loop count = " << count << endl;
}
