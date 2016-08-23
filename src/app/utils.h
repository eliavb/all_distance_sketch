#ifndef ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_
#include <iostream>
#include <fstream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "../all_distance_sketch.h"

using namespace all_distance_sketch;
using namespace google::protobuf::io;

static const int kMessageSizeLimit = 1000000000;

int load_sketch(GraphSketch* graph_sketch,
				std::string sketch_file) {

	AllDistanceSketchGpb all_distance_sketch;
  LoadGraphSketchFromFiles(&all_distance_sketch, sketch_file);
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

// TODO (eliav) : Add alpha interface to allow different decay functions
template<class T, class M, typename Z>
void create_random_edge_graph(graph::Graph<T>* graph, graph::Graph<M>* graph_out, int add_constant_weight_to_edge) {
  std::random_device rd;
  std::mt19937 gen(rd());

  for (auto it=graph->BegNI(); it != graph->EndNI(); it++) {
    auto node_id = it.GetId();
    graph_out->AddNode(node_id);
  }
  for (auto it=graph->BegNI(); it != graph->EndNI(); it++) {
    auto node_id = it.GetId();
    auto vertex = graph->GetNI(node_id);
    int node_out_deg = vertex.GetOutDeg();
    Z d(1 / double(node_out_deg));
    for (int i = 0 ; i < vertex.GetOutDeg(); i++) {
      int n_id = vertex.GetOutNId(i);
      double random_edge_weight = d(gen);
      LOG_M(DEBUG5, " edge between " << node_id << "->" << n_id << " edge weight=" << random_edge_weight);  
      graph_out->AddEdge(node_id, n_id, random_edge_weight + add_constant_weight_to_edge);
    }
  }
}

void load_graph_file(bool directed,
                std::string graph_dir,
                std::string delimiter,
                graph::Graph< graph::TDirectedGraph>* directed_graph,
                graph::Graph< graph::TUnDirectedGraph>* un_directed_graph,
                bool load_transpose = false) {
  if (directed) {
        directed_graph->LoadGraphFromFile(graph_dir, delimiter, load_transpose);
    } else {
        un_directed_graph->LoadGraphFromFile(graph_dir, delimiter, load_transpose);
    }
}

void load_labels(std::string seed_set_file, NodesFeaturesSortedContainer* seed_set, int vector_dim) {
  graph::Graph< graph::TDirectedGraph> directed_seed_set_graph;
  load_graph_file(true, seed_set_file, ",", &directed_seed_set_graph, NULL);
  LOG_M(NOTICE, "finished loading labels graph");
  FEATURE_WEIGHTS_VECTOR node_vector;
  for (auto node_it = directed_seed_set_graph.BegNI(); node_it != directed_seed_set_graph.EndNI(); node_it++ ) {
    node_vector.resize(vector_dim, 0);
    auto node_id = node_it.GetId();
    LOG_M(DEBUG5, "node_id=" << node_id);
    auto vertex = directed_seed_set_graph.GetNI(node_id);
    // Create seed set embedding
    double denominator = vertex.GetOutDeg();
    for (int i = 0 ; i < vertex.GetOutDeg(); i++) {
      int label_id = vertex.GetOutNId(i);
      LOG_M(DEBUG5, "label_id=" << label_id);
      // Normalize the vector
      node_vector[label_id] = 1 / denominator;
    }
    seed_set->AddNodeFeature(node_id, node_vector);
    node_vector.clear();
  }
}

void calc_graph_sketch(int K,
                       int num_threads,
                       bool directed,
                       GraphSketch* graph_sketch,
                       std::string graph_dir,
                       graph::Graph< graph::TDirectedGraph>* directed_graph,
                       graph::Graph< graph::TUnDirectedGraph>* un_directed_graph,
                       bool is_graph_sketch_initalized=false) {
    int max_node_id = directed ? directed_graph->GetMxNId() : un_directed_graph->GetMxNId();
    // Gives to the possibility to do custom initialization from outside
    if (is_graph_sketch_initalized == false) {
      graph_sketch->InitGraphSketch(K, max_node_id);
    }
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

void load_file_to_vec(std::vector<int>* vec, std::string file_path) {
  std::ifstream infile(file_path);
  std::string line;
  while (std::getline(infile, line)) {
    int node_id = atoi(line.c_str());
    // std::cout << "loaded node id=" << node_id << std::endl;
    vec->push_back(node_id);
  }
}

void load_distribution_file_to_vec(std::vector<double>* vec, std::string file_path) {
  std::ifstream infile(file_path);
  std::string line;
  vec->clear();
  while (std::getline(infile, line)) {
    // std::cout << line << std::endl;
    std::stringstream linestream(line);
    std::string cell;
    std::getline(linestream, cell, ',');
    int node_id = atoi(cell.c_str());
    std::getline(linestream, cell, ',');
    double random_id = atof(cell.c_str());
    if (node_id >= vec->size()) {
      vec->resize(node_id+1, ILLEGAL_RANDOM_ID);  
    }
    (*vec)[node_id] = random_id;
  }
}

void dump_labels_to_csv(std::string f_name,
                        const NodesFeaturesContainer& labels) {
  std::ofstream ofs;
  ofs.open (f_name, std::ofstream::out);
  auto nodes_labels = labels.GetNodesFeatures();
  for (int i=0; i < nodes_labels.size(); i++) {
    int node_id = nodes_labels[i].GetNId();
    ofs << std::to_string(node_id);
    auto label_weights = nodes_labels[i].GetFeatureWeights();
    for (int j=0; j < label_weights.size(); j++) {
      ofs << ",";
      ofs << std::to_string(label_weights[j]);
    }
    ofs << "\n";
  }
  ofs.close();
}

#endif  //  ALL_DISTANCE_SKETCH_SRC_APP_UTILS_H_