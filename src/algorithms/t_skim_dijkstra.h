#ifndef SRC_ALGORITHMS_T_SKIM_DIJKSTRA_H_
#define SRC_ALGORITHMS_T_SKIM_DIJKSTRA_H_

#include "./t_skim.h"

namespace all_distance_sketch {

/*! \cond
*/
template <class Z>
class TSkimApproxSeedExactCover : public TSkimBase<Z> {
 public:
  void InitTSkim(int T,
            int K_all_distance_sketch,
            int min_influence_for_seed,
            std::vector<std::vector<int>>* reverse_refernce,
            Cover * cover,
            graph::Graph<Z>* graph) {
    K_all_distance_sketch_ = K_all_distance_sketch;
    graph_sketch_ = NULL;
    reverse_refernce_ = reverse_refernce;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }

  void set_graph_sketch(GraphSketch* graph_sketch) {
    graph_sketch_ = graph_sketch;
  }

  int AddSeed(int seed, std::unordered_map<int, int>* influence_change) {
    LOG_M(DEBUG3, "seed node = " << seed);
    return TSkimBase<Z>::UpdateCover(seed, influence_change, (*reverse_refernce_)[seed]);
  }

  const std::vector<int>& CalculateVisitedNodes(int source_node_id) {
      NodeIdRandomIdData source_node_details(source_node_id, graph_sketch_->GetNodeRandomId(source_node_id));
      NodeSketch * source_sketch = graph_sketch_->GetNodeSketch(source_node_details);
      double distance = source_sketch->GetDistanceCoverNeighborhood(this->T_);
      typename Z::TNode source(source_node_id);
      LOG_M(DEBUG3, "Running from node=" << source_node_id << " T=" << this->T_ << " Distance=" << distance);
      call_backs_.InitTSkimDijkstraCallBacksDistancePrune(distance);
      PrunedDijkstra< Z, TSkimDijkstraCallBacksDistancePrune<Z> > (source,
                                                      TSkimBase<Z>::graph_,
                                                      &call_backs_,
                                                      &param_);
      return call_backs_.get_visited_nodes();
  }

  int Run() {
    TSkimBase<Z>::PreRunInit();
    GraphSketch local_graph_sketch;
    this->graph_->Transpose(&graph_transpose_);
    if (graph_sketch_ == NULL) {
      graph_sketch_ = &local_graph_sketch;
      graph_sketch_->InitGraphSketch(K_all_distance_sketch_, this->graph_->GetMxNId());
      CalculateGraphSketch<Z>(&graph_transpose_, graph_sketch_);
    }
    reverse_rank_call_backs_.InitTSkimReverseRankCallBacks(this->T_);
    reverse_rank_call_backs_.SetWantedNodes(this->wanted_nodes_);
    call_backs_.SetWantedNodes(this->rankees_nodes_);
    return TSkimBase<Z>::Run(false);
  }

 private:
  int K_all_distance_sketch_;
  graph::Graph<Z> graph_transpose_;
  GraphSketch* graph_sketch_;
  TSkimReverseRankCallBacks<Z> reverse_rank_call_backs_;
  DijkstraParams param_;
  TSkimDijkstraCallBacksDistancePrune<Z> call_backs_;
  std::vector<std::vector<int>>* reverse_refernce_;
};

template <class Z>
class TSkimExactComputationBased : public TSkimBase<Z> {
 public:
  void InitTSkim(int T,
                 Cover * cover,
                 std::vector<std::vector<int> >* reachable_nodes,
                 std::vector<std::vector<int> >* reverse_refernce,
                 graph::Graph<Z>* graph) {
    reverse_refernce_ = reverse_refernce;
    reachable_nodes_ = reachable_nodes;
    TSkimBase<Z>::InitTSkimBase(T, INT_MAX, cover, graph);
  }

  int AddSeed(int seed, std::unordered_map<int, int>* influence_change) {
    LOG_M(DEBUG3, "seed node = " << seed);
    return TSkimBase<Z>::UpdateCover(seed, influence_change, (*reverse_refernce_)[seed]);
  }

  const std::vector<int>& CalculateVisitedNodes(int source_node_id) {
      return (*reachable_nodes_)[source_node_id];
  }

 private:
  std::vector<std::vector<int> >* reachable_nodes_;
  std::vector<std::vector<int> >* reverse_refernce_;
};


template <class Z>
class TSkimApproxVsExact : public TSkimBase<Z> {
 public:
  void InitTSkim(int T,
            int K_all_distance_sketch,
            int min_influence_for_seed,
            std::vector<std::vector<int>>* reverse_refernce,
            Cover * cover,
            graph::Graph<Z>* graph) {
    K_all_distance_sketch_ = K_all_distance_sketch;
    graph_sketch_approx = NULL;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }

  int AddSeed(int seed, std::unordered_map<int, int>* influence_change) {
    LOG_M(DEBUG3, "seed node = " << seed);
    std::vector<int> ranking;
    return TSkimBase<Z>::UpdateCover(seed, influence_change, reverse_rank_call_backs_.get_visited_nodes());
  }

  const std::vector<int>& CalculateVisitedNodes(int source_node_id) {
      NodeIdRandomIdData source_node_details(source_node_id, graph_sketch_approx->GetNodeRandomId(source_node_id));
      NodeSketch * source_sketch = graph_sketch_approx->GetNodeSketch(source_node_details);
      double distance = source_sketch->GetDistanceCoverNeighborhood(this->T_);
      typename Z::TNode source(source_node_id);
      LOG_M(DEBUG3, "Running from node=" << source_node_id << " T=" << this->T_ << " Distance=" << distance);
      call_backs_.InitTSkimDijkstraCallBacksDistancePrune(distance);
      PrunedDijkstra< Z, TSkimDijkstraCallBacksDistancePrune<Z> > (source,
                                                      TSkimBase<Z>::graph_,
                                                      &call_backs_,
                                                      &param_);
      return call_backs_.get_visited_nodes();
  }

  int Run() {
    TSkimBase<Z>::PreRunInit();
    GraphSketch local_graph_sketch_approx;
    graph_sketch_approx = &local_graph_sketch_approx;
    graph_sketch_approx->GetNodesDistributionLean();
    this->graph_->Transpose(&graph_transpose_);
    CalculateGraphSketch<Z>(&graph_transpose_, graph_sketch_approx);
    reverse_rank_call_backs_.InitTSkimReverseRankCallBacks(this->T_);
    reverse_rank_call_backs_.SetWantedNodes(this->wanted_nodes_);
    call_backs_.SetWantedNodes(this->rankees_nodes_);
    return TSkimBase<Z>::Run(false);
  }

 private:
  int K_all_distance_sketch_;
  graph::Graph<Z> graph_transpose_;
  GraphSketch* graph_sketch_approx;
  TSkimReverseRankCallBacks<Z> reverse_rank_call_backs_;
  DijkstraParams param_;
  TSkimDijkstraCallBacksDistancePrune<Z> call_backs_;
  std::unordered_map<int , std::set<int> > coveres;
  std::unordered_map<int, int> exact_influence_;
  std::vector<std::vector<int>>* reverse_refernce;
};


template<class Z>
void ExactCoverGreedy(graph::Graph<Z>* graph,
                      int T,
                      std::vector<std::vector<NodeIdDistanceData> >* reachable_nodes) {
  reachable_nodes->resize(graph->GetMxNId());
  CollectorNodesUpToTRank<Z> collect_nodes_call_backs;
  int j = 0;
  for (auto it = graph->BegNI(); it != graph->EndNI(); it++) {
    j++;
    if (j % 1000 == 0) {
      std::cout << j << "/" << graph->GetMxNId() << std::endl;
    }
    int node_id = it.GetId();
    DijkstraParams params;
    typename Z::TNode source(node_id);
    collect_nodes_call_backs.InitCollectorNodesUpToTRank(T+1);
    PrunedDijkstra<Z, CollectorNodesUpToTRank<Z> > (source,
                                                    graph,
                                                    &collect_nodes_call_backs,
                                                    &params);
    (*reachable_nodes)[node_id] = collect_nodes_call_backs.get_nodes_found();
  }
}

/*! \endcond
*/

}  //  namespace all_distance_sketch

#endif  //  SRC_ALGORITHMS_T_SKIM_DIJKSTRA_H_
