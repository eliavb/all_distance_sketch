#include <limits.h>
#include "../../../include/gtest/include/gtest/gtest.h"
#include "../snap_graph_adaptor.h"

using namespace all_distance_sketch;

class BasicGraph : public ::testing::Test {
 protected:
  BasicGraph() {}

  virtual ~BasicGraph() { }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    mySampleData = "./data/facebook";
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  std::string GetSampleData() {return mySampleData;}

  std::string mySampleData;
};
// =======================================================================

// Create a basic 
TEST_F(BasicGraph, BuildGraph) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
     // EXPECT_EQ(graph.IsEdge(i+1, i), true);
  }
}
/*
TEST_F(BasicGraph, BuildDGraph) {
  all_distance_sketch::graph::DGraph graph;
  int count = 100;
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
  }
}
*/

TEST_F(BasicGraph, BuildGraphWithDefaultWeightsWeights) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
    graph.AddEdge(i, i+1);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, true);
     EXPECT_EQ(res.second, 1);
  }
}
/*
TEST_F(BasicGraph, BuildDGraphWithDefaultWeightsWeights) {
  all_distance_sketch::graph::DGraph graph;
  int count = 100;
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
    graph.AddEdge(i, i+1);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, true);
     EXPECT_EQ(res.second, 1);
  }
}*/

TEST_F(BasicGraph, BuildGraphWithWeightsWeights) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1, i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
     EXPECT_EQ(graph.IsEdge(i+1, i), true);
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, true);
     EXPECT_EQ(res.second, i);
     // Check that the undirected graph 
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res1 = graph.GetEdgeWeight(i+1, i);
     EXPECT_EQ(res1.first, true);
     EXPECT_EQ(res1.second, i);
  }

  // None existing edge
  for (int i = count; i < 2*count; ++i)
  {
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, false);
     EXPECT_EQ(res.second, all_distance_sketch::constants::UNREACHABLE);
  }
}

/*
TEST_F(BasicGraph, BuildDGraphWithWeightsWeights) {
  all_distance_sketch::graph::DGraph graph;
  int count = 100;
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
    graph.AddEdge(i, i+1, i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_EQ(graph.IsEdge(i, i+1), true);
     EXPECT_EQ(graph.IsEdge(i+1, i), false);
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, true);
     EXPECT_EQ(res.second, i);
     // Check that the undirected graph 
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res1 = graph.GetEdgeWeight(i+1, i);
     EXPECT_EQ(res1.first, false);
  }

  // None existing edge
  for (int i = count; i < 2*count; ++i)
  {
     std::pair<bool, all_distance_sketch::graph::EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_EQ(res.first, false);
     EXPECT_EQ(res.second, all_distance_sketch::constants::UNREACHABLE);
  }
}
*/

TEST_F(BasicGraph, checkNeighborhood) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  int count = 10;
  // 0 -> 1 -> 2 -> 3
  // 4 -> 5 -> 6
  // 7
  // 8 -> 9
  for (int i = 0; i < count; ++i)
  {
    graph.AddNode(i);
  }
  // 0 -> 1 -> 2 -> 3
  graph.AddEdge(0, 1);
  graph.AddEdge(1, 2);
  graph.AddEdge(2, 3);
  // 4 -> 5 -> 6
  graph.AddEdge(4, 5);
  graph.AddEdge(5, 6);
  // 8 -> 9
  graph.AddEdge(8, 9);
  // Check neghiborhood of 0
  all_distance_sketch::graph::TUnDirectedGraph::TNodeI NI = graph.GetNI(0);
  for (int i = 0; i < NI.GetDeg(); i++) {
    const int nid = NI.GetNbrNId(i);
    EXPECT_TRUE(nid == 1 || nid == 2 || nid == 3);
  }
  // Check neghiborhood of 4
  NI = graph.GetNI(4);
  for (int i = 0; i < NI.GetDeg(); i++) {
    const int nid = NI.GetNbrNId(i);
    EXPECT_TRUE(nid == 4 || nid == 5);
  }
  // Check neghiborhood of 7
  NI = graph.GetNI(7);
  EXPECT_EQ(NI.GetDeg(), 0);
  // Check neghiborhood of 8
  NI = graph.GetNI(8);
  for (int i = 0; i < NI.GetDeg(); i++) {
    const int nid = NI.GetNbrNId(i);
    EXPECT_TRUE(nid == 9);
  }
}

TEST_F(BasicGraph, SelfLoops){
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  // Note that the graph allows self loops
  graph.AddNode(0);
  graph.AddEdge(0, 0);

  all_distance_sketch::graph::TUnDirectedGraph::TNodeI NI = graph.GetNI(0);
  for (int i = 0; i < NI.GetDeg(); i++) {
    const int nid = NI.GetNbrNId(i);
    EXPECT_TRUE(nid == 0);
  }
}

// Load Graph from file repository
TEST_F(BasicGraph, LoadFiles){
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir(GetSampleData());
  EXPECT_EQ(graph.GetNumNodes(), 4039);
  EXPECT_EQ(graph.GetNumEdges(), 84243);
} 
/*
 Undirected graph: ../../data/output/youtube.ungraph.txt
 Youtube
 Nodes: 1134890 Edges: 2987624
 FromNodeId    ToNodeId
*/


TEST_F(BasicGraph, LoadFilesYoutube){
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  // 3959
  EXPECT_EQ(graph.GetNumNodes(), 1134890);
  // 84243
  EXPECT_EQ(graph.GetNumEdges(), 2987624);
} 


/*
TEST_F(BasicGraph, basicWeightMap) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddEdge(1, 2, 22);
  EXPECT_EQ(graph.IsEdge(1, 2), true);
  EXPECT_EQ(graph.IsEdge(2, 1), true);
  EXPECT_EQ( (all_distance_sketch::graph::EdgeWeight)graph.GetEdgeWeight(1, 2).second, (all_distance_sketch::graph::EdgeWeight)22);
  EXPECT_EQ( (all_distance_sketch::graph::EdgeWeight)graph.GetEdgeWeight(2, 1).second, (all_distance_sketch::graph::EdgeWeight)22);

  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > dgraph;
  dgraph.AddNode(1);
  dgraph.AddNode(2);
  dgraph.AddEdge(1, 2, 22);
  EXPECT_EQ(dgraph.IsEdge(1, 2), true);
  EXPECT_EQ(dgraph.IsEdge(2, 1), false);
  EXPECT_EQ( (all_distance_sketch::graph::EdgeWeight)dgraph.GetEdgeWeight(1, 2).second, (all_distance_sketch::graph::EdgeWeight)22);
  EXPECT_EQ( (all_distance_sketch::graph::EdgeWeight)dgraph.GetEdgeWeight(2, 1).second, (all_distance_sketch::graph::EdgeWeight)all_distance_sketch::constants::UNREACHABLE);
}
*/
