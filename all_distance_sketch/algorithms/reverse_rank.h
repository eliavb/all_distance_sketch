#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_

#include "../common.h"
#include "../sketch/graph_sketch.h"

namespace all_distance_sketch {

template <class T, class CallBacks>
static void CalculateReverseRank(typename T::TNode source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking,
                       CallBacks* call_backs);

template <class T>
static int EstimateReverseRankUpperBound(graph::Graph<T> * graph,
                    GraphSketch * graph_sketch,
                    int source,
                    int target,
                    graph::EdgeWeight distance_from_source_to_targer);


struct RankData {
    RankData(int _node_id, int _rank, graph::EdgeWeight _distance) :
      node_id(_node_id), rank(_rank), distance(_distance) {}
    RankData() : node_id(-1), rank(-1), distance(constants::UNREACHABLE) { }
    int node_id;
    int rank;
    graph::EdgeWeight distance;
};

struct compareRankNode {
    bool operator()(const RankData& n1, const RankData& n2) const {
        // return n1.rank > n2.rank;
        if (n1.rank > n2.rank) {
            return true;
        }
        if (n1.rank < n2.rank) {
            return false;
        }
        if (n1.distance > n2.distance) {
            return true;
        }
        if (n1.distance < n2.distance) {
            return false;
        }
        if (n1.node_id > n2.node_id) {
            return true;
        }
        return false;
    }
};

template <class T>
static int EstimateReverseRankUpperBound(graph::Graph<T> * graph,
                    GraphSketch * graph_sketch,
                    int source,
                    int target,
                    graph::EdgeWeight distance_from_source_to_targer) {
    if (source == target) {
        LOG_M(DEBUG4, " Source node is equal to target");
        return 0;
    }
    NodeIdRandomIdData source_node_details(source, graph_sketch->GetNodeRandomId(source));
    NodeSketch * sourceNodeAds = graph_sketch->GetNodeSketch(source_node_details);
    int reverse_rank = sourceNodeAds->GetSizeNeighborhoodUpToDistance(distance_from_source_to_targer,
                                                                      graph_sketch->GetNodesDistributionLean());
    LOG_M(DEBUG3, " Source=" << source <<
                " node=" << target <<
                " distance= " << distance_from_source_to_targer <<
                " reverse_rank=" << reverse_rank);
    return reverse_rank;
}


template <class T>
static double EstimateReverseRankAverage(graph::Graph<T> * graph,
                                         GraphSketch * graph_sketch,
                                         int source,
                                         int target,
                                         graph::EdgeWeight distance_from_source_to_target) {
    if (source == target) {
        LOG_M(DEBUG4, " Source node is equal to target");
        return 0;
    }
    NodeIdRandomIdData source_node_details(source, graph_sketch->GetNodeRandomId(source));
    NodeSketch * sourceNodeAds = graph_sketch->GetNodeSketch(source_node_details);
    int distance_index = sourceNodeAds->GetNeighborhoodDistanceIndex(distance_from_source_to_target);
    if (distance_index == -1) {
      return 0;
    }
    int n_size_up = sourceNodeAds->GetNeighborhoodDistanceByIndex(distance_index);
    int n_size_down = sourceNodeAds->GetNeighborhoodDistanceByIndex(distance_index - 1);
    LOG_M(DEBUG3, " Source=" << source <<
                  " node=" << target <<
                  " distance= " << distance_from_source_to_target <<
                  " n_size_up=" << n_size_up <<
                  " n_size_down=" << n_size_down <<
                  " index=" << distance_index);

    double reverse_rank = static_cast<double>(n_size_up + n_size_down + 1) / static_cast<double>(2);
    return reverse_rank;
}


template<class T>
class DefaultReverseRankCallBacks {
public:

  inline void Started(int source_node_id, graph::Graph<T>* graph) { return; }

  inline void NodePopedFromHeap(int poped_node_id, const RankData& heap_value) { return; }

  inline bool ShouldPrune(int visited_node_id, const RankData& rank_data) { return false; }

  inline bool ShouldStop() { return false; }

  inline void RelaxedPath(int node_id) { }

};

template <class T, class CallBacks>
static void CalculateReverseRank(int source_node_id,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking,
                       CallBacks* call_backs) {
    int max_node_id = graph->GetMxNId();
    ranking->clear();
    ranking->resize(max_node_id, constants::UNREACHABLE);
    (*ranking)[source_node_id] = 0;
    TBitSet poped; poped.resize(max_node_id);
    TBitSet touced; touced.resize(max_node_id);
    std::vector<graph::EdgeWeight> min_distance;
    min_distance.resize(max_node_id, constants::UNREACHABLE);
    min_distance[source_node_id] = 0;
    std::set< RankData, compareRankNode > heap;
    touced[source_node_id] = true;
    heap.insert(RankData(source_node_id, (*ranking)[source_node_id], 0));

    LOG_M(DEBUG4, "Starting Reverse rank from node=" << source_node_id);
    call_backs->Started(source_node_id, graph);
    while (!heap.empty()) {
        // Get the node with the minimum rank
        int visited_node_id = heap.begin()->node_id;
        graph::EdgeWeight distance_from_source_to_visited_node = heap.begin()->distance;
        (*ranking)[visited_node_id] = heap.begin()->rank;
        call_backs->NodePopedFromHeap(visited_node_id, *(heap.begin()));

        heap.erase(heap.begin());
        poped[visited_node_id] = true;

        if (call_backs->ShouldPrune(visited_node_id, (*heap.begin()) )) {
          continue;
        }

        if (call_backs->ShouldStop()) {
          return;
        }

        // Visit each edge exiting visited_node_id
        graph::TEdgesWeights * nodeWeights = graph->GetNodeWeights(visited_node_id);
        typename T::TNodeI neighbors = graph->GetNI(visited_node_id);
        for (int i = 0 ; i < neighbors.GetOutDeg(); i++) {
            int id_of_neighbor_of_visited_node = neighbors.GetOutNId(i);

            if (poped[id_of_neighbor_of_visited_node]) {
                continue;
            }
            std::pair<bool, graph::EdgeWeight> edge_u_v = graph->GetEdgeWeight(visited_node_id, id_of_neighbor_of_visited_node);
            graph::EdgeWeight distance_through_u = distance_from_source_to_visited_node + edge_u_v.second; // (*nodeWeights)[i].weight;
            LOG_M(DEBUG5, "edge weight between " <<
                          " visited_node_id = " << visited_node_id <<
                          " id_of_neighbor_of_visited_node = " << id_of_neighbor_of_visited_node <<
                          " edge weight = " << (*nodeWeights)[i].get_edge_weight());
            // relax
            LOG_M(DEBUG5, "Node " << visited_node_id
                        << " neighbor=" << id_of_neighbor_of_visited_node
                        << " Distance before relax=" << distance_from_source_to_visited_node
                        << " distance through visited_node_id=" << distance_through_u);

            if (touced[id_of_neighbor_of_visited_node] == false || distance_through_u <  min_distance[id_of_neighbor_of_visited_node]) {
                int rankOfSourceNode =  EstimateReverseRankUpperBound<T>(graph,
                                                    graph_sketch,
                                                    id_of_neighbor_of_visited_node,
                                                    source_node_id,
                                                    distance_through_u);

                if (touced[id_of_neighbor_of_visited_node] == false) {
                    LOG_M(DEBUG5, "Inserting node " << id_of_neighbor_of_visited_node <<
                                " to heap with distance " << distance_through_u);
                    heap.insert(RankData(id_of_neighbor_of_visited_node, rankOfSourceNode, distance_through_u));
                } else {
                    LOG_M(DEBUG5, "Decreasing distance of node " << id_of_neighbor_of_visited_node <<
                                " from " << min_distance[id_of_neighbor_of_visited_node] <<
                                " to " << distance_through_u);
                    heap.erase(RankData(id_of_neighbor_of_visited_node, (*ranking)[id_of_neighbor_of_visited_node], min_distance[id_of_neighbor_of_visited_node]));
                    heap.insert(RankData(id_of_neighbor_of_visited_node, rankOfSourceNode, distance_through_u));
                }
                LOG_M(DEBUG5, "Updating rank of node " << id_of_neighbor_of_visited_node <<
                            " rank=" << rankOfSourceNode <<
                            " old rank=" << (*ranking)[id_of_neighbor_of_visited_node]);
                touced[id_of_neighbor_of_visited_node] = true;
                min_distance[id_of_neighbor_of_visited_node] = distance_through_u;
                (*ranking)[id_of_neighbor_of_visited_node] = rankOfSourceNode;
            }
        }
    }
}


};  //  namespace all_distance_sketch
#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_
