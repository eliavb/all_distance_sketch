
#include "gtest/gtest.h"
#include "../reverse_rank.h"
#include "../sketch_calculation.h"
#include "../../graph/snap_graph_adaptor.h"

using namespace all_distance_sketch;

class AlgoGraph : public ::testing::Test {
 protected:
  AlgoGraph() {}

  virtual ~AlgoGraph() { }

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

TEST_F(AlgoGraph, DijkstraSingleNode) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  graph.AddNode(0);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &call_backs,
                                                                                      &param);
  EXPECT_EQ(param.min_distance[source.GetId()], 0);
}

TEST_F(AlgoGraph, DijkstraSingleNodeDirected) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph::TDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  PrunedDijkstra< graph::TDirectedGraph, DefaultDijkstraCallBacks< graph::TDirectedGraph > >(source,
                                                      &graph,
                                                      &call_backs,
                                                      &param);
  // EXPECT_EQ(param.min_distance[source.GetId()], 0);
}

TEST_F(AlgoGraph, DijkstraSimpleGraph1) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 1 -> 2
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddEdge(0, 1);
  EXPECT_EQ(graph.GetEdgeWeight(0, 1).second, 1);
  EXPECT_EQ(graph.GetEdgeWeight(0, 1).first, true);
  EXPECT_EQ(graph.GetNI(0).GetOutDeg(), 1);
  EXPECT_EQ(graph.GetNI(0).GetOutNId(0), 1);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  EXPECT_EQ(param.min_distance[source.GetId()], 0);
  graph::TUnDirectedGraph::TNode target(1);
  EXPECT_EQ(param.min_distance[target.GetId()], 1);
  // EXPECT_EQ(previous[2], 1);
}

TEST_F(AlgoGraph, DijkstraSimpleGraphWithWeight) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 1 -> 2
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddEdge(0, 1, 100);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  EXPECT_EQ(param.min_distance[source.GetId()], 0);
  graph::TUnDirectedGraph::TNode target(1);
  EXPECT_EQ(param.min_distance[target.GetId()], 100);

}

TEST_F(AlgoGraph, DijkstraSimpleGraphWithWeight2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 0 -> 1
  // 1 -> 2
  // 3
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddEdge(0, 1, 10);
  graph.AddEdge(1, 2, 20);
  EXPECT_EQ(graph.GetEdgeWeight(0, 1).second, 10);
  EXPECT_EQ(graph.GetEdgeWeight(1, 2).second, 20);
  EXPECT_EQ(graph.GetNI(0).GetOutDeg(), 1);
  EXPECT_EQ(graph.GetNI(1).GetOutDeg(), 2);
  EXPECT_EQ(graph.GetNI(2).GetOutDeg(), 1);
  
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >
                                                                   (source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  EXPECT_EQ(param.min_distance[0], 0);
  // EXPECT_EQ(previous[1], -1);
  EXPECT_EQ(param.min_distance[1], 10);
  // EXPECT_EQ(previous[2], 1);
  EXPECT_EQ(param.min_distance[2], 30);
  // EXPECT_EQ(previous[3], 2);
  EXPECT_EQ(param.min_distance[3], constants::UNREACHABLE);
  // EXPECT_EQ(previous[4], -1);
}


TEST_F(AlgoGraph, DijkstraSimpleGraphWithWeight3) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 0 ->(10) 1 -> (20) 2
  // 0 ->(1) 3 ->(1) 2 ->(20) 1
  // 4
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddEdge(0, 1, 10);
  graph.AddEdge(1, 2, 20);
  graph.AddEdge(0, 3, 1);
  graph.AddEdge(3, 2, 1);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  EXPECT_EQ(param.min_distance[0], 0);
  EXPECT_EQ(param.min_distance[1], 10);
  EXPECT_EQ(param.min_distance[3], 1);
  EXPECT_EQ(param.min_distance[2], 2);
  EXPECT_EQ(param.min_distance[4], constants::UNREACHABLE);
}


TEST_F(AlgoGraph, DijkstraSimpleGraphWithZeroWeight) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 0 -> 1
  // 0 -> 2 -> 1
  // 4
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddEdge(0, 1, 10);
  graph.AddEdge(1, 2, 0);
  graph.AddEdge(0, 3, 0);
  graph.AddEdge(3, 2, 0);
  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  EXPECT_EQ(param.min_distance[0], 0);
  EXPECT_EQ(param.min_distance[1], 0);
  EXPECT_EQ(param.min_distance[2], 0);
  EXPECT_EQ(param.min_distance[3], 0);
  EXPECT_EQ(param.min_distance[4], constants::UNREACHABLE);
}

TEST_F(AlgoGraph, DijkstraGraph1) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 1 -> 2 -> 3 -> .... -> 99
  int count = 100;
  for (int i = 0; i < count; i++){
    graph.AddNode(i);
  }
    
  for (int i = 0; i < count - 1; i++){
    graph.AddEdge(i, i+1);
  }

  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  for (int i = 0; i < count; i++){
    EXPECT_EQ(param.min_distance[i], i);
    if (i != 0){
      // EXPECT_EQ(previous[i], i - 1);
    }
  }
}

TEST_F(AlgoGraph, DijkstraGraph2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  // 1 -> 2 -> 3 -> .... -> 99
  int count = 100;
  for (int i = 0; i < count; i++){
    graph.AddNode(i);
  }
    
  for (int i = 0; i < count - 1; i++){
    graph.AddEdge(i, i+1);
  }

  // Adding heavy edges that will not effect the graph
  for (int i = 0; i < count - 1; i++){
    int j = ( i * i ) % 100;
    int l = (j * j ) % 100;
    if (j == 0 || l == 0 || l == j +1){
      continue;
    }
    graph.AddEdge(j, l, 100000);
  }

  graph::TUnDirectedGraph::TNode source(0);
  DijkstraParams param;
  DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
  PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
  for (int i = 0; i < count; i++){
    EXPECT_EQ(param.min_distance[i], i);
    if (i != 0){
      // EXPECT_EQ(previous[i], i - 1);
    }
  }
}


TEST_F(AlgoGraph, ADSBasic) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddEdge(1, 2);
  graph.AddEdge(2, 3);
  // Init distribution
  graph::TUnDirectedGraph::TNode source(1);
  graphAds.InitGraphSketch(1, graph.GetMxNId());
  std::vector<RandomId> v;
  v.resize(4);
  v[0] = 0;
  v[1] = 0.1;
  v[2] = 0.2;
  v[3] = 0.3;
  graphAds.SetNodesDistribution(&v);

  DijkstraParams param;
  SketchDijkstraCallBacks<graph::TUnDirectedGraph> call_backs;
  call_backs.InitSketchDijkstraCallBacks(&graphAds);
  CalculateNodeSketch< graph::TUnDirectedGraph >  (source,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
 
  NodeIdRandomIdData d(2, 0.2);
  
  NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
  NodeIdDistanceVector nodeAdsVector;
  sourceSketch->Get(5, &nodeAdsVector);
  EXPECT_EQ(nodeAdsVector.size(), 1);
  // Only node 1 updated self with all other nodes
  for (unsigned int i=0; i < nodeAdsVector.size(); i++){
    // std::cout << nodeAdsVector[i].GetNId() << std::endl;
    EXPECT_EQ(nodeAdsVector[i].GetNId(), 1);
  }
}


TEST_F(AlgoGraph, ADSBasic2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3
  // 1 -> 4
  // d(1,2) = 1
  // d(1,3) = 2
  // d(1,4) = 1
  // NodeSketch should only contatin 2 and 4. (Case of tie braking in distance we take both)
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddEdge(1, 2);
  graph.AddEdge(2, 3);
  graph.AddEdge(1, 4);
  // Init distribution
  graph::TUnDirectedGraph::TNode source(1);
  graphAds.InitGraphSketch(1, graph.GetMxNId());
  std::vector<RandomId> v;
  v.resize(5);
  v[0] = 0;
  v[1] = 0.1;
  v[2] = 0.2;
  v[3] = 0.3;
  v[4] = 0.01;
  graphAds.SetNodesDistribution(&v);

  DijkstraParams param;
  SketchDijkstraCallBacks<graph::TUnDirectedGraph> call_backs;
  call_backs.InitSketchDijkstraCallBacks(&graphAds);
  CalculateNodeSketch< graph::TUnDirectedGraph >
                                                  (source,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
 
  for (unsigned int j = 2; j < 5; j++){
    NodeIdRandomIdData d(j, 0.1);
    NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
    NodeIdDistanceVector nodeAdsVector;
    sourceSketch->Get(5, &nodeAdsVector);
    EXPECT_EQ(nodeAdsVector.size(), 1);
    EXPECT_EQ(nodeAdsVector[0].GetNId(), 1);
  }
}

TEST_F(AlgoGraph, ADSBasicK2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3
  // 1 -> 4 -> 5
  // @ distance 1 from node 1 = (2, 0.2) (4. 0.01)
  // @ distance 2 from node 1 = (3, 0.3) (5, 0.001)
  // The NodeSketch sketch should contain both 2 and 4 @ distance 1
  // The NodeSketch sketch should contain node 5
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddEdge(1, 2);
  graph.AddEdge(2, 3);
  graph.AddEdge(1, 4);
  graph.AddEdge(4, 5);
  
  // Init distribution
  graph::TUnDirectedGraph::TNode source(1);
  graphAds.InitGraphSketch(2, graph.GetMxNId());
  EXPECT_EQ(graph.GetEdgeWeight(2, 1).second, 1);
  std::vector<RandomId> v;
  v.resize(6);
  v[0] = 0;
  v[1] = 0.1;
  v[2] = 0.2;
  v[3] = 0.3;
  v[4] = 0.01;
  v[5] = 0.001;
  graphAds.SetNodesDistribution(&v);
  EXPECT_EQ(graph.GetEdgeWeight(2, 1).second, 1);

  DijkstraParams param;
  SketchDijkstraCallBacks<graph::TUnDirectedGraph> call_backs;
  call_backs.InitSketchDijkstraCallBacks(&graphAds);
  CalculateNodeSketch< graph::TUnDirectedGraph > (source,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
  
  graph::TUnDirectedGraph::TNode source2(2);
  EXPECT_EQ(graph.GetEdgeWeight(2, 1).second, 1);
  CalculateNodeSketch< graph::TUnDirectedGraph >  (source2,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
 
  NodeIdRandomIdData d(1, 0.1);
  NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
  NodeIdDistanceVector nodeAdsVector;
  sourceSketch->Get(5, &nodeAdsVector);
  EXPECT_EQ(nodeAdsVector.size(), 2);
  EXPECT_EQ(nodeAdsVector[1].GetNId(), 2);
}


TEST_F(AlgoGraph, ADSBasicK2Directed) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3
  // 1 -> 4 -> 5
  // @ distance 1 from node 1 = (2, 0.2) (4. 0.01)
  // @ distance 2 from node 1 = (3, 0.3) (5, 0.001)
  // The NodeSketch sketch should contain both 2 and 4 @ distance 1
  // The NodeSketch sketch should contain node 5
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddEdge(1, 2);
  graph.AddEdge(2, 3);
  graph.AddEdge(1, 4);
  graph.AddEdge(4, 5);
  
  // Init distribution
  graph::TDirectedGraph::TNode source(1);
  graphAds.InitGraphSketch(2, graph.GetMxNId());
  EXPECT_EQ(graph.GetEdgeWeight(1, 2).second, 1);
  std::vector<RandomId> v;
  v.resize(6);
  v[0] = 0;
  v[1] = 0.1;
  v[2] = 0.2;
  v[3] = 0.3;
  v[4] = 0.01;
  v[5] = 0.001;
  graphAds.SetNodesDistribution(&v);
  EXPECT_EQ(graph.GetEdgeWeight(1, 2).second, 1);

  DijkstraParams param;
  SketchDijkstraCallBacks<graph::TDirectedGraph> call_backs;
  call_backs.InitSketchDijkstraCallBacks(&graphAds);
  CalculateNodeSketch< graph::TDirectedGraph > (source,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
  
  graph::TDirectedGraph::TNode source2(2);
  EXPECT_EQ(graph.GetEdgeWeight(1, 2).second, 1);
  CalculateNodeSketch< graph::TDirectedGraph > (source2,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
 
  NodeIdRandomIdData d(1, 0.1);
  NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
  NodeIdDistanceVector nodeAdsVector;
  sourceSketch->Get(5, &nodeAdsVector);
  EXPECT_EQ(nodeAdsVector.size(), 1);
  EXPECT_EQ(nodeAdsVector[0].GetNId(), 1);
}

TEST_F(AlgoGraph, ADSBasicFullGraphADS) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3 ... -> 99
  int count = 100;
  for (int i=0; i < count; ++i){
    graph.AddNode(i);
  }
    
  for (int i=0; i < count - 1; ++i){
    graph.AddEdge(i, i+1);
  }

  std::vector<RandomId> v;
  v.resize(count+1);
  
  for (int i = count -1; i >= 0; --i){
    // 1 gets the lowest and increase from there
    v[count - i] = 1 / (double)(i*10 + 1);
    // std::cout << " Node " << count -i << " hash=" << v[count - i] << std::endl;
  }

  // Init graph
  graphAds.InitGraphSketch(1, graph.GetMxNId());
  graphAds.SetNodesDistribution(&v);
  
  DijkstraParams param;
  SketchDijkstraCallBacks<graph::TUnDirectedGraph> call_backs;
  call_backs.InitSketchDijkstraCallBacks(&graphAds);
  for (int i=0; i < count; ++i){
      graph::TUnDirectedGraph::TNode source(i);
      CalculateNodeSketch< graph::TUnDirectedGraph > (source,
                                                  &graph,
                                                  &call_backs,
                                                  &param);
  }

  for (int i=0; i < count; ++i){
    NodeIdRandomIdData d(i, v[i]);
    NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
    NodeIdDistanceVector nodeAdsVector;
    sourceSketch->Get(100, &nodeAdsVector);
    if (i != 99 && i != 0){
      EXPECT_EQ(nodeAdsVector.size(), i + 1);
    }
    if (i == 0){
      EXPECT_EQ(nodeAdsVector.size(), 1);
    }
  }

}


TEST_F(AlgoGraph, ADSBasicFullGraphSketchDedicatedFunc) {
  std::vector<RandomId> v;
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3 ... -> 99
  int count = 100;
  for (int i=0; i < count; ++i){
    graph.AddNode(i);
  }
    
  for (int i=0; i < count - 1; ++i){
    graph.AddEdge(i, i+1);
  }

  
  v.resize(count+1);
  for (int i = count -1; i >= 0; --i){
    // 1 gets the lowest and increase from there
    v[count - i] = 1 / (double)(i*10 + 1);
    // std::cout << " Node " << count -i << " hash=" << v[count - i] << std::endl;
  }

  graphAds.InitGraphSketch(1, graph.GetMxNId());  
  graphAds.SetNodesDistribution(&v);
  // graphAds.CreateNodesDistribution(graph.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
  
  for (int i=0; i < count; ++i){
      NodeIdRandomIdData d(i, v[i]);
      NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
      NodeIdDistanceVector nodeAdsVector;
      sourceSketch->Get(100, &nodeAdsVector);
      if (i == 99){
          EXPECT_EQ(nodeAdsVector[1].GetNId(), 98);
      }
      else if (i == 0){
          EXPECT_EQ(nodeAdsVector.size(), 1);
      }
      else{
          // std::cout << "size=" << nodeAdsVector.size() << "i="  << i << std::endl;
          EXPECT_EQ(nodeAdsVector.size(), i + 1);
      }
  }
}

TEST_F(AlgoGraph, ADSBasicFullDirectedGraphSketchDedicatedFunc) {
  std::vector<RandomId> v;
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Adding some nodes and edges
  // 1 -> 2 -> 3 ... -> 99
  int count = 100;
  for (int i=0; i < count; ++i){
    graph.AddNode(i);
  }
    
  for (int i=1; i < count - 1; ++i){
    graph.AddEdge(i, i+1);
  }

  
  v.resize(count+1);
  for (int i = count -1; i > 0; --i){
    // 1 gets the lowest and increase from there
    v[count - i] = 1 / (double)(i*10 + 1);
    // std::cout << " Node " << count -i << " hash=" << v[count - i] << std::endl;
  }

  graphAds.InitGraphSketch(1, graph.GetMxNId());
  graphAds.SetNodesDistribution(&v);
  // graphAds.CreateNodesDistribution(graph.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
  
  for (int i=1; i < count; ++i){
      NodeIdRandomIdData d(i, v[i]);
      NodeSketch * sourceSketch = graphAds.GetNodeSketch(d);
      NodeIdDistanceVector nodeAdsVector;
      sourceSketch->Get(100, &nodeAdsVector);
      if (i == 1){
          EXPECT_EQ(nodeAdsVector.size(), 1);
      }
      else{
          EXPECT_EQ(nodeAdsVector.size(), i);
      }
  }
}



TEST_F(AlgoGraph, CalculateInsertProb) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graphAds;
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
  graphAds.InitGraphSketch(k, graph.GetMxNId());
  graphAds.SetNodesDistribution(&dist);
  graphAds.set_should_calc_zvalues(true);
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graphAds);

  // Node 7
  std::vector<NodeProb> insert_prob;
  graphAds.CalculateInsertProb(7, &insert_prob);
  std::reverse(insert_prob.begin(), insert_prob.end());
  std::vector<int> node_ids = {7, 2, 6, 1, 0};
  std::vector<double> node_probs = {1, 0.45, 0.45, 0.32, 0.23 };
  EXPECT_EQ(insert_prob.size(), 5);
  for (int i=0; i < insert_prob.size(); i++) {
    EXPECT_EQ(insert_prob[i].node_id, node_ids[i]);
    EXPECT_EQ(insert_prob[i].prob, node_probs[i]);
  }

  // Node 9
  graphAds.CalculateInsertProb(9, &insert_prob);
  std::reverse(insert_prob.begin(), insert_prob.end());
  node_ids = {9, 1, 4, 3, 0};
  node_probs = {1, 0.69, 0.69, 0.33, 0.32};
  EXPECT_EQ(insert_prob.size(), 5);
  for (int i=0; i < insert_prob.size(); i++) {
    EXPECT_EQ(insert_prob[i].node_id, node_ids[i]);
    EXPECT_EQ(insert_prob[i].prob, node_probs[i]);
  }
}

TEST_F(AlgoGraph, ZValueCalculation) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graphAds;
  int k = 2;
  std::vector<double> dist = {0.5, 0.8, 0.9};
  for (int i=0; i < dist.size(); i++) {
    graph.AddNode(i);
  }
  // 0.5 -> 0.8
  graph.AddEdge(0, 1);
  // 0.9 -> 0.8
  graph.AddEdge(2, 1);

  graphAds.InitGraphSketch(k, graph.GetMxNId());
  graphAds.SetNodesDistribution(&dist);
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graphAds);

  std::vector<NodeProb> insert_prob;
  graphAds.CalculateInsertProb(1, &insert_prob);
  std::reverse(insert_prob.begin(), insert_prob.end());
  
  EXPECT_EQ(insert_prob.size(), 2);
  std::vector<int> node_ids = {1, 0};
  std::vector<double> node_probs = {1, 0.9 };
  for (int i=0; i < insert_prob.size(); i++) {
    EXPECT_EQ(insert_prob[i].node_id, node_ids[i]);
    EXPECT_EQ(insert_prob[i].prob, node_probs[i]);
  }
}


TEST_F(AlgoGraph, SketchCalculationFacebookGraphApproximation) {
  std::vector<int> histo;
  histo.resize(101, 0);
  int numCompared = 0;
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  
  graph.LoadGraphFromDir(GetSampleData());
  graphAds.InitGraphSketch(100, graph.GetMxNId());
  
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
  int d = 0;
  // std::cout << "Node Id,distance,neighborhood_size,neighborhood_size_approx,delta" << std::endl;

  for (graph::TUnDirectedGraph::TNodeI nodeItr = graph.BegNI();  /*nodeItr.HasMore()*/ nodeItr != graph.EndNI() ; nodeItr++ ){
    DijkstraParams param;
    graph::TUnDirectedGraph::TNode source(nodeItr.GetId());
    // std::cout << " Starting node " << nodeItr.GetId() << std::endl;
    DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
    PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
    // std::cout << " Finished node " << nodeItr.GetId() << std::endl;
    graph::EdgeWeight max_distance = 0;
    for (unsigned int i =0 ; i < param.min_distance.size(); i++){
      if (param.min_distance[i] != constants::UNREACHABLE){
        max_distance = max_distance > param.min_distance[i] ? max_distance : param.min_distance[i];
      }
    }

    NodeIdRandomIdData sourceDetails(nodeItr.GetId(), graphAds.GetNodeRandomId(nodeItr.GetId()));
    NodeSketch * nodeAds = graphAds.GetNodeSketch(sourceDetails);

    for (unsigned int i = 0; i < max_distance; i++){
      int num_neighbors = 0;
      for (unsigned int j=0; j < param.min_distance.size(); j++){
        if (param.min_distance[j] <= i && j != (unsigned int)nodeItr.GetId()){
          num_neighbors += 1;
        }
      }

      int num_neighbors_approximate = nodeAds->GetSizeNeighborhoodUpToDistance(i, graphAds.GetNodesDistributionLean());
      if (num_neighbors == 0) {
        continue;
      }
      double relativeError = ((double)num_neighbors - (double)num_neighbors_approximate) / (double)num_neighbors;
      if (relativeError < 0) {
        relativeError = -relativeError;
      }
      
      numCompared += 1;
      if (relativeError >= 1) {
        histo[100] += 1;
      } else {
        int bean = relativeError / 0.01;
        histo[bean] += 1;
      }
      /*
      std::cout << nodeItr.GetId() << "," << i << "," 
                  << num_neighbors << "," 
                  << num_neighbors_approximate << "," 
                  << num_neighbors - num_neighbors_approximate << "," 
                  << std::endl;
      */
      int a = (num_neighbors - num_neighbors_approximate) > 0 ? (num_neighbors - num_neighbors_approximate) : (-num_neighbors + num_neighbors_approximate);
      d = d > a ? d : a;
      
    }
    
  }
  int expected_values[] = { 6107, 1342, 1367, 1199, 1265, 1747, 1292, 1888, 3857, 7101, 7682, 2722,
 982, 411, 413, 196, 184, 331, 630, 138, 101, 178, 63,  120, 63, 29,  242, 165, 51,  28,  118, 118, 10,  1 };
  
  for (unsigned int i=0; i < 33; i++) {
    EXPECT_EQ(histo[i], expected_values[i]);
  }
  
}


#if 0
TEST_F(AlgoGraph, ADSCalculationFacebookGraphApproximationSearlization) {
  std::vector<int> histo;
  histo.resize(101, 0);
  int numCompared = 0;
  graph::Graph< graph::TUnDirectedGraph > graphtemp;
  GraphSketch graphAdsTemp;
  
  graphtemp.LoadGraphFromDir(GetSampleData());
  graphAdsTemp.InitGraphSketch(100, graphtemp.GetMxNId());
  graphAdsTemp.CreateNodesDistribution(graphtemp.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graphtemp, &graphAdsTemp);
  int d = 0;
  std::ofstream ofs("./out/graph/state/facebook_k_100");
  {
    boost::archive::text_oarchive oa(ofs);
    // oa << graphtemp;
    oa << graphAdsTemp;
  }

  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  graph.LoadGraphFromDir(GetSampleData());
  {
        // create and open an archive for input
        std::ifstream ifs("./out/graph/state/facebook_k_100");
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        // ia >> graph;
        ia >> graphAds;
        // archive and stream closed when destructors are called
  }
  // std::cout << "Node Id,distance,neighborhood_size,neighborhood_size_approx,delta" << std::endl;
  
  for (graph::TUnDirectedGraph::TNodeI nodeItr = graph.BegNI(); nodeItr != graph.EndNI(); nodeItr++ ){
    DijkstraParams param;
    param.set_lean(false);
    graph::TUnDirectedGraph::TNode source(nodeItr.GetId());
    PrunedDijkstra< graph::TUnDirectedGraph >( source,
                                          &graph,
                                          &param);
    
    graph::EdgeWeight max_distance = 0;
    for (unsigned int i =0 ; i < param.min_distance.size(); i++){
      if (param.min_distance[i] != constants::UNREACHABLE){
        max_distance = max_distance > param.min_distance[i] ? max_distance : param.min_distance[i];
      }
    }

    NodeIdRandomIdData sourceDetails(nodeItr.GetId(), graphAds.GetNodeRandomId(nodeItr.GetId()));
    NodeSketch * nodeAds = graphAds.GetNodeSketch(sourceDetails);

    for (unsigned int i = 0; i < max_distance; i++){
      int num_neighbors = 0;
      for (unsigned int j=0; j < param.min_distance.size(); j++){
        if (param.min_distance[j] <= i && j != (unsigned int)nodeItr.GetId()){
          num_neighbors += 1;
        }
      }

      int num_neighbors_approximate = nodeAds->GetSizeNeighborhoodUpToDistance(i, graphAds.GetNodesDistributionLean());
      if (num_neighbors == 0) {
        continue;
      }
      double relativeError = ((double)num_neighbors - (double)num_neighbors_approximate) / (double)num_neighbors;
      if (relativeError < 0) {
        relativeError = -relativeError;
      }
      
      numCompared += 1;
      if (relativeError >= 1) {
        histo[100] += 1;
      } else {
        int bean = relativeError / 0.01;
        histo[bean] += 1;
      }
      /*
      std::cout << nodeItr.GetId() << "," << i << "," 
                  << num_neighbors << "," 
                  << num_neighbors_approximate << "," 
                  << num_neighbors - num_neighbors_approximate << "," 
                  << std::endl;
      */
      int a = (num_neighbors - num_neighbors_approximate) > 0 ? (num_neighbors - num_neighbors_approximate) : (-num_neighbors + num_neighbors_approximate);
      d = d > a ? d : a;
      
    }
    
  }

  int expected_values[] = { 6107, 1342, 1367, 1199, 1265, 1747, 1292, 1888, 3857, 7101, 7682, 2722,
 982, 411, 413, 196, 184, 331, 630, 138, 101, 178, 63,  120, 63, 29,  242, 165, 51,  28,  118, 118, 10,  1 };
  
  for (unsigned int i=0; i < 33; i++) {
    EXPECT_EQ(histo[i], expected_values[i]);
  }
  
}


TEST_F(AlgoGraph, ADSCalculationSlashdotGraphApproximation) {
  GraphSketch graphAds;
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  graphAds.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
}

TEST_F(AlgoGraph, ADSCalculationMultiSlashdotGraphApproximation) {
  GraphSketch graphAds;
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  graphAds.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketchMultiCore< graph::TUnDirectedGraph >(&graph, &graphAds);
}
#endif
TEST_F(AlgoGraph, ADSCalculationMultiFacebookGraphApproximation) {
  std::vector<int> histo;
  histo.resize(101, 0);
  int numCompared = 0;
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  
  graph.LoadGraphFromDir(GetSampleData());
  graphAds.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketchMultiCore< graph::TUnDirectedGraph >(&graph, &graphAds);
  int d = 0;
  // std::cout << "Node Id,distance,neighborhood_size,neighborhood_size_approx,delta" << std::endl;

  for (graph::TUnDirectedGraph::TNodeI nodeItr = graph.BegNI();  /*nodeItr.HasMore()*/ nodeItr != graph.EndNI() ; nodeItr++ ){
    DijkstraParams param;
    graph::TUnDirectedGraph::TNode source(nodeItr.GetId());
    DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
    PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >(source,
                                                                    &graph,
                                                                    &call_backs,
                                                                    &param);
    
    graph::EdgeWeight max_distance = 0;
    for (unsigned int i =0 ; i < param.min_distance.size(); i++){
      if (param.min_distance[i] != constants::UNREACHABLE){
        max_distance = max_distance > param.min_distance[i] ? max_distance : param.min_distance[i];
      }
    }

    NodeIdRandomIdData sourceDetails(nodeItr.GetId(), graphAds.GetNodeRandomId(nodeItr.GetId()));
    NodeSketch * nodeAds = graphAds.GetNodeSketch(sourceDetails);

    for (unsigned int i = 0; i < max_distance; i++){
      int num_neighbors = 0;
      for (unsigned int j=0; j < param.min_distance.size(); j++){
        if (param.min_distance[j] <= i && j != (unsigned int)nodeItr.GetId()){
          num_neighbors += 1;
        }
      }

      int num_neighbors_approximate = nodeAds->GetSizeNeighborhoodUpToDistance(i, graphAds.GetNodesDistributionLean());
      if (num_neighbors == 0) {
        continue;
      }
      double relativeError = ((double)num_neighbors - (double)num_neighbors_approximate) / (double)num_neighbors;
      if (relativeError < 0) {
        relativeError = -relativeError;
      }
      
      numCompared += 1;
      if (relativeError >= 1) {
        histo[100] += 1;
      } else {
        int bean = relativeError / 0.01;
        histo[bean] += 1;
      }
      /*
      std::cout << nodeItr.GetId() << "," << i << "," 
                  << num_neighbors << "," 
                  << num_neighbors_approximate << "," 
                  << num_neighbors - num_neighbors_approximate << "," 
                  << std::endl;
      */
      int a = (num_neighbors - num_neighbors_approximate) > 0 ? (num_neighbors - num_neighbors_approximate) : (-num_neighbors + num_neighbors_approximate);
      d = d > a ? d : a;
      
    }
    
  }
  
  int expected_values[] = { 6107, 1342, 1367, 1199, 1265, 1747, 1292, 1888, 3857, 7101, 7682, 2722,
 982, 411, 413, 196, 184, 331, 630, 138, 101, 178, 63,  120, 63, 29,  242, 165, 51,  28,  118, 118, 10,  1 };
  for (unsigned int i=0; i < 33; i++) {
    EXPECT_EQ(histo[i], expected_values[i]);
  }
}


TEST_F(AlgoGraph, ReverseRankAverageCalculation) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graph_ads;
  for (int i=0; i < 7; i++) {
    graph.AddNode(i);
  }
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 1);
  graph.AddEdge(5, 2);
  graph.AddEdge(6, 3);

  graph_ads.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_ads);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 0, 0), 0);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 1, 1), 2);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 2, 1), 2);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 3, 1), 2);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 4, 2), 5);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 5, 2), 5);
  EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, 6, 2), 5);
}


TEST_F(AlgoGraph, ReverseRankAverageCalculation1) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graph_ads;
  for (int i=0; i < 100; i++) {
    graph.AddNode(i);
  }
  for (int i=0; i < 100; i++) {
    graph.AddEdge(i, 0);
  }


  graph_ads.InitGraphSketch(101, graph.GetMxNId());
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_ads);
  for (int i=1; i < 101; i++) {
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 1), 50);
  }
}

TEST_F(AlgoGraph, ReverseRankAverageCalculation2) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graph_ads;
  for (int i=0; i < 200; i++) {
    graph.AddNode(i);
  }
  for (int i=0; i < 100; i++) {
    graph.AddEdge(i, 0);
  }

  for (int i=100; i < 200; i++) {
    graph.AddEdge(i, 99);
  }

  graph_ads.InitGraphSketch(201, graph.GetMxNId());
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_ads);
  for (int i=1; i < 100; i++) {
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 1), 50);
  }
  for (int i=101; i < 200; i++) {
    // 99 + 199 + 1 / 2
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 2), 149.5);
  }
}

TEST_F(AlgoGraph, ReverseRankAverageCalculation3) {
  graph::Graph< graph::TDirectedGraph > graph;
  GraphSketch graph_ads;
  for (int i=0; i < 300; i++) {
    graph.AddNode(i);
  }
  for (int i=0; i < 100; i++) {
    graph.AddEdge(i, 0);
  }

  for (int i=100; i < 200; i++) {
    graph.AddEdge(i, 99);
  }

  for (int i=200; i < 300; i++) {
    graph.AddEdge(i, 199);
  }

  graph_ads.InitGraphSketch(301, graph.GetMxNId());
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_ads);
  for (int i=1; i < 100; i++) {
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 1), 50);
  }
  for (int i=101; i < 200; i++) {
    // 99 + 199 + 1 / 2
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 2), 149.5);
  }

  for (int i=201; i < 300; i++) {
    // 199 + 299  + 1 / 2 =
    EXPECT_EQ(EstimateReverseRankAverage(&graph, &graph_ads, 0, i, 3), 249.5);
  }
}

TEST_F(AlgoGraph, RNNTree) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  
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
  
  graphAds.InitGraphSketch(100, graph.GetMxNId());
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
  int sourceNodeId = 3;
  std::vector<int> ranking;
  DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
  CalculateReverseRank< graph::TUnDirectedGraph,
                        DefaultReverseRankCallBacks< graph::TUnDirectedGraph > >(sourceNodeId,
                                                                                 &graph,
                                                                                 &graphAds,
                                                                                 &ranking,
                                                                                 &reverse_rank_call_backs);
  NodeIdRandomIdData d(sourceNodeId, 0);
  // NodeIdDistanceVector * a = graphAds.GetNodeSketch(d)->UTGetNodeAdsVector();
  
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


TEST_F(AlgoGraph, BasicRNN2) {
  graph::Graph< graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  // Nodes: 1, ...., 99
  int count = 100;
  for (int i=1; i < count; ++i){
    graph.AddNode(i);
  }

  for (int i=1; i < count - 1; ++i){
    graph.AddEdge(i, i+1);
  }


  graphAds.InitGraphSketch(100, graph.GetMxNId());

  std::vector<int> ranking;
  CalculateGraphSketch< graph::TUnDirectedGraph >(&graph, &graphAds);
  DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
  CalculateReverseRank< graph::TUnDirectedGraph >(1,
                                                   &graph,
                                                   &graphAds,
                                                   &ranking,
                                                   &reverse_rank_call_backs);
  
  for (unsigned int j=1; j < ranking.size(); ++j){
    if (j == 1){
      EXPECT_EQ(ranking[j], 0);
    }
    else if (j < 50 ) {
      // std::cout << " ranking j = " << ranking[j] << " j= " << j << std::endl;
      EXPECT_TRUE(ranking[j] == (int)((j * 2) - 2));
    }
    else {
     EXPECT_TRUE(ranking[j] == 98); 
    }
  }
}

/*
TEST_F(AlgoGraph, Analysis1) {
  std::vector<int> nodes = {40, 78, 511, 368749, 3394, 480};
  const std::string file_name = "/work/eng/eliavb/all_distance_sketch/out/all_distance_sketch/youtube_signal_compare.csv";
  std::string del = "\t";
  std::ofstream file(file_name);
  file << "Name,CoverSize,Signal\n";
  file.flush();
  graph::Graph< graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  GraphSketch graph_sketch;
  graph_sketch.InitGraphSketch(128, graph.GetMxNId());
  CalculateGraphSketch(&graph, &graph_sketch);
  for (int i=0; i < nodes.size(); i++) {
    std::vector<int> ranking;
    DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
    CalculateReverseRank< graph::TUnDirectedGraph,
                        DefaultReverseRankCallBacks< graph::TUnDirectedGraph > > 
                        (nodes[i],
                        &graph,
                        &graph_sketch,
                        &ranking,
                        &reverse_rank_call_backs);
    file << "ReverseRank" + del;
    file << std::to_string(static_cast<long long>(nodes[i]));
    file << del;
    std::string seperator = ",";
    for (int j=0; j < ranking.size(); j++) {
      if (ranking[j] == constants::UNREACHABLE) {
        continue;
      }
      file << std::to_string(static_cast<long long>(ranking[j])) + seperator;
    }
    file << "\n";
    file.flush();
    graph::TUnDirectedGraph::TNode source(nodes[i]);
    DijkstraParams param;
    DefaultDijkstraCallBacks< graph::TUnDirectedGraph > call_backs;
    PrunedDijkstra< graph::TUnDirectedGraph, DefaultDijkstraCallBacks< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &call_backs,
                                                                                      &param);

    file << "Distance" + del;
    file << std::to_string(static_cast<long long>(nodes[i]));
    file << del;
    for (int j=0; j < param.min_distance.size(); j++) {
      if (param.min_distance[j] == constants::UNREACHABLE) {
        continue;
      }
      file << std::to_string(static_cast<long long>(param.min_distance[j])) + seperator;
    }
    file << "\n";                                                                                     
    file.flush();
  }
}*/

template<class T>
  class StopAfterReverseRankCallBacks {
  public:
    inline void InitStopAfterReverseRankCallBacks(int stop_after_rank) {
      _stop_rank = stop_after_rank;
    }
    inline void Started(int source_node_id, graph::Graph<T>* graph) { return; }

    inline void NodePopedFromHeap(int poped_node_id, const RankData& heap_value) {
      if (heap_value.rank <= _stop_rank) {
        ranks[poped_node_id] = heap_value.rank;
      }
    }

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
TEST_F(AlgoGraph, CheckRankUpTo100) {
  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
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
