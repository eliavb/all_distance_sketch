#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_

#include <limits.h>

#include "sketch_calculation.h"
#include "../sketch/graph_sketch.h"
#include "../graph/graph.h"
#include "reverse_rank.h"

namespace all_distance_sketch {


class Cover {
  public:
    typedef std::unordered_map< int, std::vector<int> >::iterator Iterator;

    Cover() {
      Clear();
    }
    inline void Clear() {
      cover.clear();
      is_covered.clear();
    }
    inline void AddSeed(int seed) {
      is_covered[seed] = true;
      cover[seed] = std::vector<int>();
      cover[seed].push_back(seed);
    }
    inline void AddNodeToSeed(int seed, int node_id) {
      is_covered[node_id] = true;
      if (cover.count(seed) == 0) {
        cover[seed] = std::vector<int>();
      }
      cover[seed].push_back(node_id);
    }
    inline bool IsCovered(int node_id) {
      return (is_covered.count(node_id) != 0);
    }
    inline unsigned int Size() {
      return cover.size();
    }
    inline const std::vector<int>& GetSeedCover(int seed) {
      if (cover.count(seed) == 0) {
        return empty_cover;
      }
      return cover[seed];
    }
    inline Iterator Begin() {
      return cover.begin();
    }
    inline Iterator End() {
      return cover.end();
    }
  private:
    std::vector<int> empty_cover;
    std::unordered_map< int, std::vector<int> > cover;
    std::unordered_map< int, bool > is_covered;
};


template<class Z>
class TSkimReverseRankCallBacks {
 public:
  void SetWantedNodes(const std::vector<int>& nodes) {
    for (auto node : nodes) {
      wanted_nodes_[node] = true;
    }
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


template<class Z>
class TSkimDijkstraCallBacks {
public:
  void InitTSkimDijkstraCallBacks(int T) {
    T_ = T;
    num_nodes_visited_ = 0;
    visited_nodes_including_self_.clear();
  }

  void SetWantedNodes(const std::vector<int>& nodes) {
    for (auto node : nodes) {
      wanted_nodes_[node] = true;
    }
  }

  inline void Started(int source_node_id, graph::Graph<Z>* graph) {
    LOG_M(DEBUG4, "Source node id=" << source_node_id);
    source_node_id_ = source_node_id;
  }

  inline void NodePopedFromHeap(int poped_node_id, const NodeIdDistanceData& heap_value) {
    LOG_M(DEBUG4, "poped node=" << poped_node_id);
    if (wanted_nodes_.size() != 0 && wanted_nodes_.count(poped_node_id) == 0) {
      return;
    }
    if (num_nodes_visited_ < T_) {
      visited_nodes_including_self_.push_back(poped_node_id);
    }
    ++num_nodes_visited_;
    return;
  }

  inline bool ShouldPrune(int visited_node_id, graph::EdgeWeight distance_from_source_to_visited_node) { return false; }

  inline bool ShouldStop() {
    return num_nodes_visited_ >= T_;
  }

  const std::vector<int>& get_visited_nodes() {
    return visited_nodes_including_self_;
  }
  inline void RelaxedPath(int node_id) { }
  std::unordered_map<int, bool> wanted_nodes_;
  int source_node_id_;
  int T_;
  std::vector<int> visited_nodes_including_self_;
  int num_nodes_visited_;
};


template<class Z>
class TSkimDijkstraCallBacksDistancePrune {
public:
  void InitTSkimDijkstraCallBacksDistancePrune(double distance_stop) {
    num_nodes_visited_ = 0;
    visited_nodes_including_self_.clear();
    distance_stop_ = distance_stop;
  }

  void SetWantedNodes(const std::vector<int>& nodes) {
    for (auto node : nodes) {
      wanted_nodes_[node] = true;
    }
  }

  inline void Started(int source_node_id, graph::Graph<Z>* graph) {
    LOG_M(DEBUG4, "Source node id=" << source_node_id);
    source_node_id_ = source_node_id;
  }

  inline void NodePopedFromHeap(int poped_node_id, const NodeIdDistanceData& heap_value) {
    LOG_M(DEBUG4, "poped node=" << poped_node_id);
    if (wanted_nodes_.size() != 0 && wanted_nodes_.count(poped_node_id) == 0) {
      return;
    }
    LOG_M(DEBUG3, " poped node=" << poped_node_id << 
                  " Distance=" << heap_value.GetDistance() << 
                  " Distance to stop=" << distance_stop_);
    if (heap_value.GetDistance() < distance_stop_) {
      visited_nodes_including_self_.push_back(poped_node_id);
      ++num_nodes_visited_;
    }
    return;
  }

  inline bool ShouldPrune(int visited_node_id, graph::EdgeWeight distance_from_source_to_visited_node) {
    return distance_from_source_to_visited_node >= distance_stop_;
  }

  inline bool ShouldStop() { return false; }

  const std::vector<int>& get_visited_nodes() {
    return visited_nodes_including_self_;
  }
  inline void RelaxedPath(int node_id) { }
  std::unordered_map<int, bool> wanted_nodes_;
  int source_node_id_;
  std::vector<int> visited_nodes_including_self_;
  int num_nodes_visited_;
  double distance_stop_;
};

int random_gen_func (int i) { return 42 % i;}

bool cmp(const std::pair<int, int>  &p1, const std::pair<int, int> &p2) {
    return p1.second > p2.second;
}

struct cmp_pair {
  bool operator()(const std::pair<int, int>  &p1, const std::pair<int, int> &p2) const {
    if (p1.second > p2.second) {
      return true;
    }
    if (p1.second < p2.second) {
      return false;
    }
    if (p1.first > p2.first) {
      return true;
    }
    if (p1.first < p2.first) {
      return false;
    }
    return false;
  }
};

template <class Z>
class TSkimBase {
public:
  virtual ~TSkimBase() {};

  void InitTSkimBase(int T,
                    int min_influence_for_seed,
                    Cover * cover,
                    graph::Graph<Z>* graph) {
    T_ = T;
    min_influence_for_seed_ = min_influence_for_seed;
    cover_ = cover;
    graph_ = graph;
    node_influence_.clear();
    reachable_nodes_.clear();
  }

  void set_rankers_nodes(const std::vector<int>& rankers_nodes) {
    rankers_nodes_ = rankers_nodes;
  }

  void set_rankees_nodes(const std::vector<int>& rankees_nodes) {
    rankees_nodes_ = rankees_nodes;
  }

  void set_wanted_cover_nodes(const std::vector<int>& wanted_cover_nodes) {
    wanted_nodes_ = wanted_cover_nodes;
  }

  int UpdateCover(int seed, std::unordered_map<int, int>* influence_change, const std::vector<int>& covered_nodes) {
    int num_covered_nodes = 0;
    for (const auto& covered_node : covered_nodes) {
      if (cover_->IsCovered(covered_node)) {
        // Covered by a different node
        continue;
      }
      LOG_M(DEBUG3, "Covered node= " << covered_node);
      ++num_covered_nodes;
      cover_->AddNodeToSeed(seed, covered_node);
      node_influence_.erase(covered_node);
      if (reachable_nodes_.count(covered_node) == 0) {
        continue;
      }
      for (const auto& reachable_node_from_covered_node : reachable_nodes_[covered_node]) {
        if (cover_->IsCovered(reachable_node_from_covered_node)) {
          // Covered by a different node
          continue;
        }
        --node_influence_[reachable_node_from_covered_node];
      }
      reachable_nodes_.erase(covered_node);
    }
    LOG_M(DEBUG3, "Seed=" << seed << " num actual covered nodes= " << num_covered_nodes);
    return num_covered_nodes;
  }

  virtual int AddSeed(int seed, std::unordered_map<int, int>* influence_change) = 0;

  /*
  * Invariant: Once a node is covered he will not have any influnce.
  * We will erase his influence and not update it any more.
  */
  virtual void UpdateInfluceAndSeedSet(int source_node,
                               int visited_node,
                               std::vector<int>* seed_set) {
    if (node_influence_.count(visited_node) == 0) {
      node_influence_[visited_node] = 0;
    }
    ++(node_influence_)[visited_node];
    if (node_influence_[visited_node] >= min_influence_for_seed_) {
      LOG_M(DEBUG3, "adding seed " << visited_node << " influence=" << node_influence_[visited_node]);
      seed_set->push_back(visited_node);
    }
    if (reachable_nodes_.count(source_node) == 0) {
      reachable_nodes_[source_node] = std::vector<int>();
    }
    reachable_nodes_[source_node].push_back(visited_node);
  }

  void ExtractNodesFromGraph(std::vector<int>* candidates) {
    if (candidates->size() == 0) {
      for (auto node_itr = graph_->BegNI();  /* node_itr.HasMore()*/ node_itr != graph_->EndNI() ; node_itr++ ){
        candidates->push_back(node_itr.GetId());
      }
    }
  }

  void CalculateRankeeNodes() {
    ExtractNodesFromGraph(&rankees_nodes_);
  }

  void CalculateRankerNodes() {
    ExtractNodesFromGraph(&rankers_nodes_);
  }

  void CalculateWantedNodes() {
    ExtractNodesFromGraph(&wanted_nodes_);
  }

  void CalculateRandomNodePermutation(std::vector<int>* nodes) {
    std::random_shuffle (nodes->begin(), nodes->end(), random_gen_func);
  }

  void PreRunInit() {
    CalculateRankerNodes();
    CalculateRankeeNodes();
    CalculateWantedNodes();
    CalculateRandomNodePermutation(&rankers_nodes_);
  }

  virtual const std::vector<int>& CalculateVisitedNodes(int source) = 0;

  int Run(bool ShouldRunPreRunInit = true) {
    if (ShouldRunPreRunInit) {
      PreRunInit();
    }
    int num_covered_nodes = 0;
    // Itrating the permutation of nodes
    int num_passed = 0;
    for (const auto& source_node_id : rankers_nodes_) {
      std::cout << "\r" << num_passed  << "/" << rankers_nodes_.size();
      ++num_passed;
      if (cover_->IsCovered(source_node_id)) {
        continue;
      }
      LOG_M(DEBUG3, "Iterating permutation, current node = " << source_node_id);
      std::vector<int> current_iteration_seed_set;
      const std::vector<int>& visited_nodes = CalculateVisitedNodes(source_node_id);
      for (const auto& visited_node : visited_nodes) {
        LOG_M(DEBUG3, "visited node = " << visited_node);
        if (cover_->IsCovered(visited_node)) {
          continue;
        }
        UpdateInfluceAndSeedSet(source_node_id, visited_node, &current_iteration_seed_set);
      }

      for (const auto& seed : current_iteration_seed_set) {
        if (cover_->IsCovered(source_node_id)) {
          continue;
        }
        std::unordered_map<int, int> influence_change;
        LOG_M(DEBUG3, "Adding Seed node =" << seed );
        num_covered_nodes += AddSeed(seed, &influence_change);
      }
    }

    std::cout << std::endl;

    LOG_M(DEBUG3, " num covered nodes =" << num_covered_nodes <<
                  " num nodes left =" << node_influence_.size());
    std::set< std::pair<int, int> , cmp_pair> heap;
    for (auto it = node_influence_.begin(); it != node_influence_.end(); it++) {
      LOG_M(DEBUG3, "Node influence raw node =" << it->first << " Influence=" << it->second);
      heap.insert(std::make_pair(it->first, it->second));
    }

    while (!heap.empty()) {
      auto node_details = (*heap.begin());
      heap.erase(heap.begin());
      int seed = node_details.first;
      if (cover_->IsCovered(seed)) {
        assert(node_influence_.count(seed) == 0);
        continue;
      }
      LOG_M(DEBUG3, "Adding Seed node =" << seed << " Influence=" << node_details.second);
      std::unordered_map<int, int> influence_change;
      num_covered_nodes += AddSeed(seed, &influence_change);
      for (auto change_it : influence_change) {
        heap.erase(std::make_pair(change_it.first, change_it.second));
        heap.insert( std::make_pair(change_it.first, change_it.second));
      }
    }
    return 0;
  }

protected:
  int T_;
  int min_influence_for_seed_;
  Cover * cover_;
  graph::Graph<Z>* graph_;
  std::vector<int> rankers_nodes_;
  std::vector<int> rankees_nodes_;
  std::vector<int> wanted_nodes_;

  std::unordered_map<int, int> node_influence_;
  std::unordered_map<int, std::vector<int> > reachable_nodes_;
};

template <class Z>
class TSkimForwardRank : public TSkimBase<Z> {
public:
  void InitTSkim(int T,
            int K_all_distance_sketch,
            int min_influence_for_seed,
            Cover * cover,
            graph::Graph<Z>* graph) {
    K_all_distance_sketch_ = K_all_distance_sketch;
    graph_sketch_ = NULL;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }

  void set_graph_sketch(GraphSketch* graph_sketch) {
    graph_sketch_ = graph_sketch;
  }

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

  int Run() {
    TSkimBase<Z>::PreRunInit();
    GraphSketch local_graph_sketch;
    if (graph_sketch_ == NULL) {
      graph_sketch_ = &local_graph_sketch;
      graph_sketch_->InitGraphSketch(K_all_distance_sketch_, this->graph_->GetMxNId());
    }
    CalculateGraphSketch<Z>(this->graph_, graph_sketch_);
    graph_sketch_->CalculateAllDistanceNeighborhood();
    this->graph_->Transpose(&graph_transpose_);
    reverse_rank_call_backs_.InitTSkimReverseRankCallBacks(this->T_);
    reverse_rank_call_backs_.SetWantedNodes(this->wanted_nodes_);
    call_backs_.SetWantedNodes(this->rankees_nodes_);
    return  TSkimBase<Z>::Run(false);
  }

private:
  int K_all_distance_sketch_;
  graph::Graph<Z> graph_transpose_;
  GraphSketch* graph_sketch_;
  TSkimReverseRankCallBacks<Z> reverse_rank_call_backs_;
  DijkstraParams param_;
  TSkimDijkstraCallBacksDistancePrune<Z> call_backs_;
};


template <class Z>
class TSkim : public TSkimBase<Z> {
public:
  void InitTSkim(int T,
            int K_all_distance_sketch,
            int min_influence_for_seed,
            Cover * cover,
            graph::Graph<Z>* graph) {
    K_all_distance_sketch_ = K_all_distance_sketch;
    graph_sketch_ = NULL;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }

  void set_graph_sketch(GraphSketch* graph_sketch) {
    graph_sketch_ = graph_sketch;
  }

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
      typename Z::TNode source(source_node_id);
      call_backs_.InitTSkimDijkstraCallBacks(TSkimBase<Z>::T_);
      PrunedDijkstra< Z, TSkimDijkstraCallBacks<Z> > (source,
                                                      TSkimBase<Z>::graph_,
                                                      &call_backs_,
                                                      &param_);
      return call_backs_.get_visited_nodes();
  }

  int Run() {
    TSkimBase<Z>::PreRunInit();
    GraphSketch local_graph_sketch;
    if (graph_sketch_ == NULL) {
      graph_sketch_ = &local_graph_sketch;
      graph_sketch_->InitGraphSketch(K_all_distance_sketch_, this->graph_->GetMxNId());
    }
    CalculateGraphSketch<Z>(this->graph_, graph_sketch_);
    graph_sketch_->CalculateAllDistanceNeighborhood();
    this->graph_->Transpose(&graph_transpose_);
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
  TSkimDijkstraCallBacks<Z> call_backs_;
};


template <class Z>
class TSkimGreedy : public TSkimBase<Z> {
public:
  void InitTSkimGreedy(int T,
                      Cover * cover,
                      graph::Graph<Z>* graph) {
    int min_influence_for_seed = INT_MAX;
    TSkimBase<Z>::InitTSkimBase(T, min_influence_for_seed, cover, graph);
  }

  void UpdateInfluceAndSeedSet(int source_node,
                                 int visited_node,
                                 std::vector<int>* seed_set) {
    if (coveres_.count(visited_node) == 0) {
      coveres_[visited_node] = std::vector<int>();
    }
    coveres_[visited_node].push_back(source_node);
    TSkimBase<Z>::UpdateInfluceAndSeedSet(source_node, visited_node, seed_set);
    assert(coveres_[visited_node].size() == this->node_influence_[visited_node]);
  }

  int AddSeed(int seed, std::unordered_map<int, int>* influence_change) {
    LOG_M(DEBUG3, "seed node = " << seed);
    std::vector<int> convered_nodes;
    for (const auto& node: this->coveres_[seed]) {
      for (const auto& wanted_node : this->wanted_nodes_) {
        if (wanted_node == node) {
          convered_nodes.push_back(node);
          break;
        }
      }
    }
    LOG_M(DEBUG3, " Converted nodes size=" << convered_nodes.size() <<
                  " coveres_ size=" << this->coveres_[seed].size() <<
                  " Influence=" << this->node_influence_[seed]);
    return TSkimBase<Z>::UpdateCover(seed, influence_change, convered_nodes);
  }

  const std::vector<int>& CalculateVisitedNodes(int source_node_id) {
      typename Z::TNode source(source_node_id);
      call_backs_.InitTSkimDijkstraCallBacks(TSkimBase<Z>::T_);
      PrunedDijkstra< Z, TSkimDijkstraCallBacks<Z> > (source,
                                                      TSkimBase<Z>::graph_,
                                                      &call_backs_,
                                                      &param_);
      return call_backs_.get_visited_nodes();
  }

  int Run() {
    call_backs_.SetWantedNodes(this->rankees_nodes_);
    return TSkimBase<Z>::Run();
  }

private:
  std::unordered_map<int, std::vector<int> > coveres_;
  DijkstraParams param_;
  TSkimDijkstraCallBacks<Z> call_backs_;
};



}  //  all_distance_sketch
#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
