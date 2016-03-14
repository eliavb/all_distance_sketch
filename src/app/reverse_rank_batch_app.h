#ifndef ALL_DISTANCE_SKETCH_SRC_APP_REVERSE_RANK_BATCH_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_REVERSE_RANK_BATCH_APP_H_

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
                                                 std::string* sources,
                                                 std::string* graph_dir,
                                                 std::string* sketch_file,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("sources", po::value<std::string>(sources)->required(), 
                  "sources file")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision")
            ("num_threads", po::value<int>(num_threads)->default_value(1), 
                  "num_threads to use")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("sketch_file", po::value< std::string >(sketch_file),
                  "File with the calculated sketch")
            ("output_dir", po::value< std::string > (output_file)->required(), 
                  "output directory path, here the sketch Gpbs will be saved")
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
        cout << "sources=" << *sources << endl;
        cout << "sketch_file=" << *sketch_file << endl;
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

void get_vector_from_file(std::string file_path,
                          std::vector<int>* nodes) {
  std::ifstream input(file_path);
  for( std::string line; getline( input, line ); ) {
      int node_id =  std::stoi(line);
      nodes->push_back(node_id);
  }
}

int reverse_rank_app_main(int ac, char* av[]) {
    int K, num_threads;
    bool directed;
    std::string output_file, graph_dir, sketch_file, sources;
    sketch_file="";
    if (parse_command_line_args(ac, av, &K, &num_threads, 
                                        &directed, &sources, &graph_dir,
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

    graph::Graph< graph::TDirectedGraph> directed_graph_transpose;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph_transpose;
    if (directed) {
        directed_graph.Transpose(&directed_graph_transpose);
    } else {
        un_directed_graph.Transpose(&un_directed_graph_transpose);
    }


    std::vector<int> nodes;
    get_vector_from_file(sources, &nodes);
    for (const auto node_id : nodes ) {
      std::string file_name = output_file + std::to_string(node_id) + "_r";
      std::vector<int> ranking;
      if (directed)
          CalculateReverseRank<graph::TDirectedGraph> (node_id,
                                                       &directed_graph_transpose,
                                                       &graph_sketch,
                                                       &ranking);
      else {
          CalculateReverseRank<graph::TUnDirectedGraph> (node_id,
                                                       &un_directed_graph_transpose,
                                                       &graph_sketch,
                                                       &ranking);
      }
      NodeRanksGpb node_ranks;
      SaveRankingToGpb(node_id, ranking, &node_ranks);
      {
          fstream output(file_name, ios::out | ios::trunc | ios::binary);
          if (!node_ranks.SerializeToOstream(&output)) {
              cerr << "Failed to write node_ranks to file=" << file_name << endl;
              return 1;
          }
      }
  }
  return 0;   
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_REVERSE_RANK_BATCH_APP_H_