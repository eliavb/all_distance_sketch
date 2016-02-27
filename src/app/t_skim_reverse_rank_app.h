#ifndef ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_REVERSE_RANK_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_REVERSE_RANK_APP_H_

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
                                                 int* K,
                                                 int* min_influence_for_seed_set,
                                                 int* num_threads,
                                                 bool* directed,
                                                 std::string* graph_dir,
                                                 std::string* sketch_file,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("T", po::value<int>(T)->required(), 
                  "Defines the influence of a node. If pi_{us} < T then s coveres u")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision")
            ("num_threads", po::value<int>(num_threads)->default_value(1), 
                  "num_threads to use")
            ("min_influence_for_seed_set", po::value<int>(min_influence_for_seed_set), 
                  "min influence to enter to the seed set")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("sketch_file", po::value< std::string >(sketch_file),
                  "File with the calculated sketch")
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
        cout << "sketch_file=" << *sketch_file << endl;
        cout << "output_file=" << *output_file << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "directed=" << *directed << endl;
        cout << "K=" << *K << endl;
        cout << "min_influence_for_seed_set=" << *min_influence_for_seed_set << endl;
        cout << "num_threads=" << *num_threads << endl;
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

int t_skim_app_main(int ac, char* av[]) {
    int K, min_influence_for_seed_set, T, num_threads;
    bool directed;
    std::string output_file, graph_dir, sketch_file;
    sketch_file="";
    if (parse_command_line_args(ac, av, &T, &K, &min_influence_for_seed_set,
                                        &num_threads, &directed, &graph_dir,
                                        &sketch_file, &output_file)) {
        return 1;
    }
    GraphSketch graph_sketch;
    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    if (sketch_file == "") {
        calc_graph_sketch(K, num_threads, directed, &graph_sketch,
                          graph_dir, &directed_graph, &un_directed_graph);
    } else {
        load_sketch(&graph_sketch, sketch_file);
    }

    TSkimReverseRank< graph::TDirectedGraph > t_skim_algo_directed;
    TSkimReverseRank< graph::TUnDirectedGraph > t_skim_algo_un_directed;
    Cover cover;
    /*
    InitTSkim(int T,
            int k_all_distance_sketch,
            int K,
            Cover * cover,
            graph::Graph<Z>* graph)
    */
    if (directed) {
        t_skim_algo_directed.InitTSkim(T, K, min_influence_for_seed_set, &cover, &directed_graph);
        t_skim_algo_directed.Run();
    } else {
        t_skim_algo_un_directed.InitTSkim(T, K, min_influence_for_seed_set, &cover, &un_directed_graph);
        t_skim_algo_un_directed.Run();
    }

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

#endif // ALL_DISTANCE_SKETCH_SRC_APP_T_SKIM_REVERSE_RANK_APP_H_