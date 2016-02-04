#ifndef ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#include <iostream>
#include <fstream>

#include "../all_distance_sketch.h"

using namespace all_distance_sketch;

int load_sketch(GraphSketch* graph_sketch,
				std::string sketch_file) {
	AllDistanceSketchGpb all_distance_sketch;
    {
        // Read the existing address book.
        std::fstream input(sketch_file, std::ios::in | std::ios::binary);
        if (!input) {
          std::cout << sketch_file << ": File not found." << std::endl;
          return 1;
        }
        if (!all_distance_sketch.ParseFromIstream(&input)) {
          std::cout << "Failed to parse all_distance_sketch." << std::endl;
          return 1;
        }
    }
    graph_sketch->LoadGraphSketchFromGpb(all_distance_sketch);
    return 0;
}

void load_graph(bool directed,
				std::string graph_dir,
                graph::Graph< graph::TDirectedGraph>* directed_graph,
                graph::Graph< graph::TUnDirectedGraph>* un_directed_graph,
                bool load_transpose = false) {
	if (directed) {
        directed_graph->LoadGraphFromDir(graph_dir, load_transpose);
    } else {
        un_directed_graph->LoadGraphFromDir(graph_dir, load_transpose);
    }
}

void calc_graph_sketch(int K,
                       int num_threads,
                       bool directed,
                       GraphSketch* graph_sketch,
                       std::string graph_dir,
                       graph::Graph< graph::TDirectedGraph>* directed_graph,
                       graph::Graph< graph::TUnDirectedGraph>* un_directed_graph) {
    int max_node_id = directed ? directed_graph->GetMxNId() : un_directed_graph->GetMxNId();
    graph_sketch->InitGraphSketch(K, max_node_id);
    if (directed) {
        if (num_threads == 1) {
            CalculateGraphSketch<graph::TDirectedGraph> (directed_graph,
                                                    graph_sketch);
        } else {
            CalculateGraphSketchMultiCore<graph::TDirectedGraph> (directed_graph,
                                                             graph_sketch,
                                                             num_threads);
        }
    } else {
        if (num_threads == 1) {
            CalculateGraphSketch<graph::TUnDirectedGraph> (un_directed_graph,
                                                      graph_sketch);
        } else {
            CalculateGraphSketchMultiCore<graph::TUnDirectedGraph> (un_directed_graph,
                                                               graph_sketch,
                                                             num_threads);
        }
    }
}
#endif  //  ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_