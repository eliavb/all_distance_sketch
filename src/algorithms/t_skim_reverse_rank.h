#ifndef ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
#define ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_

#include "t_skim.h"


/*! \file t_skim_reverse_rank.h
    \brief Contains all TSkim influence maximization algorithms
*/
namespace all_distance_sketch {

/*! \cond
*/
template<class Z>
class TSkimReverseRankCallBacks {
 public:
  void SetWantedNodes(const std::unordered_map<int, bool>& nodes) {
    wanted_nodes_ = nodes;
  }
  void InitTSkimReverseRankCallBacks(int T) {
    T_ = T;
    num_nodes_visited_ = 0;
    wanted_nodes_.clear();
    delta_.clear();
    visited_nodes_including_self_.clear();
  }

  void PrepareForIteration() {
    num_nodes_visited_ = 0;
    visited_nodes_including_self_.clear();
  }

  inline void Started(int source_node_id, graph::Graph<Z>* graph) {
    LOG_M(DEBUG3, "Source node id=" << source_node_id);
    source_node_id_ = source_node_id;
    graph_ = graph;
  }

  inline void NodePopedFromHeap(int poped_node_id, const RankData& heap_value) {
    if (wanted_nodes_.size() != 0 && wanted_nodes_.count(poped_node_id) == 0) {
      return;
    }
    LOG_M(DEBUG3, "poped node=" << poped_node_id);
    if (T_ >= heap_value.rank) {
      LOG_M(DEBUG3, "Inserted node to visited nodes");
      visited_nodes_including_self_.push_back(poped_node_id);
    }
    ++num_nodes_visited_;
    return;
  }

  inline bool ShouldPrune(int visited_node_id, const RankData& rank_data) { 
    LOG_M(DEBUG3, " node id=" << visited_node_id <<
                  " Should prune? " << (rank_data.rank >= T_) <<
                  " current rank =" << rank_data.rank <<
                  " Distance =" << rank_data.distance <<
                  " T = " << T_ <<
                  " Deg= " << graph_->GetNI(visited_node_id).GetOutDeg());

    if (rank_data.rank > T_) {
      return true;
    }

    if (delta_.count(visited_node_id) == 0 || delta_[visited_node_id] > rank_data.rank ) {
      delta_[visited_node_id] = rank_data.rank;
    }

    if (delta_[visited_node_id] < rank_data.rank) {
      return true;
    }

    return false;
  }

  inline bool ShouldStop() {
    return false;
  }

  inline void RelaxedPath(int node_id) { }

  inline std::vector<int>& get_visited_nodes() {
    return visited_nodes_including_self_;
  }

  std::unordered_map<int, bool> wanted_nodes_;
  std::unordered_map<int, double> delta_;
  int source_node_id_;
  int T_;
  std::vector<int> visited_nodes_including_self_;
  int num_nodes_visited_;
  graph::Graph<Z>* graph_;
};

/*! \endcond
*/

/*! \brief Infuelnce maximization based on forward ranks
*/
template <class Z>
class TSkimForwardRank : public TSkimBase<Z> {
public:
  /*! \brief Initialize the class.
      \param[in] T -  The influence of each node, e.g. if T=100 then each nodes 
                      has influence over all nodes that rank him among their top 100
      \param[in] K_all_distance_sketch - The first step of the algorithm is to
                                          calculate the graph sketch. This parameter detetmines 
                                          the accuracy of the influence.
      \param[out] cover - The result cover
      \param[in] graph - The graph to conduct the influence maximization on
      \see GraphSketch
  */
  void InitTSkim(int T,
            int K_all_distance_sketch,
            int min_influence_for_seed,
            Cover * cover,
            graph::Graph<Z>* graph) {
    K_all_distance_sketch_ = K_all_distance_sketch;
    graph_sketch_ = NULL;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }
  /*! \brief Sets the graph sketch and prevents the algorithm from performing the calculation
      Calculating the graph sketch may be expensive specially if you want to play with different value of
      T.
  */
  void set_graph_sketch(GraphSketch* graph_sketch) {
    graph_sketch_ = graph_sketch;
  }

  /*! \cond
  */
  int AddSeed(int seed, std::unordered_map<int, int>* influence_change) {
    LOG_M(DEBUG3, "seed node = " << seed);
    std::vector<int> ranking;
    reverse_rank_call_backs_.PrepareForIteration();
    CalculateReverseRank<Z, TSkimReverseRankCallBacks<Z> > (seed,
                                                            &graph_transpose_,
                                                            graph_sketch_,
                                                            &ranking,
                                                            &reverse_rank_call_backs_);
    LOG_M(DEBUG3, "Adding seed node= " << seed << " Cover size=" << reverse_rank_call_backs_.get_visited_nodes().size() <<
                  " Cover size=" << this->cover_->Size());
    return TSkimBase<Z>::UpdateCover(seed, influence_change, reverse_rank_call_backs_.get_visited_nodes());
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
  /*! \endcond
  */
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
};

} // namespace all_distance_sketch

#endif  // ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
