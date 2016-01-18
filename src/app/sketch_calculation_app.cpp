#include <iostream>
#include <fstream>

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
                                                 std::string* graph_dir,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision")
            ("num_threads", po::value<int>(num_threads)->default_value(1), 
                  "num_threads to use")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("output_file", po::value< std::string > (output_file)->required(), 
                  "output file path, here the sketch Gpb will be saved (Gpb defined in src/proto/all_distance_sketch.proto)")
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
        cout << "#threads=" << *num_threads << endl;
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}


int main(int ac, char* av[]) {
    int K, num_threads;
    bool directed;
    std::string output_file, graph_dir;
    if (parse_command_line_args(ac, av, &K, &num_threads, &directed, &graph_dir, &output_file)) {
        return 1;
    }
    GraphSketch graph_sketch;
    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    calc_graph_sketch(K, num_threads, directed, &graph_sketch,
                      graph_dir, &directed_graph, &un_directed_graph);
    AllDistanceSketchGpb all_distance_sketch;
    graph_sketch.SaveGraphSketchToGpb(&all_distance_sketch);

    {
        fstream output(output_file, ios::out | ios::trunc | ios::binary);
        if (!all_distance_sketch.SerializeToOstream(&output)) {
            cerr << "Failed to write all_distance_sketch_gpb to file=" << output_file << endl;
            return 1;
        }
    }

    return 0;
}