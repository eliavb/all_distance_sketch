#ifndef ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_EXACT_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_EXACT_APP_H_

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "../all_distance_sketch.h"
#include "utils.h"
using namespace std;
using namespace boost;
using namespace all_distance_sketch;
namespace po = boost::program_options;


bool parse_command_line_args(int ac, char* av[], int* T,
                                                 bool* directed,
                                                 std::string* graph_dir,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("T", po::value<int>(T)->required(), 
                  "Defines the influence of a node. If pi_{us} < T then s coveres u")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("output_file", po::value< std::string > (output_file)->required(), 
                  "output file path, here the cover will be saved in Gpb format (Gpb defined in src/proto/cover.proto)") 
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
        cout << "T=" << *T << endl;
        cout << "output_file=" << *output_file << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "directed=" << *directed << endl;
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

template<class Z>
void calculate_exact_data_structures(graph::Graph<Z>* graph,
                                    int T,
                                    std::vector<std::vector<int> >* reachable_nodes,
                                    std::vector<std::vector<int> >* reverse_refernce) {
    reachable_nodes->resize(graph->GetMxNId());
    reverse_refernce->resize(graph->GetMxNId());
    for (auto node_it = graph->BegNI(); node_it != graph->EndNI(); node_it++ ) {
        int source_node_id = node_it.GetId();
        CollectorNodesUpToUpperBoundRankRank<Z> collector;
        collector.InitCollectorNodesUpToUpperBoundRankRank(T+1);
        DijkstraParams param;
        typename Z::TNode source(source_node_id);
        PrunedDijkstra< Z, CollectorNodesUpToUpperBoundRankRank< Z > > (source,
                                                          graph,
                                                          &collector,
                                                          &param);

        for (const auto node_details : collector.get_nodes_found()) {
            const auto reachable_node_id = node_details.GetNId();
            (*reachable_nodes)[source_node_id].push_back(reachable_node_id);
            (*reverse_refernce)[reachable_node_id].push_back(source_node_id);
        }
    }
}

void t_skim_exact(Cover* cover, bool directed, int T, 
                  graph::Graph< graph::TDirectedGraph>* directed_graph,
                  graph::Graph< graph::TUnDirectedGraph>* un_directed_graph) {
    std::vector<std::vector<int> > reachable_nodes;
    std::vector<std::vector<int> > reverse_refernce;
    TSkimExactComputationBased< graph::TDirectedGraph > t_skim_algo_directed;
    TSkimExactComputationBased< graph::TUnDirectedGraph > t_skim_algo_un_directed;
    if (directed) {
        calculate_exact_data_structures<graph::TDirectedGraph>(directed_graph, T, &reachable_nodes, &reverse_refernce);
    } else {
        calculate_exact_data_structures<graph::TUnDirectedGraph>(un_directed_graph, T, &reachable_nodes, &reverse_refernce);
    }
    if (directed) {
        t_skim_algo_directed.InitTSkim(T, cover, &reachable_nodes, &reverse_refernce, directed_graph);
        t_skim_algo_directed.Run();
    } else {
        t_skim_algo_un_directed.InitTSkim(T, cover, &reachable_nodes, &reverse_refernce, un_directed_graph);
        t_skim_algo_un_directed.Run();
    }
}

int t_skim_exact_app_main(int ac, char* av[]) {
    int T;
    bool directed;
    std::string output_file, graph_dir;
    if (parse_command_line_args(ac, av, &T, &directed, &graph_dir, &output_file)) {
        return 1;
    }
    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);

    Cover cover;
    t_skim_exact(&cover, directed, T, &directed_graph, &un_directed_graph);
    CoverGpb coverGpb;
    cover.SaveGraphSketchToGpb(&coverGpb);
    {
        fstream output(output_file, ios::out | ios::trunc | ios::binary);
        if (!coverGpb.SerializeToOstream(&output)) {
            cerr << "Failed to write node_ranks to file=" << output_file << endl;
            return 1;
        }
    }
    return 0;
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_EXACT_APP_H_