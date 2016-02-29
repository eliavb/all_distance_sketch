#ifndef ALL_DISTANCE_SKETCH_SRC_APP_DIJKSTRA_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_DIJKSTRA_APP_H_

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "../all_distance_sketch.h"
#include "utils.h"
using namespace std;
using namespace boost;
using namespace all_distance_sketch;
namespace po = boost::program_options;


bool parse_command_line_args(int ac, char* av[], int* source_id,
                                                 int* K,
                                                 bool* directed,
                                                 std::string* graph_dir,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("source_id", po::value<int>(source_id)->required(), 
                  "id of source node")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("output_file", po::value< std::string > (output_file)->required(), 
                  "output file path, here the sketch Gpb will be saved")
        ;

        po::positional_options_description p;
        
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
    
        if (vm.count("help")) {
            cout << "Usage: options_description [options]\n";
            cout << desc;
            return true;
        }
        po::notify(vm);
        cout << "source node id=" << *source_id << endl;
        cout << "output_file=" << *output_file << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "directed=" << *directed << endl;
        cout << "K=" << *K << endl;
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

int dijkstra_app_main(int ac, char* av[]) {
    int K, node_id;
    bool directed;
    std::string output_file, graph_dir;
    if (parse_command_line_args(ac, av, &node_id, &K, 
                                        &directed, &graph_dir,
                                        &output_file)) {
        return 1;
    }
    GraphSketch graph_sketch;
    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    
    DijkstraParams param;
    if (directed) {
      graph::TDirectedGraph::TNode source(node_id);
      DefaultDijkstraCallBacks< graph::TDirectedGraph > call_backs;
      PrunedDijkstra< graph::TDirectedGraph, DefaultDijkstraCallBacks< graph::TDirectedGraph > >
                                                                                     (source,
                                                                                      &directed_graph,
                                                                                      &call_backs,
                                                                                      &param);
    } else {
      graph::TUnDirectedGraph::TNode source(node_id);
      DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
      PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &un_directed_graph,
                                                                                      &call_backs,
                                                                                      &param);
    }
    
    NodeRanksGpb node_ranks;
    std::vector<int> dest(param.min_distance.begin(), param.min_distance.end());
    SaveRankingToGpb(node_id, dest , &node_ranks);
    {
        fstream output(output_file, ios::out | ios::trunc | ios::binary);
        if (!node_ranks.SerializeToOstream(&output)) {
            cerr << "Failed to write node_ranks to file=" << output_file << endl;
            return 1;
        }
    }
    return 0;  
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_DIJKSTRA_APP_H_