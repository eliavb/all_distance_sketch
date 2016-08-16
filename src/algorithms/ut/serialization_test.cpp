#include "gtest/gtest.h"
#include "../reverse_rank.h"
#include "../sketch_calculation.h"
#include "../../graph/snap_graph_adaptor.h"

using namespace all_distance_sketch;

class SerialGraph : public ::testing::Test {
 protected:
  SerialGraph() {}

  virtual ~SerialGraph() { }

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


TEST_F(SerialGraph, ReverseRankCalculation) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graph_sketch_orig;
  
  for (unsigned int i=1; i < 13; i++) {
    graph.AddNode(i);
  }
  graph.AddEdge(1, 2, 5);
  graph.AddEdge(2, 3, 2);
  graph.AddEdge(4, 3, 7);
  graph.AddEdge(5, 3, 3);
  graph.AddEdge(3, 8, 2);
  graph.AddEdge(6, 8, 4);
  graph.AddEdge(6, 7, 3);
  graph.AddEdge(8, 12, 1);
  graph.AddEdge(9, 12, 6);
  graph.AddEdge(10, 12, 4);
  graph.AddEdge(11, 12, 2);
  
  graph_sketch_orig.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graph_sketch_orig);
  /*
  Use loaded graph_sketch
  */
  AllDistanceSketchGpb all_distance_sketch_gpb;
  graph_sketch_orig.SaveGraphSketchToGpb(&all_distance_sketch_gpb);
  GraphSketch graph_sketch_after_load;
  graph_sketch_after_load.LoadGraphSketchFromGpb(all_distance_sketch_gpb);
  int sourceNodeId = 3;
  std::vector<int> ranking;
  DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
  CalculateReverseRank< graph::TUnDirectedGraph,
                        DefaultReverseRankCallBacks< graph::TUnDirectedGraph > >(sourceNodeId,
                                                                                 &graph,
                                                                                 &graph_sketch_after_load,
                                                                                 &ranking,
                                                                                 &reverse_rank_call_backs);
  NodeIdRandomIdData d(sourceNodeId, 0);
  // NodeIdDistanceVector * a = graph_sketch_orig.GetNodeSketch(d)->UTGetNodeAdsVector();
  
  EXPECT_EQ(ranking[1], 2);
  EXPECT_EQ(ranking[2], 1);
  EXPECT_EQ(ranking[3], 0);
  EXPECT_EQ(ranking[4], 1);
  EXPECT_EQ(ranking[5], 1);
  EXPECT_EQ(ranking[6], 4);
  EXPECT_EQ(ranking[7], 4);
  EXPECT_EQ(ranking[8], 2);
  EXPECT_EQ(ranking[9], 4);
  EXPECT_EQ(ranking[10], 4);
  EXPECT_EQ(ranking[11], 3);
  EXPECT_EQ(ranking[12], 3);
}

TEST_F(SerialGraph, ZValuesCalculation) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graph_sketch_before;
  int k = 2;
  // Random Id - Node Ids
  // 0.06 - 0, 0.12 - 1, 0.23 - 2
  // 0.32 - 3, 0.33 - 4, 0.34 - 5
  // 0.37 - 6, 0.45 - 7, 0.69 - 8
  // 0.77 - 9, 0.85 - 10, 0.93 - 11, 0.95 - 12
  std::vector<double> dist = {0.06, 0.12, 0.23,
                              0.32, 0.33, 0.34,
                              0.37, 0.45, 0.69,
                              0.77, 0.85, 0.93, 0.95};
  for (int i=0; i < dist.size(); i++) {
   graph.AddNode(i);
  }
  // 0.06 -> 0.95, 0.06 -> 0.32, 0.06 -> 0.85
  graph.AddEdge(0, 12); graph.AddEdge(0, 3); graph.AddEdge(0, 10);
  // 0.12 -> 0.34, 0.12 -> 0.77, 0.12 -> 0.93
  graph.AddEdge(1, 5); graph.AddEdge(1, 9); graph.AddEdge(1, 11);
  // 0.23 -> 0.45, 0.23 -> 0.37
  graph.AddEdge(2, 7); graph.AddEdge(2, 6);
  // 0.32 -> 0.69
  graph.AddEdge(3, 8);
  // 0.33 -> 0.77, 0.33 -> 0.93
  graph.AddEdge(4, 9); graph.AddEdge(4, 11);
  // 0.34 -> 0.12, 0.34 -> 0.85, 0.34 -> 0.93
  graph.AddEdge(5, 1); graph.AddEdge(5, 10); graph.AddEdge(5, 11);
  // 0.37 -> 0.45, 0.37 -> 0.85
  graph.AddEdge(6, 7); graph.AddEdge(6, 10);
  // 0.45 -> 0.85
  graph.AddEdge(7, 10);
  // 0.69 -> 0.06, 0.69 -> 0.45, 0.06 -> 0.77
  graph.AddEdge(8, 0); graph.AddEdge(8 ,7);graph.AddEdge(8 ,9);
  // 0.77 -> 0.93, 0.77 -> 0.34, 0.77 -> 0.45
  graph.AddEdge(9 ,11); graph.AddEdge(9 ,5); graph.AddEdge(9 ,7);
  // 0.85 -> 0.23
  graph.AddEdge(10 ,2);
  // 0.95 -> 0.32, 0.95 -> 0.69
  graph.AddEdge(12 ,3); graph.AddEdge(12 ,8);
  graph_sketch_before.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch_before.SetNodesDistribution(&dist);
  graph_sketch_before.set_should_calc_zvalues(true);
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_sketch_before);

  GraphSketch graph_sketch_after_load;
  AllDistanceSketchGpb all_distance_sketch_gpb;
  graph_sketch_before.SaveGraphSketchToGpb(&all_distance_sketch_gpb);
  graph_sketch_after_load.LoadGraphSketchFromGpb(all_distance_sketch_gpb);
  // Node 7
  std::vector<NodeProb> insert_prob;
  graph_sketch_after_load.CalculateInsertProb(7);
  insert_prob = graph_sketch_after_load.GetInsertProb(7);
  std::reverse(insert_prob.begin(), insert_prob.end());
  std::vector<int> node_ids = {7, 2, 6, 1, 0};
  std::vector<double> node_probs = {1, 0.45, 0.45, 0.32, 0.23 };
  EXPECT_EQ(insert_prob.size(), 5);
  for (int i=0; i < insert_prob.size(); i++) {
    EXPECT_EQ(insert_prob[i].node_id, node_ids[i]);
    EXPECT_EQ(insert_prob[i].prob, node_probs[i]);
  }

  // Node 9
  graph_sketch_after_load.CalculateInsertProb(9);
  insert_prob = graph_sketch_after_load.GetInsertProb(9);
  std::reverse(insert_prob.begin(), insert_prob.end());
  node_ids = {9, 1, 4, 3, 0};
  node_probs = {1, 0.69, 0.69, 0.33, 0.32};
  EXPECT_EQ(insert_prob.size(), 5);
  for (int i=0; i < insert_prob.size(); i++) {
    EXPECT_EQ(insert_prob[i].node_id, node_ids[i]);
    EXPECT_EQ(insert_prob[i].prob, node_probs[i]);
  }
}

TEST_F(SerialGraph, EqualityCheckCalculation) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Random Id - Node Ids
  // 0.06 - 0, 0.12 - 1, 0.23 - 2
  // 0.32 - 3, 0.33 - 4, 0.34 - 5
  // 0.37 - 6, 0.45 - 7, 0.69 - 8
  // 0.77 - 9, 0.85 - 10, 0.93 - 11, 0.95 - 12
  std::vector<double> dist = {0.06, 0.12, 0.23,
                              0.32, 0.33, 0.34,
                              0.37, 0.45, 0.69,
                              0.77, 0.85, 0.93, 0.95};
  for (int i=0; i < dist.size(); i++) {
   graph.AddNode(i);
  }
  // 0.06 -> 0.95, 0.06 -> 0.32, 0.06 -> 0.85
  graph.AddEdge(0, 12); graph.AddEdge(0, 3); graph.AddEdge(0, 10);
  // 0.12 -> 0.34, 0.12 -> 0.77, 0.12 -> 0.93
  graph.AddEdge(1, 5); graph.AddEdge(1, 9); graph.AddEdge(1, 11);
  // 0.23 -> 0.45, 0.23 -> 0.37
  graph.AddEdge(2, 7); graph.AddEdge(2, 6);
  // 0.32 -> 0.69
  graph.AddEdge(3, 8);
  // 0.33 -> 0.77, 0.33 -> 0.93
  graph.AddEdge(4, 9); graph.AddEdge(4, 11);
  // 0.34 -> 0.12, 0.34 -> 0.85, 0.34 -> 0.93
  graph.AddEdge(5, 1); graph.AddEdge(5, 10); graph.AddEdge(5, 11);
  // 0.37 -> 0.45, 0.37 -> 0.85
  graph.AddEdge(6, 7); graph.AddEdge(6, 10);
  // 0.45 -> 0.85
  graph.AddEdge(7, 10);
  // 0.69 -> 0.06, 0.69 -> 0.45, 0.06 -> 0.77
  graph.AddEdge(8, 0); graph.AddEdge(8 ,7);graph.AddEdge(8 ,9);
  // 0.77 -> 0.93, 0.77 -> 0.34, 0.77 -> 0.45
  graph.AddEdge(9 ,11); graph.AddEdge(9 ,5); graph.AddEdge(9 ,7);
  // 0.85 -> 0.23
  graph.AddEdge(10 ,2);
  // 0.95 -> 0.32, 0.95 -> 0.69
  graph.AddEdge(12 ,3); graph.AddEdge(12 ,8);

  GraphSketch graph_sketch_before;
  int k=64;
  graph_sketch_before.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch_before.SetNodesDistribution(&dist);
  graph_sketch_before.set_should_calc_zvalues(true);
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_sketch_before);
  GraphSketch graph_sketch_after_load;
  AllDistanceSketchGpb all_distance_sketch_gpb;
  graph_sketch_before.SaveGraphSketchToGpb(&all_distance_sketch_gpb);
  graph_sketch_after_load.LoadGraphSketchFromGpb(all_distance_sketch_gpb);
  EXPECT_TRUE(graph_sketch_after_load == graph_sketch_before);

}