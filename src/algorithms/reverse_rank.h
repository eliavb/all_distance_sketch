#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_

#include "../common.h"
#include "../sketch/graph_sketch.h"

/*! \file reverse_rank.h
    \brief Contains all reverse rank algorithms
*/

/*! project namespace */
namespace all_distance_sketch {

/*! \brief Calculates the reverse ranks of a single node
  \fn CalculateReverseRank(int source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking)
  \param[in] source - node id. for this node the algorithm will calculate how all the other nodes rank it.
  \param[in] graph - The graph to run the calculation on.
  \param[in] graph_sketch - See doc for more infromation.
  \param[out] ranking - here the result of the calculation is stored. 
            the ranking of j to source will be stored in (*ranking)[j]
            If there is not path between j and source then (*ranking)[j] == constants::UNREACHABLE
            The same goes for j such that j is not a node in the graph
  \see GraphSketch
  \see graph::Graph
  \see constants::UNREACHABLE
*/
template <class T>
static void CalculateReverseRank(int source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking);

/*! \brief Calculates the reverse ranks of a single node
\fn CalculateReverseRank(int source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking,
                       CallBacks* call_backs)
  \param[in] source - node id. for this node the algorithm will calculate how all the other nodes rank it.
  \param[in] graph - The graph to run the calculation on.
  \param[in] graph_sketch - See doc for more infromation.
  \param[out] ranking - here the result of the calculation is stored. 
            the ranking of j to source will be stored in (*ranking)[j].
            If there is not path between j and source then (*ranking)[j] == constants::UNREACHABLE
            The same goes for j such that j is not a node in the graph
  \param[in] call_backs - Call backs class with function that will be called in each major event.
                          It can be used to stop the calucation once we reached all nodes that rank 
                          source up to a certain level e.g. stop when ranking[i] >= 100
  \see GraphSketch
  \see graph::Graph
  \see constants::UNREACHABLE
*/
template <class T, class CallBacks>
static void CalculateReverseRank(int source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking,
                       CallBacks* call_backs);

/*! \cond
*/
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
        if (n1.rank < n2.rank) {
            return true;
        }
        if (n1.rank > n2.rank) {
            return false;
        }
        if (n1.distance < n2.distance) {
            return true;
        }
        if (n1.distance > n2.distance) {
            return false;
        }
        if (n1.node_id < n2.node_id) {
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
    int reverse_rank = sourceNodeAds->GetSizeNeighborhoodUpToDistance(distance_from_source_to_targer);
    LOG_M(DEBUG3, " Source=" << source <<
                  " node=" << target <<
                  " distance= " << distance_from_source_to_targer <<
                  " reverse_rank=" << reverse_rank <<
                  " Node degree=" << graph->GetNI(source).GetOutDeg());
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


template <class T>
static void CalculateReverseRank(int source,
                       graph::Graph<T> * graph,
                       GraphSketch * graph_sketch,
                       std::vector<int> * ranking) {
  DefaultReverseRankCallBacks<T> default_reverse_rank_call_backs;
  CalculateReverseRank(source, graph, graph_sketch, ranking, &default_reverse_rank_call_backs);
}

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

    LOG_M(DEBUG3, "Starting Reverse rank from node=" << source_node_id);
    call_backs->Started(source_node_id, graph);
    double rank_last = 0;
    while (!heap.empty()) {
        // Get the node with the minimum rank
        int visited_node_id = heap.begin()->node_id;
        auto node_data = *heap.begin();
        graph::EdgeWeight distance_from_source_to_visited_node = node_data.distance;
        (*ranking)[visited_node_id] = node_data.rank;
        call_backs->NodePopedFromHeap(visited_node_id, node_data);
        LOG_M(DEBUG3, "Poped node=" << visited_node_id << " Distance=" << node_data.distance <<
                      " Rank=" << node_data.rank);
        assert(rank_last <= node_data.rank);
        rank_last = node_data.rank;
        _unused(rank_last);
        heap.erase(heap.begin());
        poped[visited_node_id] = true;
        if (call_backs->ShouldPrune(visited_node_id, node_data)) {
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
            LOG_M(DEBUG3, "neighbors node=" << id_of_neighbor_of_visited_node);
            if (poped[id_of_neighbor_of_visited_node]) {
                continue;
            }
            std::pair<bool, graph::EdgeWeight> edge_u_v = graph->GetEdgeWeight(visited_node_id, id_of_neighbor_of_visited_node);
            graph::EdgeWeight distance_through_u = distance_from_source_to_visited_node + edge_u_v.second; // (*nodeWeights)[i].weight;
            LOG_M(DEBUG3, "edge weight between " <<
                          " visited_node_id = " << visited_node_id <<
                          " id_of_neighbor_of_visited_node = " << id_of_neighbor_of_visited_node <<
                          " edge weight = " << (*nodeWeights)[i].get_edge_weight() <<
                          " Distance before relax=" << distance_from_source_to_visited_node <<
                          " distance through visited_node_id=" << distance_through_u);

            if (touced[id_of_neighbor_of_visited_node] == false || distance_through_u <  min_distance[id_of_neighbor_of_visited_node]) {
                int rankOfSourceNode =  EstimateReverseRankUpperBound<T>(graph,
                                                    graph_sketch,
                                                    id_of_neighbor_of_visited_node,
                                                    source_node_id,
                                                    distance_through_u);

                if (touced[id_of_neighbor_of_visited_node] == false) {
                    LOG_M(DEBUG3, "Inserting node " << id_of_neighbor_of_visited_node <<
                                " to heap with distance " << distance_through_u);
                    heap.insert(RankData(id_of_neighbor_of_visited_node, rankOfSourceNode, distance_through_u));
                } else {
                    LOG_M(DEBUG3, "Decreasing distance of node " << id_of_neighbor_of_visited_node <<
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
/*! \endcond
*/
/*!
  \example examples/reverse_rank.cpp
  Examples on how to use CalculateReverseRank functions
*/
};  //  namespace all_distance_sketch
#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_ALGORITHMS_REVERSE_RANK_H_
