#ifndef ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#include "../all_distance_sketch.h"

using namespace all_distance_sketch;

void load_graph(bool directed,
				std::string graph_dir,
                graph::Graph< graph::TDirectedGraph>* directed_graph,
                graph::Graph< graph::TUnDirectedGraph>* un_directed_graph) {
	if (directed) {
        directed_graph->LoadGraphFromDir(graph_dir);
    } else {
        un_directed_graph->LoadGraphFromDir(graph_dir);
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