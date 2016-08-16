#ifndef ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include <boost/program_options.hpp>


#include "../all_distance_sketch.h"
#include "utils.h"
using namespace std;
using namespace boost;
using namespace all_distance_sketch;
namespace po = boost::program_options;


bool parse_command_line_args(int ac, char* av[], int* K,
                                                 int* num_threads,
                                                 bool* directed,
                                                 bool* should_calc_insert_prob,
                                                 std::string* nodes_to_run,
                                                 std::string* nodes_distribution,
                                                 std::string* graph_dir,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision of the approximation")
            ("num_threads", po::value<int>(num_threads)->default_value(1), 
                  "num_threads to use")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("should_calc_insert_prob", po::value<bool>(should_calc_insert_prob), 
                  "Should we calculate the insertion probability of nodes. default false")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("nodes_to_run", po::value< std::string >(nodes_to_run),
                  "path to file with node ids. Only from the nodes specified we will run the algorithm. default the algorithm will run from every node")
            ("nodes_distribution", po::value< std::string >(nodes_distribution),
                  "path to file with nodes distribution. CSV file with the first entry is the node id and the second is the random id. default is uniform(0,1)")
            ("output_file", po::value< std::string > (output_file)->required(), 
                  "output file path pattern, here the sketch Gpb will be saved with the prefix entered and suffix _[%d] (Gpb defined in src/proto/all_distance_sketch.proto)")
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
        cout << "output_file=" << *output_file << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "directed=" << *directed << endl;
        cout << "K=" << *K << endl;
        cout << "should_calc_insert_prob=" << *should_calc_insert_prob << endl;
        cout << "#threads=" << *num_threads << endl;
        if (nodes_to_run->size() != 0 && nodes_distribution->size() == 0) {
            std::cout << " when specifing nodes_to_run must also provide nodes_distribution" << std::endl;
            return true;
        }
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

int sketch_app_main(int ac, char*av[]) {
    int K, num_threads;
    bool directed, should_calc_insert_prob = false;
    std::string output_file, graph_dir, nodes_to_run_path, nodes_distribution_path;
    if (parse_command_line_args(ac, av, &K, &num_threads, &directed, &should_calc_insert_prob, &nodes_to_run_path, &nodes_distribution_path, &graph_dir, &output_file)) {
        return 1;
    }
    std::vector<RandomId> seed_set_distribution;
    std::vector<double>* nodes_distribution = NULL;
    if (nodes_distribution_path.size() != 0) {
        nodes_distribution = new std::vector<double>();
        load_distribution_file_to_vec(nodes_distribution, nodes_distribution_path);
    }

    std::vector<int>* nodes_to_run = NULL;
    if (nodes_to_run_path.size() != 0) {
        nodes_to_run = new std::vector<int>();
        load_file_to_vec(nodes_to_run, nodes_to_run_path);
        seed_set_distribution.resize(nodes_distribution->size(), ILLEGAL_RANDOM_ID);
        for (int i=0; i < nodes_to_run->size(); i++) {
            int seed_id = (*nodes_to_run)[i];
            seed_set_distribution[seed_id] = (*nodes_distribution)[seed_id];
        }
        nodes_distribution = &seed_set_distribution;
    } 

    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    int max_node_id = directed ? directed_graph.GetMxNId() : un_directed_graph.GetMxNId();

    GraphSketch graph_sketch;
    graph_sketch.InitGraphSketch(K, max_node_id, nodes_to_run);
    graph_sketch.set_should_calc_zvalues(should_calc_insert_prob);
    if (nodes_distribution != NULL) {
        graph_sketch.SetNodesDistribution(nodes_distribution);
    }

    bool is_graph_sketch_initalized=true;
    calc_graph_sketch(K, num_threads, directed, &graph_sketch,
                      graph_dir, &directed_graph, &un_directed_graph, is_graph_sketch_initalized);

    AllDistanceSketchGpb all_distance_sketch;
    graph_sketch.SaveGraphSketchToGpb(&all_distance_sketch);
    DumpGraphSketchToFile(all_distance_sketch, output_file);
    return 0;
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_