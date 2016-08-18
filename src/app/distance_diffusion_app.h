#ifndef ALL_DISTANCE_SKETCH_SRC_APP_DISTANCE_DIFFUSION_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_DISTANCE_DIFFUSION_APP_H_

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "../all_distance_sketch.h"
#include "utils.h"
using namespace std;
using namespace boost;
using namespace all_distance_sketch;
namespace po = boost::program_options;


bool parse_command_line_args(int ac, char* av[], bool* directed,
                                                 int* vector_dim,
                                                 std::string* graph_dir,
                                                 std::string* seed_set_file,
                                                 std::string* seed_sketch_file,
                                                 int* num_iterations,
                                                 std::string* sketch_file,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("vector_dim", po::value<int>(vector_dim), 
                  "embedding vector dimension")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("seed_set_file", po::value< std::string >(seed_set_file)->required(),
                  "path to file containing the labels of the seed set. CSV file first entry represents the user and the second entry represents the group index")
            ("sketch_file", po::value< std::string >(sketch_file),
                  "File prefix with the calculated sketch. The prefix should match what you entered in the sketch_app")
            ("seed_sketch_file", po::value< std::string >(seed_sketch_file),
                  "File prefix with the calculated sketch only on the seed set. The prefix should match what you entered in the sketch_app")
            ("num_iterations", po::value< int >(num_iterations),
                  "number of iterations to run the algorithm")
            ("output_file", po::value< std::string > (output_file)->required(),
                  "output file path, here the embedding will be saved in Gpb format (Gpb defined in src/proto/embedding.proto)") 
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
        cout << "sketch_file=" << *sketch_file << endl;
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

int distance_diffusion_app_main(int ac, char* av[]) {
    bool directed;
    int vector_dim, num_iterations;
    std::string output_file, graph_dir, sketch_file, seed_set_file, seed_sketch_file;
    sketch_file="";
    if (parse_command_line_args(ac, av, &directed, &vector_dim, &graph_dir, &seed_set_file,
                                        &seed_sketch_file, &num_iterations, &sketch_file, &output_file)) {
      return 1;
    }

    // We will have edges between label -> [node id,...]
    NodesFeaturesSortedContainer seed_set;
    load_labels(seed_set_file, &seed_set, vector_dim);
    for (int i=0; i < num_iterations; i++) {
        LOG_M(NOTICE, "Seed set labels size after load =" << seed_set.Size());
        GraphSketch all_graph_sketch;
        load_sketch(&all_graph_sketch, sketch_file);
        LOG_M(NOTICE, "Size of full graph sketch after load= " << all_graph_sketch.GetNodesSketch().size());

        GraphSketch only_seed_nodes_sketch;
        load_sketch(&only_seed_nodes_sketch, seed_sketch_file);
        LOG_M(NOTICE, "Size of seed nodes sketch after load= " << only_seed_nodes_sketch.GetNodesSketch().size());

        graph::Graph< graph::TDirectedGraph> directed_graph;
        graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
        load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
        
        graph::Graph< graph::TDirectedGraph> directed_random_graph;
        graph::Graph< graph::TUnDirectedGraph> un_directed_random_graph;
        if (directed) {
            create_random_edge_graph<graph::TDirectedGraph, std::exponential_distribution<> >(&directed_graph, &directed_random_graph);    
        } else {
            create_random_edge_graph<graph::TunDirectedGraph, std::exponential_distribution<> >(&un_directed_graph, &un_directed_random_graph);    
        }
        

        NodesFeaturesContainer result_node_labels;
        InverseDecay decay_func;
        if (directed) {
          calculate_labels_distance_diffusion<graph::TDirectedGraph>(&directed_random_graph,
                                                                    vector_dim,
                                                                    &seed_set,
                                                                    &decay_func,
                                                                    &result_node_labels,
                                                                    &all_graph_sketch,
                                                                    &only_seed_nodes_sketch);
        } else {
          calculate_labels_distance_diffusion<graph::TUnDirectedGraph>(&un_directed_random_graph,
                                                                    vector_dim,
                                                                    &seed_set,
                                                                    &decay_func,
                                                                    &result_node_labels,
                                                                    &all_graph_sketch,
                                                                    &only_seed_nodes_sketch);
        }

        // TODO (eliav) : Add averaging

        EmbeddingGpb embedding;
        result_node_labels.SaveToGpb(&embedding);
        std::filebuf fb;
        fb.open (output_file, std::ios::out);
        std::ostream os(&fb);
        embedding.SerializeToOstream(&os);    
    }
    
    return 0;
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_DISTANCE_DIFFUSION_APP_H_