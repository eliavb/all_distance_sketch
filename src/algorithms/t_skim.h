#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_


#include "sketch_calculation.h"
#include "../sketch/graph_sketch.h"
#include "../graph/graph.h"
#include "./reverse_rank.h"

namespace all_distance_sketch {

typedef struct SeedCover_t {
  int seed;
  int index;
  std::vector<int> covered_nodes;
} SeedCover;

class Cover {
  public:
    typedef std::unordered_map< int, SeedCover >::iterator Iterator;

    Cover() {
      Clear();
    }

    inline void Clear() {
      cover.clear();
      is_covered.clear();
    }
    inline void AddSeed(int seed) {
      is_covered[seed] = true;
      cover[seed] = SeedCover();
      cover[seed].index = cover.size();
      cover[seed].covered_nodes.push_back(seed);
    }
    inline void SetSeedEstimate(int seed, double estimate_cover_size) {
      estimated_cover[seed] = estimate_cover_size;
    }
    inline double GetSeedEstimate(int seed) {
      return estimated_cover[seed];
    }
    inline void AddNodeToSeed(int seed, int node_id) {
      is_covered[node_id] = true;
      if (cover.count(seed) == 0) {
        cover[seed] = SeedCover();
        cover[seed].index = cover.size();
      }
      cover[seed].covered_nodes.push_back(node_id);
    }
    inline bool IsCovered(int node_id) {
      return (is_covered.count(node_id) != 0);
    }
    inline unsigned int Size() {
      return cover.size();
    }
    inline const SeedCover& GetSeedCover(int seed) {
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
    SeedCover empty_cover;
    std::unordered_map< int, SeedCover > cover;
    std::unordered_map< int, bool > is_covered;
    std::unordered_map< int, double> estimated_cover;
};


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


template<class Z>
class TSkimDijkstraCallBacks {
public:
  void InitTSkimDijkstraCallBacks(int T) {
    T_ = T;
    num_nodes_visited_ = 0;
    visited_nodes_including_self_.clear();
  }

  void SetWantedNodes(const std::unordered_map<int, bool>& nodes) {
    wanted_nodes_ = nodes;
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

  void SetWantedNodes(const std::unordered_map<int, bool>& nodes) {
    wanted_nodes_ = nodes;
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
    for (int i=0; i < rankees_nodes.size(); i++) {
      rankees_nodes_[rankees_nodes[i]] = true;
    }
  }

  void set_wanted_cover_nodes(const std::vector<int>& wanted_cover_nodes) {
    for (int i=0; i < wanted_cover_nodes.size(); i++) {
      wanted_nodes_[wanted_cover_nodes[i]] = true;
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
    for (int i=0; i < nodes->size(); i++) {
      boost::random::mt19937 gen;
      boost::random::uniform_int_distribution<> dis(i, nodes->size() - 1);
      int swap_index = dis(gen);
      int a = (*nodes)[i];
      int b = (*nodes)[swap_index];
      (*nodes)[i] = b;
      (*nodes)[swap_index] = a;
    }
  }

  void ExtractNodesFromGraph(std::unordered_map<int, bool>* candidates) {
    if (candidates->size() == 0) {
      for (auto node_itr = graph_->BegNI();  /*node_itr.HasMore()*/ node_itr != graph_->EndNI() ; node_itr++ ){
        (*candidates)[node_itr.GetId()] = true;
      }
    }
  }

  void ExtractNodesFromGraph(std::vector<int>* candidates) {
    if (candidates->size() == 0) {
      for (auto node_itr = graph_->BegNI();  /*node_itr.HasMore()*/  node_itr != graph_->EndNI() ; node_itr++ ){
        candidates->push_back(node_itr.GetId());
      }
    }
  }

  void PreRunInit() {
    CalculateRankerNodes();
    CalculateRankeeNodes();
    CalculateWantedNodes();
    CalculateRandomNodePermutation(&rankers_nodes_);
  }

  int UpdateCover(int seed, std::unordered_map<int, int>* influence_change, const std::vector<int>& covered_nodes) {
    int num_covered_nodes = 0;
    for (int j=0; j < covered_nodes.size(); j++) {
      int covered_node = covered_nodes[j];
      if (cover_->IsCovered(covered_node)) {
        // Covered by a different node
        continue;
      }
      LOG_M(DEBUG3, "Seed=" << seed << "Covered node= " << covered_node);
      ++num_covered_nodes;
      cover_->AddNodeToSeed(seed, covered_node);
      node_influence_.erase(covered_node);
    }

    for (int j=0; j < covered_nodes.size(); j++) {
      int covered_node = covered_nodes[j];
      if (reachable_nodes_.count(covered_node) == 0) {
        continue;
      }
      for (int i=0; i < reachable_nodes_[covered_node].size(); i++) {
        int reachable_node_from_covered_node = reachable_nodes_[covered_node][i];
        if (cover_->IsCovered(reachable_node_from_covered_node)) {
          // Covered by a different node
          continue;
        }
        LOG_M(DEBUG3, "Node " << reachable_node_from_covered_node << " reachable from " << covered_node);
        if (influence_change->count(reachable_node_from_covered_node) == 0) {
          (*influence_change)[reachable_node_from_covered_node] = node_influence_[reachable_node_from_covered_node];
        }
        node_influence_[reachable_node_from_covered_node] = node_influence_[reachable_node_from_covered_node] - 1;
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

  virtual const std::vector<int>& CalculateVisitedNodes(int source) = 0;

  int Run(bool ShouldRunPreRunInit = true) {
    if (ShouldRunPreRunInit) {
      PreRunInit();
    }
    int num_covered_nodes = 0;
    // Itrating the permutation of nodes
    int num_passed = 0;
    for (int i=0; i < rankers_nodes_.size(); i++) {
      int source_node_id = rankers_nodes_[i];
      // std::cout << "\r" << num_passed  << "/" << rankers_nodes_.size();
      ++num_passed;
      if (cover_->IsCovered(source_node_id)) {
        continue;
      }
      LOG_M(DEBUG3, "Iterating permutation, current node = " << source_node_id);
      std::vector<int> current_iteration_seed_set;
      const std::vector<int>& visited_nodes = CalculateVisitedNodes(source_node_id);
      for (int i=0; i < visited_nodes.size(); i++) {
        int visited_node = visited_nodes[i];
        LOG_M(DEBUG3, "visited node = " << visited_node);
        if (cover_->IsCovered(visited_node)) {
          continue;
        }
        UpdateInfluceAndSeedSet(source_node_id, visited_node, &current_iteration_seed_set);
      }

      for (int i=0; i <current_iteration_seed_set.size(); i++) {
        int seed = current_iteration_seed_set[i];
        if (cover_->IsCovered(source_node_id) || node_influence_[seed] < min_influence_for_seed_) {
          continue;
        }
        std::unordered_map<int, int> influence_change;
        LOG_M(DEBUG5, "Adding Seed node =" << seed );
        double estimate_seed_cover_size = ( min_influence_for_seed_ * (static_cast<double>(rankers_nodes_.size()) / static_cast<double>(num_passed)) );
        cover_->SetSeedEstimate(seed, estimate_seed_cover_size);
        num_covered_nodes += AddSeed(seed, &influence_change);        
      }
    }

    LOG_M(DEBUG3, " num covered nodes =" << num_covered_nodes <<
                  " num nodes left =" << node_influence_.size());
    std::set< std::pair<int, int> , cmp_pair> heap;
    for (auto it = node_influence_.begin(); it != node_influence_.end(); it++) {
      LOG_M(DEBUG3, "Node influence raw node =" << it->first << " Influence=" << it->second);
      heap.insert(std::make_pair(it->first, it->second));
    }
    int last_cover_size = INT_MAX;
    int last_influence = INT_MAX;
    while (!heap.empty()) {
      auto node_details = (*heap.begin());
      heap.erase(heap.begin());
      int seed = node_details.first;
      int seed_influence = node_details.second;
      if (seed_influence > last_influence) {
        LOG_M(NOTICE, "Assumption failed num influence increased" <<
                      "last influence=" << last_influence <<
                      "current influence=" << seed_influence);
        assert(seed_influence <= last_influence);
      }
      last_influence = seed_influence;
      if (cover_->IsCovered(seed)) {
        assert(node_influence_.count(seed) == 0);
        continue;
      }
      std::unordered_map<int, int> influence_change;
      auto num_covered_nodes_by_seed = AddSeed(seed, &influence_change);
      if (this->wanted_nodes_.size() == this->graph_->GetNumNodes()) {
        assert(num_covered_nodes_by_seed == seed_influence);
      }
      if (num_covered_nodes_by_seed > last_cover_size && 
          this->wanted_nodes_.size() == this->graph_->GetNumNodes())  {
        LOG_M(NOTICE, "Assumption failed num covered nodes increased" <<
                      " last=" << last_cover_size <<
                      " current="<< num_covered_nodes_by_seed << 
                      " seed=" << seed <<
                      " seed influence=" << seed_influence);
        assert(num_covered_nodes_by_seed <= last_cover_size);
      }
      last_cover_size = num_covered_nodes_by_seed;
      LOG_M(DEBUG3, "Adding Seed node =" << seed << " Influence=" << seed_influence << "covered nodes=" << num_covered_nodes_by_seed);
      num_covered_nodes += num_covered_nodes_by_seed;
      cover_->SetSeedEstimate(seed, num_covered_nodes_by_seed);
      for (std::unordered_map<int, int>::iterator change_it = influence_change.begin();
          change_it != influence_change.end();
          change_it++) {
        heap.erase(std::make_pair(change_it->first, change_it->second));
        heap.insert( std::make_pair(change_it->first, node_influence_[change_it->first]));
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
  std::unordered_map<int, bool> rankees_nodes_;
  std::unordered_map<int, bool> wanted_nodes_;

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
  int j=0;
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

}  //  all_distance_sketch
#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_T_SKIM_H_
