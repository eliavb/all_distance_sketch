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
                                                 // std::string* seed_sketch_file,
                                                 int* K,
                                                 std::string* nodes_distribution,
                                                 int* num_iterations,
                                                 // std::string* sketch_file,
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
            ("K", po::value<int>(K)->required(),
                  "K = 1 / (epsilon^2). Where epsilon is the error")
            ("nodes_distribution", po::value< std::string >(nodes_distribution),
                  "path to file with nodes distribution. CSV file with the first entry is the node id and the second is the random id. default is uniform(0,1)")
            /*
            ("sketch_file", po::value< std::string >(sketch_file),
                  "File prefix with the calculated sketch. The prefix should match what you entered in the sketch_app")
            ("seed_sketch_file", po::value< std::string >(seed_sketch_file),
                  "File prefix with the calculated sketch only on the seed set. The prefix should match what you entered in the sketch_app")*/
            ("num_iterations", po::value< int >(num_iterations)->required(),
                  "number of iterations to run the algorithm")
            ("output_file", po::value< std::string > (output_file)->required(),
                  "output file path, here the embedding will be saved in CSV format. Also partial results will be saved") 
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
        cout << "directed=" << *directed << endl;
        cout << "vector_dim=" << *vector_dim << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "seed_set_file=" << *seed_set_file << endl;
        cout << "nodes_distribution" << *nodes_distribution << endl;
        // cout << "sketch_file=" << *sketch_file << endl;
        // cout << "seed_sketch_file=" << *sketch_file << endl;
        cout << "num_iterations=" << *num_iterations << endl;
        cout << "output_file=" << *output_file << endl;
        
        
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

void single_iteration_distance_diffusion(bool directed, int K, int vector_dim, graph::Graph< graph::TUnDirectedGraph>* un_directed_graph,
                                        graph::Graph< graph::TDirectedGraph>* directed_graph, NodesFeaturesSortedContainer* seed_set,
                                        std::vector<double>* nodes_distribution, NodesFeaturesContainer* result) {
    GraphSketch all_graph_sketch;
    GraphSketch only_seed_nodes_sketch;

    
    int max_node_id = directed ? directed_graph->GetMxNId() : un_directed_graph->GetMxNId();

    if (nodes_distribution != NULL) {
        nodes_distribution->resize(max_node_id, ILLEGAL_RANDOM_ID);
    }

    graph::Graph< graph::TDirectedGraph> directed_random_graph;
    if (directed) {
        create_random_edge_graph<graph::TDirectedGraph, graph::TDirectedGraph, std::exponential_distribution<> >(directed_graph, &directed_random_graph);
    } else {
        create_random_edge_graph<graph::TUnDirectedGraph, graph::TDirectedGraph, std::exponential_distribution<> >(un_directed_graph, &directed_random_graph);
    }
    
    InitGraphSketches<graph::TDirectedGraph> (&directed_random_graph, seed_set, K, &all_graph_sketch, &only_seed_nodes_sketch, nodes_distribution);
    InverseDecay decay_func;
    calculate_labels_distance_diffusion<graph::TDirectedGraph>(&directed_random_graph,
                                                                vector_dim,
                                                                seed_set,
                                                                &decay_func,
                                                                result,
                                                                &all_graph_sketch,
                                                                &only_seed_nodes_sketch);
}

void process_and_write_result(std::vector<NodesFeaturesContainer>* result_node_labels, std::string output_file, int iteration) {
    std::function<double(double)> lambda = [iteration](double a)-> double { return double(a) / (iteration + 1); };
    (*result_node_labels)[iteration].Apply( lambda );
    std::string output_file_current_iteration = output_file + std::to_string(iteration);
    dump_labels_to_csv(output_file_current_iteration, (*result_node_labels)[iteration]);
}

int distance_diffusion_app_main(int ac, char* av[]) {
    bool directed;
    int vector_dim, num_iterations, K;
    std::string output_file, graph_dir, sketch_file, seed_set_file, seed_sketch_file, nodes_distribution_path;
    sketch_file="";
    if (parse_command_line_args(ac, av, &directed, &vector_dim, &graph_dir, &seed_set_file,
                                        &K, &nodes_distribution_path , &num_iterations, &output_file)) {
      return 1;
    }

    std::vector<double>* nodes_distribution = NULL;
    if (nodes_distribution_path.size() != 0) {
        nodes_distribution = new std::vector<double>();
        load_distribution_file_to_vec(nodes_distribution, nodes_distribution_path);
    }

    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    // We will have edges between label -> [node id,...]
    std::vector<NodesFeaturesContainer> result_node_labels;
    result_node_labels.resize(num_iterations);
    NodesFeaturesSortedContainer seed_set;
    load_labels(seed_set_file, &seed_set, vector_dim);
    std::vector<std::thread> threads;
    for (int i=0; i < num_iterations; i++) {
        threads.push_back(std::thread(single_iteration_distance_diffusion, directed, K, vector_dim, &un_directed_graph, &directed_graph, &seed_set, nodes_distribution, &(result_node_labels[i])));
        // single_iteration_distance_diffusion(directed, K, vector_dim, &un_directed_graph, &directed_graph, &seed_set, nodes_distribution, &(result_node_labels[i]));
    }
    for (int i=0; i < threads.size(); i++) {
        threads[i].join();
    }
    threads.clear();
    
    // Aggregate the results
    for (int i=1; i < num_iterations; i++) {
        result_node_labels[i].Add(result_node_labels[i-1]);
    }
    
    for (int i=0; i < num_iterations; i++) {
        // threads.push_back(std::thread(process_and_write_result, &result_node_labels, output_file, i));
        process_and_write_result(&result_node_labels, output_file, i);
    }
    for (int i=0; i < threads.size(); i++) {
        threads[i].join();
    }
    return 0;
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_DISTANCE_DIFFUSION_APP_H_