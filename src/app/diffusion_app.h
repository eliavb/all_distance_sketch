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
                                                 int* num_seeds_to_consider,
                                                 int* add_constant_weight_to_edge,
                                                 std::string* type,
                                                 std::string* decay_func_rank,
                                                 std::string* decay_func_weights,
                                                 std::string* nodes_distribution,
                                                 int* num_iterations,
                                                 // std::string* sketch_file,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("directed", po::value<bool>(directed)->required(),
                  "is the graph directed")
            ("vector_dim", po::value<int>(vector_dim)->required(),
                  "embedding vector dimension")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("seed_set_file", po::value< std::string >(seed_set_file)->required(),
                  "path to file containing the labels of the seed set. CSV file first entry represents the user and the second entry represents the group index")
            ("K", po::value<int>(K)->required(),
                  "K = 1 / (epsilon^2). Where epsilon is the error")
            ("num_seeds_to_consider", po::value<int>(num_seeds_to_consider)->default_value(constants::INF),
                  "num_seeds_to_consider, default all")
            ("add_constant_weight_to_edge", po::value<int>(add_constant_weight_to_edge)->default_value(0),
                  "Add constant to edge weight, default 0")
            ("nodes_distribution", po::value< std::string >(nodes_distribution)->default_value(""),
                  "path to file with nodes distribution. CSV file with the first entry is the node id and the second is the random id. default is uniform(0,1)")
            ("type", po::value< std::string >(type)->required(),
                  "type of diffusion {distance|reach}")
            ("decay_func_rank", po::value< std::string >(decay_func_rank)->required(),
                  "type of decay for the rank calculation {inverse|inverse_root|inverse_log|inverse_root_log}")
            ("decay_func_weights", po::value< std::string >(decay_func_weights)->required(),
                  "type of decay for the weights. the weights will be calculated using exp(decay(degree)) {inverse|inverse_root|inverse_log|inverse_root_log}")
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
        cout << "K=" << *K << endl;
        cout << "num_seeds_to_consider=" << *num_seeds_to_consider << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "seed_set_file=" << *seed_set_file << endl;
        cout << "add_constant_weight_to_edge" << *add_constant_weight_to_edge << endl;
        cout << "nodes_distribution" << *nodes_distribution << endl;
        cout << "type=" << *type << endl;
        std::cout << "decay_func_rank=" << *decay_func_rank << std::endl;
        std::cout << "decay_func_weights=" << *decay_func_weights << std::endl;
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

DecayInterface* get_decay_function(const std::string& decay_func) {
    if (decay_func == "inverse") {
        return new InverseDecay();
    }
    if (decay_func == "inverse_root") {
        return new InverseRootDecay();
    }
    if (decay_func == "inverse_log") {
        return new InverseLogDecay();
    }
    if (decay_func == "inverse_root_log") {
        return new InverseRootLogDecay();
    }
    return NULL;
}

void single_iteration_distance_diffusion(bool directed, int K, int num_seeds_to_consider, int add_constant_weight_to_edge, const std::string& type, const std::string decay_func_str,
                                        const std::string& decay_func_weights_str, int vector_dim, graph::Graph< graph::TUnDirectedGraph>* un_directed_graph,
                                        graph::Graph< graph::TDirectedGraph>* directed_graph, NodesFeaturesSortedContainer* seed_set,
                                        std::vector<double>* nodes_distribution, NodesFeaturesContainer* result) {
    GraphSketch all_graph_sketch;
    GraphSketch only_seed_nodes_sketch;

    
    int max_node_id = directed ? directed_graph->GetMxNId() : un_directed_graph->GetMxNId();

    if (nodes_distribution != NULL) {
        nodes_distribution->resize(max_node_id, ILLEGAL_RANDOM_ID);
    }

    graph::Graph< graph::TDirectedGraph> directed_random_graph;
    DecayInterface* decay_func_weights = get_decay_function(decay_func_weights_str);
    if (directed) {
        create_random_edge_graph<graph::TDirectedGraph, graph::TDirectedGraph, std::exponential_distribution<> >(directed_graph, &directed_random_graph, add_constant_weight_to_edge, decay_func_weights);
    } else {
        create_random_edge_graph<graph::TUnDirectedGraph, graph::TDirectedGraph, std::exponential_distribution<> >(un_directed_graph, &directed_random_graph, add_constant_weight_to_edge, decay_func_weights);
    }
    if (type == "distance") {
        InitGraphSketchesDistanceDiffusion<graph::TDirectedGraph> (&directed_random_graph, seed_set, K, &all_graph_sketch, &only_seed_nodes_sketch, nodes_distribution);
    }
    else if (type == "reach") {
        InitGraphSketchesReachDiffusion<graph::TDirectedGraph> (&directed_random_graph, seed_set, K, &all_graph_sketch, &only_seed_nodes_sketch, nodes_distribution);
    } else {
        std::cout << "Unkonw type, please see type option for help" << std::endl;
        return;
    }
    
    DecayInterface* decay_func = get_decay_function(decay_func_str);
    calculate_labels_diffusion<graph::TDirectedGraph>(&directed_random_graph,
                                                                vector_dim,
                                                                seed_set,
                                                                decay_func,
                                                                result,
                                                                &all_graph_sketch,
                                                                &only_seed_nodes_sketch,
                                                                num_seeds_to_consider);
}

void process_and_write_result(std::vector<NodesFeaturesContainer>* result_node_labels, std::string output_file, int iteration) {
    std::function<double(double)> lambda = [iteration](double a)-> double { return double(a) / (iteration + 1); };
    (*result_node_labels)[iteration].Apply( lambda );
    std::string output_file_current_iteration = output_file + std::to_string(iteration);
    dump_labels_to_csv(output_file_current_iteration, (*result_node_labels)[iteration]);
}

int diffusion_app_main(int ac, char* av[]) {
    bool directed;
    int vector_dim, num_iterations, K, num_seeds_to_consider, add_constant_weight_to_edge;
    std::string output_file, graph_dir, seed_set_file, nodes_distribution_path, type, decay_func_rank, decay_func_weights;
    if (parse_command_line_args(ac, av, &directed, &vector_dim, &graph_dir, &seed_set_file,
                                        &K, &num_seeds_to_consider, &add_constant_weight_to_edge, &type, &decay_func_rank, &decay_func_weights,
                                        &nodes_distribution_path , &num_iterations, &output_file)) {
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
    std::cout << "nodes=" << un_directed_graph.GetNumNodes() << std::endl;
    std::cout << "edges=" << un_directed_graph.GetNumEdges() << std::endl;
    // We will have edges between label -> [node id,...]
    std::vector<NodesFeaturesContainer> result_node_labels;
    result_node_labels.resize(num_iterations);
    NodesFeaturesSortedContainer seed_set;
    load_labels(seed_set_file, &seed_set, vector_dim);
    std::vector<std::thread> threads;
    for (int i=0; i < num_iterations; i++) {
        threads.push_back(std::thread(single_iteration_distance_diffusion, directed, K, num_seeds_to_consider,
                                      add_constant_weight_to_edge, type, decay_func_rank, decay_func_weights, vector_dim, &un_directed_graph, &directed_graph, 
                                      &seed_set, nodes_distribution, &(result_node_labels[i])));
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