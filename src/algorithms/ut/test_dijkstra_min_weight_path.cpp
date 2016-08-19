
#include "gtest/gtest.h"
#include "../dijkstra_min_weight_path.h"

using namespace all_distance_sketch;

class MaxAlgoGraph : public ::testing::Test {
 protected:
  MaxAlgoGraph() {}

  virtual ~MaxAlgoGraph() { }

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

TEST_F(MaxAlgoGraph, DijkstraSingleNode) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  graph.AddNode(0);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TUnDirectedGraph, DefaultDijkstraCallBacks<graph::TUnDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  EXPECT_EQ(param.max_distance[source.GetId()], constants::INF);
}

TEST_F(MaxAlgoGraph, DijkstraSingleNode2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  int num_nodes = 100;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }
  for (int i=0; i < num_nodes-1; i++) {
    graph.AddEdge(i,i+1, 1+i);
  }
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TUnDirectedGraph, DefaultDijkstraCallBacks<graph::TUnDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  for (int i=1; i < num_nodes; i++) {
    EXPECT_EQ(param.max_distance[i], 1);
  }
}

TEST_F(MaxAlgoGraph, DijkstraSingleNode3) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  int num_nodes = 100;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }
  for (int i=0; i < num_nodes-1; i++) {
    graph.AddEdge(i,i+1, 100-i);
  }
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TUnDirectedGraph, DefaultDijkstraCallBacks<graph::TUnDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  for (int i=1; i < num_nodes; i++) {
    EXPECT_EQ(param.max_distance[i], 101-i);
  }
}

TEST_F(MaxAlgoGraph, DijkstraSingleNode4) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 5;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }

  graph.AddEdge(0, 1, 1);
  graph.AddEdge(0, 2, 2);
  graph.AddEdge(0, 3, 3);
  graph.AddEdge(1, 4, 10);
  graph.AddEdge(2, 4, 10);
  graph.AddEdge(3, 4, 10);

  graph::TDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TDirectedGraph, DefaultDijkstraCallBacks<graph::TDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  for (int i=1; i < 4; i++) {
    EXPECT_EQ(param.max_distance[i], i);
  }
}

TEST_F(MaxAlgoGraph, DijkstraSingleNode5) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 5;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }

  graph.AddEdge(0, 1, 10);
  graph.AddEdge(0, 2, 10);
  graph.AddEdge(0, 3, 10);
  graph.AddEdge(1, 4, 1);
  graph.AddEdge(2, 4, 1);
  graph.AddEdge(3, 4, 1);

  graph::TDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TDirectedGraph, DefaultDijkstraCallBacks<graph::TDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  EXPECT_EQ(param.max_distance[4], 1);
}

TEST_F(MaxAlgoGraph, DijkstraSingleNode6) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 5;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }

  graph.AddEdge(0, 1, 10);
  graph.AddEdge(0, 2, 10);
  graph.AddEdge(0, 3, 10);
  graph.AddEdge(1, 4, 0.1);
  graph.AddEdge(2, 4, 0.5);
  graph.AddEdge(3, 4, 9);

  graph::TDirectedGraph::TNode source(0);
  DijkstraParamsMaxWeight param;
  DefaultDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  PrunedDijkstraMinWeight< graph::TDirectedGraph, DefaultDijkstraCallBacks<graph::TDirectedGraph> >
                          (source,
                          &graph,
                          &call_backs,
                          &param);
  EXPECT_EQ(param.max_distance[4], 9);
}
