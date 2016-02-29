#ifndef SRC_ALGORITHMS_DIJKSTRA_SHORTEST_PATHS_H_
#define SRC_ALGORITHMS_DIJKSTRA_SHORTEST_PATHS_H_

#include "../common.h"
#include "../utils/thread_utils.h"
#include "../sketch/node_sketch.h"
#include "../sketch/graph_sketch.h"

/*! \file dijkstra_shortest_paths.h
    \brief Optimized version of Dijkstra algorithm for the sketch calculation algorithm
*/

namespace all_distance_sketch {

/*! \cond
*/

struct PrunningAlgoStatistics {
  PrunningAlgoStatistics() : num_visited_nodes(0),
                             num_pruned_nodes(0),
                             num_relaxed_edges(0) {}
  unsigned int num_visited_nodes;
  unsigned int num_pruned_nodes;
  unsigned int num_relaxed_edges;

  void Clear() {
    num_visited_nodes = 0;
    num_pruned_nodes = 0;
    num_relaxed_edges = 0;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const PrunningAlgoStatistics& algo_statistics) {
    os << " num Nodes Visited=" << algo_statistics.num_visited_nodes <<
          " num Pruned Nodes=" << algo_statistics.num_pruned_nodes <<
          " num Relaxed Edges=" << algo_statistics.num_relaxed_edges;
    return os;
  }
};

struct compareNodeDistanceAndId {
  inline bool operator()(const NodeIdDistanceData& n1,
                        const NodeIdDistanceData& n2) const {
    if (n1.GetDistance() < n2.GetDistance()) {
      return true;
    }
    if (n1.GetDistance() > n2.GetDistance()) {
      return false;
    }
    if (n1.GetNId() < n2.GetNId()) {
      return true;
    }
    if (n1.GetNId() > n2.GetNId()) {
      return false;
    }
    return false;
  }
};

struct DijkstraParams {
  std::set< NodeIdDistanceData, compareNodeDistanceAndId > heap;
  std::vector<graph::EdgeWeight> min_distance;
  TBitSet poped;
  TBitSet touched;
};

template<class T>
class DefaultDijkstraCallBacks {
 public:
  inline void Started(int source_node_id, graph::Graph<T>* graph) { return; }

  inline void NodePopedFromHeap(int poped_node_id,
                                const NodeIdDistanceData& heap_value)
                                { return; }

  inline bool ShouldPrune(int visited_node_id,
                          graph::EdgeWeight distance_from_source_to_visited_node)
                          { return false; }

  inline bool ShouldStop() { return false; }

  inline void RelaxedPath(int node_id) { }
};

template<class Z>
class CollectorNodesUpToTRank {
 public:
  void InitCollectorNodesUpToTRank(int T) {
    T_ = T;
  }
  inline void Started(int source_node_id, graph::Graph<Z>* graph) {
    nodes_found_.clear();
    algo_statistics_.Clear();
  }

  inline void NodePopedFromHeap(int poped_node_id, const NodeIdDistanceData& heap_value) {
    if (algo_statistics_.num_visited_nodes <= T_) {
      nodes_found_.push_back(heap_value);
    }
    ++algo_statistics_.num_visited_nodes;
  }

  inline bool ShouldPrune(int visited_node_id,
                          graph::EdgeWeight distance_from_source_to_visited_node) {
    return algo_statistics_.num_visited_nodes >= T_;
  }

  inline bool ShouldStop() { return algo_statistics_.num_visited_nodes >= T_; }

  inline void RelaxedPath(int node_id) { }

  const std::vector<NodeIdDistanceData>& get_nodes_found() {
    return nodes_found_;
  }
 private:
  PrunningAlgoStatistics algo_statistics_;
  std::vector<NodeIdDistanceData> nodes_found_;
  int T_;
};

template<class T>
class DijkstraRankCallBack {
 public:
  inline void Started(int source_node_id, graph::Graph<T>* graph) {
    dijkstra_rank_.clear();
    dijkstra_rank_.resize(graph->GetMxNId(), constants::UNREACHABLE);
    dijkstra_rank_[source_node_id] = 0;
    algo_statistics_.Clear();
  }

  inline void NodePopedFromHeap(int poped_node_id, const NodeIdDistanceData& heap_value) {
    dijkstra_rank_[poped_node_id] = algo_statistics_.num_visited_nodes;
    ++algo_statistics_.num_visited_nodes;
  }

  inline bool ShouldPrune(int visited_node_id, graph::EdgeWeight distance_from_source_to_visited_node) { return false; }

  inline bool ShouldStop() { return false; }

  inline void RelaxedPath(int node_id) { }

  const std::vector<int>& get_dijkstra_rank() {
    return dijkstra_rank_;
  }

 private:
  PrunningAlgoStatistics algo_statistics_;
  std::vector<int> dijkstra_rank_;
};

template<class T>
class SketchDijkstraCallBacks {
 public:
  void InitSketchDijkstraCallBacks(GraphSketch* graph_sketch) { 
    graph_sketch_ = graph_sketch;
    is_multi_threaded_ = false;
    should_calculate_dijkstra_rank_ = false;
    sketch_lock_ = NULL;
    stop_after_ = -1;
    algo_statistics_.Clear();
  }                             

  void set_multi_threaded_params(bool multi_threaded_run, thread::ModuloLock * lock) {
    is_multi_threaded_ = multi_threaded_run;
    sketch_lock_ = lock;
  }
    
  void set_should_calculate_dijkstra_rank(bool should_calc) {
    should_calculate_dijkstra_rank_ = should_calc;
  }

  inline void Started(int source_node_id, graph::Graph<T>* graph) {
    algo_statistics_.Clear();
    source_node_id_ = source_node_id;
    source_node_random_id_ = graph_sketch_->GetNodeRandomId(source_node_id_);
    if (should_calculate_dijkstra_rank_) {
      dijkstra_rank_.clear();
      dijkstra_rank_.resize(graph->GetMxNId(), constants::UNREACHABLE);
      dijkstra_rank_[source_node_id_] = 0;
    }
  }
  
  inline void NodePopedFromHeap(int poped_node_id, const NodeIdDistanceData& heap_value) {
    ++algo_statistics_.num_visited_nodes;
    if (should_calculate_dijkstra_rank_) {
      dijkstra_rank_[poped_node_id] = algo_statistics_.num_visited_nodes;
    }
  }

  inline bool ShouldPrune(int visited_node_id, graph::EdgeWeight distance_from_source_to_visited_node) {
    bool should_prune = false;
    bool added_node_to_ads = true;
    NodeSketch * visited_noted_sketch = NULL;
    if (graph_sketch_->ShouldPrune(distance_from_source_to_visited_node, visited_node_id)) {
      ++algo_statistics_.num_pruned_nodes;
      return true;
    }
    
    // Get Node NodeSketch
    NodeDistanceIdRandomIdData visitingNode(distance_from_source_to_visited_node,
                                            visited_node_id, graph_sketch_->GetNodeRandomId(visited_node_id));
    visited_noted_sketch = graph_sketch_->GetNodeSketch(visitingNode);
    // Distance from source to visiting node
    NodeDistanceIdRandomIdData sourceNodeDetails(distance_from_source_to_visited_node, source_node_id_, source_node_random_id_);
    // Update the visiting node NodeSketch with the source Node
    if (is_multi_threaded_) {
       sketch_lock_->Lock(visited_node_id);
       added_node_to_ads = visited_noted_sketch->AddToCandidates(sourceNodeDetails);
       sketch_lock_->UnLock(visited_node_id);
    } else {
       added_node_to_ads = visited_noted_sketch->Add(sourceNodeDetails);
    }
    
    if (added_node_to_ads == false && visited_node_id != source_node_id_) {
      ++algo_statistics_.num_pruned_nodes;
      return true;
    }

    return should_prune;
  }

  inline bool ShouldStop() {
    if (stop_after_ != -1 && algo_statistics_.num_visited_nodes > stop_after_) {
      return true;
    }
    return false;
  }
  
  inline void RelaxedPath(int node_id) {
    ++algo_statistics_.num_relaxed_edges;
  }

  inline int get_num_pruned_nodes() {
    return algo_statistics_.num_pruned_nodes;
  }
private:
  GraphSketch * graph_sketch_;
  bool is_multi_threaded_;
  bool should_calculate_dijkstra_rank_;
  thread::ModuloLock * sketch_lock_;
  int stop_after_;
  PrunningAlgoStatistics algo_statistics_;

  int source_node_id_;
  RandomId source_node_random_id_;
  std::vector<int> dijkstra_rank_;
};


template <class T, class CallBacks>
static void PrunedDijkstra(typename T::TNode source,
                           graph::Graph<T> *graph,
                           CallBacks* call_backs,
                           DijkstraParams * param) {
  int max_node_id = graph->GetMxNId();
  int source_node_id = source.GetId();
  param->poped.clear();
  param->poped.resize(max_node_id, false);
  param->touched.clear();
  param->touched.resize(max_node_id, false);
  call_backs->Started(source_node_id, graph);

  param->min_distance.resize(max_node_id, constants::UNREACHABLE);
  param->min_distance[source.GetId()] = 0;
  param->heap.clear();
  param->heap.insert(NodeIdDistanceData(source_node_id, 0));
  param->touched[source_node_id] = true;
  LOG_M(DEBUG4, "Starting Dijkstra from node=" << source_node_id << " Max node Id " << max_node_id);
  double last_distance = 0;
  while (!param->heap.empty()) {
    NodeIdDistanceData top_node = *(param->heap.begin());
    graph::EdgeWeight distance_from_source_to_visited_node = top_node.GetDistance();
    int visited_node_id = top_node.GetNId();
    call_backs->NodePopedFromHeap(visited_node_id, top_node);
    if (call_backs->ShouldStop()) {
      LOG_M(DEBUG4, " Stopped on node=" << visited_node_id);
      return;
    }
    assert(last_distance <= distance_from_source_to_visited_node);
    last_distance = distance_from_source_to_visited_node;
    _unused(last_distance);
    param->min_distance[visited_node_id] = distance_from_source_to_visited_node;
    param->heap.erase(param->heap.begin());
    param->poped[visited_node_id] = true;
    // Insert the source node to the top NodeSketch

    if (call_backs->ShouldPrune(visited_node_id, distance_from_source_to_visited_node)) {
      LOG_M(DEBUG4, "Prunned node=" << visited_node_id);
      continue;
    }
    // Visit each edge exiting visited_node_id
    typename T::TNodeI neighbors = graph->GetNI(visited_node_id);
    graph::TEdgesWeights * nodeWeights = graph->GetNodeWeights(visited_node_id);
    for (int i = 0 ; i < neighbors.GetOutDeg(); i++) {
      int id_of_neighbor_of_visited_node = neighbors.GetOutNId(i);
      LOG_M(DEBUG4, " Node u= " << visited_node_id <<
                    " neighbor= " << id_of_neighbor_of_visited_node <<
                    " Out deg= " << neighbors.GetOutDeg() <<
                    " Index " << i <<
                    " Is Node? " << graph->IsNode(visited_node_id));

      if (param->poped[id_of_neighbor_of_visited_node] == true) {
	       continue;
      }

      // std::pair<bool, graph::EdgeWeight> edge_u_v = graph->GetEdgeWeight(visited_node_id, id_of_neighbor_of_visited_node);
      graph::EdgeWeight distance_through_u = distance_from_source_to_visited_node + (*nodeWeights)[i].get_edge_weight(); // + edge_u_v.second; 
      if ((param->touched[id_of_neighbor_of_visited_node] == false) ||
	        (distance_through_u < param->min_distance[id_of_neighbor_of_visited_node])) {
        
        if (param->touched[id_of_neighbor_of_visited_node] == false) {
          LOG_M(DEBUG1, "Node " << id_of_neighbor_of_visited_node << " Untouced ");
        } else {
          LOG_M(DEBUG1, "Node " << visited_node_id <<
                        " neighbor=" << id_of_neighbor_of_visited_node <<
                        " Distance before relax=" << param->min_distance[id_of_neighbor_of_visited_node] <<
                        " distance through visited_node_id=" << distance_through_u);
	      }
        
        call_backs->RelaxedPath(id_of_neighbor_of_visited_node);
	      // UNREACHABLE only if this is our first time
    	  if ( param->touched[id_of_neighbor_of_visited_node] == false ) { 
          param->heap.insert(NodeIdDistanceData(id_of_neighbor_of_visited_node, distance_through_u));
          LOG_M(DEBUG1, "Inserting node " << id_of_neighbor_of_visited_node << " to heap with distance " << distance_through_u);
        } else {
          param->heap.erase(NodeIdDistanceData(id_of_neighbor_of_visited_node, param->min_distance[id_of_neighbor_of_visited_node]));
          param->heap.insert(NodeIdDistanceData(id_of_neighbor_of_visited_node, distance_through_u));
          LOG_M(DEBUG5, "Decreasing distance of node " << id_of_neighbor_of_visited_node <<
                        " from " << param->min_distance[id_of_neighbor_of_visited_node] <<
    		                " to " << distance_through_u);
        }
    	 param->touched[id_of_neighbor_of_visited_node] = true;
    	 param->min_distance[id_of_neighbor_of_visited_node] = distance_through_u;
      }
    }
  }
}

/*! \endcond
*/

}  //  namespace all_distance_sketch
#endif  // SRC_ALGORITHMS_DIJKSTRA_SHORTEST_PATHS_H_
