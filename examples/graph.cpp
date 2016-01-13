
using namespace all_distance_sketch::graph;
/*! \brief How to build a directed graph
*/
void how_to_build_undirected_unweighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i) {
    EXPECT_TRUE(graph.IsEdge(i, i+1));
    EXPECT_TRUE(graph.IsEdge(i+1, i));
  }
}

void how_to_build_directed_unweighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i) {
    EXPECT_TRUE(graph.IsEdge(i, i+1));
  }
}

void how_to_build_undirected_weighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1, i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_TRUE(graph.IsEdge(i, i+1));
     EXPECT_TRUE(graph.IsEdge(i+1, i));
     std::pair<bool, EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_TRUE(res.first);
     EXPECT_EQ(res.second, i);
     // Check that the undirected graph 
     std::pair<bool, EdgeWeight> res1 = graph.GetEdgeWeight(i+1, i);
     EXPECT_TRUE(res1.first);
     EXPECT_EQ(res1.second, i);
  }
}