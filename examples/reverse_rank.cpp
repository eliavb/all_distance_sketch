/*
	Calculate node id 3 ranks. The vector ranking will hold how each node i in the graph
	ranks node 3. If a there is no path between i to 3 then the value would be
	UNREACHABLE (see doc)
*/
using namespace all_distance_sketch;
template <class T>
void calculate_reverse_rank(graph::Graph<T>* graph) {
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(64, graph->GetMxNId());
  CalculateGraphSketch<T>(graph, &graphAds);
  int node_id = 3;
  std::vector<int> ranking;
  // Drop line to help doc tagging
  CalculateReverseRank
  						<T>(node_id,
                        	graph,
                        	&graph_sketch,
                        	&ranking);
}

/*
	For advanced use you can use the call back class.
	I will show how to find only the nodes that rank 3 in their top 100.
*/

/*
	This is the prototype of the call back class which we can use
	to extend to our needs. Since the function only takes a template
	argument there is no need for inheritance.
*/
template<class T>
class DefaultReverseRankCallBacks {
public:

  inline void Started(int source_node_id, graph::Graph<T>* graph) { return; }

  inline void NodePopedFromHeap(int poped_node_id, const RankData& heap_value) { return; }

  inline bool ShouldPrune(int visited_node_id, const RankData& rank_data) { return false; }

  inline bool ShouldStop() { return false; }

  inline void RelaxedPath(int node_id) { }

};

/*
	Build a class that will collect all ranks <= 100 and
	tell the algorithm to stop once he reaches nodes with rank 100.
*/
template<class T>
  class StopAfterReverseRankCallBacks {
  public:
  	// New init function
    inline void InitStopAfterReverseRankCallBacks(int stop_after_rank) {
      _stop_rank = stop_after_rank;
    }
    inline void Started(int source_node_id, graph::Graph<T>* graph) { return; }

    // Collect the node only if its rank is <= _stop_rank
    inline void NodePopedFromHeap(int poped_node_id, const RankData& heap_value) {
      if (heap_value.rank <= _stop_rank) {
        ranks[poped_node_id] = heap_value.rank;
      }
    }

    // Prune all nodes with rank > _stop_rank
    inline bool ShouldPrune(int visited_node_id, const RankData& rank_data) { 
      return rank_data.rank > _stop_rank; 
    }

    inline bool ShouldStop() { return false; }

    inline void RelaxedPath(int node_id) { }

    const std::unordered_map<int, int>& get_ranks() {
      return ranks;
    }
  private:
    int _stop_rank;
    std::unordered_map<int, int> ranks;
};
void calculate_reverse_rank_up_to_100() {
	// Load the graph
	graph::Graph< graph::TUnDirectedGraph> graph;
	graph.LoadGraphFromDir("path to data");
	GraphSketch graph_sketch;
	int k = 64;
	graph_sketch.InitGraphSketch(k, graph.GetMxNId());
	CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);
	int node_id = 3;
	std::vector<int> ranking;
	// Drop line to help doc tagging
	StopAfterReverseRankCallBacks<graph::TUnDirectedGraph> stop_after_100;
	stop_after_100.InitStopAfterReverseRankCallBacks(100);
	CalculateReverseRank
	          <graph::TUnDirectedGraph, StopAfterReverseRankCallBacks<graph::TUnDirectedGraph> >
	           	  (node_id,
	              &graph,
	              &graph_sketch,
	              &ranking,
	              &stop_after_100);
	for (auto node_rank : stop_after_100.get_ranks()) {
		EXPECT_TRUE(node_rank.second <= 100);
		std::cout << "node id=" << node_rank.first << " rank=" << node_rank.second << std::endl;
	}
}