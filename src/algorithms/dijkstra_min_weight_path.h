#ifndef SRC_ALGORITHMS_DIJKSTRA_MIN_WEIGHT_PATH_H_
#define SRC_ALGORITHMS_DIJKSTRA_MIN_WEIGHT_PATH_H_

#include "dijkstra_shortest_paths.h"

/*! \file dijkstra_shortest_paths.h
  \brief Optimized version of Dijkstra algorithm for the sketch calculation algorithm
  */

namespace all_distance_sketch {

/*! \cond
*/

struct compareNodeDistanceAndIdMax {
    inline bool operator()(const NodeIdDistanceData& n1,
                           const NodeIdDistanceData& n2) const {
      if (n1.GetDistance() > n2.GetDistance()) {
        return true;
      }
      if (n1.GetDistance() < n2.GetDistance()) {
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

struct DijkstraParamsMaxWeight {
  std::set< NodeIdDistanceData, compareNodeDistanceAndIdMax > heap;
  std::vector<graph::EdgeWeight> max_distance;
  TBitSet poped;
  TBitSet touched;
};

template <class T, class CallBacks>
static void PrunedDijkstraMinWeight(typename T::TNode source,
                           graph::Graph<T> *graph,
                           CallBacks* call_backs,
                           DijkstraParamsMaxWeight * param) {
  int max_node_id = graph->GetMxNId();
  int source_node_id = source.GetId();
  param->poped.clear();
  param->poped.resize(max_node_id, false);
  param->touched.clear();
  param->touched.resize(max_node_id, false);
  call_backs->Started(source_node_id, graph);

  param->max_distance.resize(max_node_id, 0);
  param->max_distance[source.GetId()] = constants::INF;
  param->heap.clear();
  param->heap.insert(NodeIdDistanceData(source_node_id, constants::INF));
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
    assert(last_distance >= distance_from_source_to_visited_node);
    last_distance = distance_from_source_to_visited_node;
    _unused(last_distance);
    param->max_distance[visited_node_id] = distance_from_source_to_visited_node;
    param->heap.erase(param->heap.begin());
    param->poped[visited_node_id] = true;
    // Insert the source node to the top NodeSketch
    LOG_M(DEBUG3, " poped node=" << visited_node_id << " distance=" << distance_from_source_to_visited_node);
    if (call_backs->ShouldPrune(visited_node_id, distance_from_source_to_visited_node)) {
      LOG_M(DEBUG4, "Prunned node=" << visited_node_id);
      continue;
    }
    // Visit each edge exiting visited_node_id
    typename T::TNodeI neighbors = graph->GetNI(visited_node_id);
    graph::TEdgesWeights * nodeWeights = graph->GetNodeWeights(visited_node_id);
    for (int i = 0 ; i < neighbors.GetOutDeg(); i++) {
      int id_of_neighbor_of_visited_node = neighbors.GetOutNId(i);
      LOG_M(DEBUG3, " Node u= " << visited_node_id <<
                    " neighbor= " << id_of_neighbor_of_visited_node <<
                    " Out deg= " << neighbors.GetOutDeg() <<
                    " Index " << i <<
                    " Is Node? " << graph->IsNode(visited_node_id));

      if (param->poped[id_of_neighbor_of_visited_node] == true) {
        continue;
      }

      graph::EdgeWeight distance_through_u = std::min(distance_from_source_to_visited_node, (*nodeWeights)[i].get_edge_weight());
      LOG_M(DEBUG3, "distance through=" << distance_through_u << " param->max_distance[id_of_neighbor_of_visited_node]=" << param->max_distance[id_of_neighbor_of_visited_node]); 
      if ((param->touched[id_of_neighbor_of_visited_node] == false) ||
          (distance_through_u > param->max_distance[id_of_neighbor_of_visited_node])) {

        if (param->touched[id_of_neighbor_of_visited_node] == false) {
          LOG_M(DEBUG1, "Node " << id_of_neighbor_of_visited_node << " Untouced ");
        } else {
          LOG_M(DEBUG3, "Node " << visited_node_id <<
                        " neighbor=" << id_of_neighbor_of_visited_node <<
                        " Distance before relax=" << param->max_distance[id_of_neighbor_of_visited_node] <<
                        " distance through visited_node_id=" << distance_through_u);
        }

        call_backs->RelaxedPath(id_of_neighbor_of_visited_node);
        // 0 only if this is our first time
        if ( param->touched[id_of_neighbor_of_visited_node] == false ) {
          param->heap.insert(NodeIdDistanceData(id_of_neighbor_of_visited_node, distance_through_u));
          LOG_M(DEBUG1, "Inserting node " << id_of_neighbor_of_visited_node << " to heap with distance " << distance_through_u);
        } else {
          param->heap.erase(NodeIdDistanceData(id_of_neighbor_of_visited_node, param->max_distance[id_of_neighbor_of_visited_node]));
          param->heap.insert(NodeIdDistanceData(id_of_neighbor_of_visited_node, distance_through_u));
          LOG_M(DEBUG5, "Decreasing distance of node " << id_of_neighbor_of_visited_node <<
                        " from " << param->max_distance[id_of_neighbor_of_visited_node] <<
                        " to " << distance_through_u);
        }
        param->touched[id_of_neighbor_of_visited_node] = true;
        param->max_distance[id_of_neighbor_of_visited_node] = distance_through_u;
      }
    }
  }
}

/*! \endcond
*/

}  //  namespace all_distance_sketch
#endif  // SRC_ALGORITHMS_DIJKSTRA_MIN_WEIGHT_PATH_H_
