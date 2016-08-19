#ifndef SRC_ALGORITHMS_REACH_DIFFUSION_H_
#define SRC_ALGORITHMS_REACH_DIFFUSION_H_

#include "../labels/labels.h"
#include "../measures/decay_interface.h"
#include "../sketch/graph_sketch.h"

namespace all_distance_sketch {

template <class T>
void InitGraphSketchesReachDiffusion(graph::Graph<T>* graph,
            NodesFeaturesSortedContainer* seed_set,
            int K,
            GraphSketch* all_graph_sketch,
            GraphSketch* only_seed_nodes_sketch,
            std::vector<double>* nodes_distribution = NULL) {
  // Init both graph sketches
  all_graph_sketch->InitGraphSketch(K, graph->GetMxNId());
  only_seed_nodes_sketch->InitGraphSketch(K, graph->GetMxNId());
  all_graph_sketch->set_should_calc_zvalues(true);
  only_seed_nodes_sketch->set_should_calc_zvalues(true);
  if (nodes_distribution != NULL) {
    all_graph_sketch->SetNodesDistribution(nodes_distribution);
  }
  // At this point the seed sketch distribution is equal to the full graph distribution (random ids)
  const std::vector<RandomId>* dist = all_graph_sketch->GetNodesDistributionLean();
  std::vector<RandomId> seed_set_distribution;
  seed_set_distribution.resize(graph->GetMxNId(), ILLEGAL_RANDOM_ID);
  for (auto node_info : seed_set->GetNodesFeatures()) {
    int seed_id = node_info.GetNId();
    seed_set_distribution[seed_id] = (*dist)[seed_id];
  }
  only_seed_nodes_sketch->SetNodesDistribution(&seed_set_distribution);
  // Calculated the graph sketch
  CalculateGraphSketchInverseWeight<T>(graph, all_graph_sketch);
  // Run the sketch calculation only from the seet set
  CalculateGraphSketchInverseWeight<T>(graph, only_seed_nodes_sketch);
}

}

#endif  // SRC_ALGORITHMS_REACH_DIFFUSION_H_
