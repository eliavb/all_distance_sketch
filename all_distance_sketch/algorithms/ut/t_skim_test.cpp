#include "../t_skim.h"
#include "../../graph/snap_graph_adaptor.h"

namespace all_distance_sketch {

// EXAMPLE: With a test fixture and TEST_F_Fs
// A test fixture is used because {TODO(eliav): put reason here}.
class TSkimTest : public ::testing::Test {
 protected:

  TSkimTest() {}

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

TEST_F(TSkimTest, CheckCover) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Building a tree
  // 0 -> 1, 0 -> 2
  // 1 -> 3, 1 -> 4
  // 2 -> 5, 2 -> 6
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddEdge(0, 1);
  graph.AddEdge(0, 2);
  graph.AddEdge(1, 3);
  graph.AddEdge(1, 4);
  graph.AddEdge(2, 5);
  graph.AddEdge(2, 6);
  DijkstraParams param;
  TSkimDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  call_backs.InitTSkimDijkstraCallBacks(3);
  graph::TDirectedGraph::TNode source(0);
  PrunedDijkstra< graph::TDirectedGraph,
                  TSkimDijkstraCallBacks< graph::TDirectedGraph > > (source,
                                                                      &graph,
                                                                      &call_backs,
                                                                      &param);
  EXPECT_EQ(call_backs.visited_nodes_including_self_.size(), 3);
  std::vector<int> visited = {0, 1, 2};
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), call_backs.visited_nodes_including_self_.begin()));

  call_backs.InitTSkimDijkstraCallBacks(3);
  graph::TDirectedGraph::TNode source2(1);
  PrunedDijkstra< graph::TDirectedGraph,
                  TSkimDijkstraCallBacks< graph::TDirectedGraph > > (source2,
                                                                      &graph,
                                                                      &call_backs,
                                                                      &param);
  EXPECT_EQ(call_backs.visited_nodes_including_self_.size(), 3);
  std::vector<int> visited2 = {1, 3, 4};
  EXPECT_TRUE( std::is_permutation(visited2.begin(), visited2.end(), call_backs.visited_nodes_including_self_.begin()));

  call_backs.InitTSkimDijkstraCallBacks(1);
  graph::TDirectedGraph::TNode source3(4);
  PrunedDijkstra< graph::TDirectedGraph,
                  TSkimDijkstraCallBacks< graph::TDirectedGraph > > (source3,
                                                                      &graph,
                                                                      &call_backs,
                                                                      &param);
  EXPECT_EQ(call_backs.visited_nodes_including_self_.size(), 1);
}

TEST_F(TSkimTest, CheckCover2) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Building a tree
  // 0 -> 1, 0 -> 2
  // 1 -> 3, 1 -> 4
  // 2 -> 5, 2 -> 6
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddEdge(0, 1);
  graph.AddEdge(0, 2);
  graph.AddEdge(1, 3);
  graph.AddEdge(1, 4);
  graph.AddEdge(2, 5);
  graph.AddEdge(2, 6);
  DijkstraParams param;
  TSkimDijkstraCallBacks< graph::TDirectedGraph > call_backs;
  call_backs.InitTSkimDijkstraCallBacks(100);
  graph::TDirectedGraph::TNode source(0);
  PrunedDijkstra< graph::TDirectedGraph,
                  TSkimDijkstraCallBacks< graph::TDirectedGraph > > (source,
                                                                      &graph,
                                                                      &call_backs,
                                                                      &param);
  EXPECT_EQ(call_backs.visited_nodes_including_self_.size(), 7);
  std::vector<int> visited = {0, 1, 2, 3, 4, 5, 6};
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), call_backs.visited_nodes_including_self_.begin()));
}

TEST_F(TSkimTest, CheckBasicStarTSkim) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> expected_cover = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).begin(), cover.GetSeedCover(0).end(), expected_cover.begin()) );
}

TEST_F(TSkimTest, CheckDistancePruning) {
  graph::Graph< graph::TDirectedGraph > graph;
  for (int i=0; i < 100; i++) {
    graph.AddNode(i);
  }
  for (int i=1; i < 100; i++) {
    graph.AddEdge(i -1, i);
  }
  std::vector<int> visited;
  visited.push_back(0);
  for (int distance =1; distance < 100; distance++) {
    visited.push_back(distance);
    TSkimDijkstraCallBacksDistancePrune<graph::TDirectedGraph> call_backs;
    call_backs.InitTSkimDijkstraCallBacksDistancePrune(distance + 1);
    DijkstraParams param_;
    PrunedDijkstra< graph::TDirectedGraph, TSkimDijkstraCallBacksDistancePrune<graph::TDirectedGraph> > (0,
                                                                &graph,
                                                                &call_backs,
                                                                &param_);
    EXPECT_TRUE(is_permutation(visited.begin(), visited.end(), call_backs.get_visited_nodes().begin()) );   
  }
}

TEST_F(TSkimTest, CheckBasicStarTSkim2) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  // 7 -> 6
  // 8 -> 6
  // 9 -> 6
  // 10
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddNode(7);
  graph.AddNode(8);
  graph.AddNode(9);
  graph.AddNode(10);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  graph.AddEdge(7, 6);
  graph.AddEdge(8, 6);
  graph.AddEdge(9, 6);
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 3);
  std::vector<int> expected_cover0 = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).begin(), cover.GetSeedCover(0).end(), expected_cover0.begin()) );
  std::vector<int> expected_cover6 = {6, 7, 8, 9};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(6).begin(), cover.GetSeedCover(6).end(), expected_cover6.begin()) );
  std::vector<int> expected_cover10 = {10};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(10).begin(), cover.GetSeedCover(10).end(), expected_cover10.begin()) );
}

TEST_F(TSkimTest, TSkimHighMinInfluenceValue) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  // 7 -> 6
  // 8 -> 6
  // 9 -> 6
  // 10
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddNode(7);
  graph.AddNode(8);
  graph.AddNode(9);
  graph.AddNode(10);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  graph.AddEdge(7, 6);
  graph.AddEdge(8, 6);
  graph.AddEdge(9, 6);
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 3);
  std::vector<int> expected_cover0 = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).begin(), cover.GetSeedCover(0).end(), expected_cover0.begin()) );
  std::vector<int> expected_cover6 = {6, 7, 8, 9};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(6).begin(), cover.GetSeedCover(6).end(), expected_cover6.begin()) );
  std::vector<int> expected_cover10 = {10};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(10).begin(), cover.GetSeedCover(10).end(), expected_cover10.begin()) );
}

TEST_F(TSkimTest, TSkimGreedyHighMinInfluenceValue2) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  // 7 -> 0
  // 8 -> 0
  // 9 -> 0
  // 1 -> 6
  // 2 -> 6
  // 3 -> 6
  // 4 -> 6
  // 5 -> 6
  // 7 -> 6
  // 8 -> 6
  // 9 -> 6
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddNode(7);
  graph.AddNode(8);
  graph.AddNode(9);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  graph.AddEdge(7, 0);
  graph.AddEdge(8, 0);
  graph.AddEdge(9, 0);
  graph.AddEdge(1, 6);
  graph.AddEdge(2, 6);
  graph.AddEdge(3, 6);
  graph.AddEdge(4, 6);
  graph.AddEdge(5, 6);
  graph.AddEdge(7, 6);
  graph.AddEdge(8, 6);
  graph.AddEdge(9, 6);
  Cover cover;
  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).size() == 1 || cover.GetSeedCover(0).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() == 1 || cover.GetSeedCover(6).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() != cover.GetSeedCover(0).size() );
}

TEST_F(TSkimTest, TSkimHighMinInfluenceValue2) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  // 7 -> 0
  // 8 -> 0
  // 9 -> 0
  // 1 -> 6
  // 2 -> 6
  // 3 -> 6
  // 4 -> 6
  // 5 -> 6
  // 7 -> 6
  // 8 -> 6
  // 9 -> 6
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddNode(7);
  graph.AddNode(8);
  graph.AddNode(9);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  graph.AddEdge(7, 0);
  graph.AddEdge(8, 0);
  graph.AddEdge(9, 0);
  graph.AddEdge(1, 6);
  graph.AddEdge(2, 6);
  graph.AddEdge(3, 6);
  graph.AddEdge(4, 6);
  graph.AddEdge(5, 6);
  graph.AddEdge(7, 6);
  graph.AddEdge(8, 6);
  graph.AddEdge(9, 6);
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).size() == 1 || cover.GetSeedCover(0).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() == 1 || cover.GetSeedCover(6).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() != cover.GetSeedCover(0).size() );
}

TEST_F(TSkimTest, TSkimHighMinInfluenceValue3) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Star graph
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  // 7 -> 0
  // 8 -> 0
  // 9 -> 0
  // 1 -> 6
  // 2 -> 6
  // 3 -> 6
  // 4 -> 6
  // 5 -> 6
  // 7 -> 6
  // 8 -> 6
  // 9 -> 6
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddNode(6);
  graph.AddNode(7);
  graph.AddNode(8);
  graph.AddNode(9);
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  graph.AddEdge(7, 0);
  graph.AddEdge(8, 0);
  graph.AddEdge(9, 0);
  graph.AddEdge(1, 6);
  graph.AddEdge(2, 6);
  graph.AddEdge(3, 6);
  graph.AddEdge(4, 6);
  graph.AddEdge(5, 6);
  graph.AddEdge(7, 6);
  graph.AddEdge(8, 6);
  graph.AddEdge(9, 6);
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).size() == 1 || cover.GetSeedCover(0).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() == 1 || cover.GetSeedCover(6).size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).size() != cover.GetSeedCover(0).size() );
}

TEST_F(TSkimTest, TSkimClusters) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Cluster 1 - Clique of size 3
  // 1 -> 0, 0 -> 1
  // 2 -> 0, 0 -> 2
  // 1 -> 2, 2 -> 1
  // Cluster 2 - Clique of size 3
  // 3 -> 4, 4 -> 3
  // 3 -> 5, 5 -> 3
  // 4 -> 5, 5 -> 4
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddEdge(1, 0); graph.AddEdge(0, 1);
  graph.AddEdge(2, 0); graph.AddEdge(0, 2);
  graph.AddEdge(1, 2); graph.AddEdge(2, 1);

  graph.AddEdge(3, 4); graph.AddEdge(4, 3);
  graph.AddEdge(3, 5); graph.AddEdge(5, 3);
  graph.AddEdge(4, 5); graph.AddEdge(5, 4);

  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  for (auto it = cover.Begin(); it != cover.End(); ++it) {
    EXPECT_TRUE(it->second.size() == 3);
  }
}

TEST_F(TSkimTest, TSkimGreedyClusters) {
  graph::Graph< graph::TDirectedGraph > graph;
  // Cluster 1 - Clique of size 3
  // 1 -> 0, 0 -> 1
  // 2 -> 0, 0 -> 2
  // 1 -> 2, 2 -> 1
  // Cluster 2 - Clique of size 3
  // 3 -> 4, 4 -> 3
  // 3 -> 5, 5 -> 3
  // 4 -> 5, 5 -> 4
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  graph.AddEdge(1, 0); graph.AddEdge(0, 1);
  graph.AddEdge(2, 0); graph.AddEdge(0, 2);
  graph.AddEdge(1, 2); graph.AddEdge(2, 1);

  graph.AddEdge(3, 4); graph.AddEdge(4, 3);
  graph.AddEdge(3, 5); graph.AddEdge(5, 3);
  graph.AddEdge(4, 5); graph.AddEdge(5, 4);

  Cover cover;
  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(3, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  for (auto it = cover.Begin(); it != cover.End(); ++it) {
    EXPECT_TRUE(it->second.size() == 3);
  }
}


TEST_F(TSkimTest, TSkimSize1Clusters) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);

  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 6);
}

TEST_F(TSkimTest, TSkimGreedySize1Clusters) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);

  Cover cover;
  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(3, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 6);
}

TEST_F(TSkimTest, TSkimSelective) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);

  Cover cover;
  std::vector<int> wanted = {1};
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.set_rankers_nodes(wanted);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
}

TEST_F(TSkimTest, TSkimSelective2) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  Cover cover;
  std::vector<int> wanted = {0 , 1, 5};
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.set_wanted_cover_nodes(wanted);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited = {0 , 1, 5};
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(0).begin()));
}

TEST_F(TSkimTest, TSkimLongChain) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i =0; i < num_nodes; ++i) {
    graph.AddNode(i);
  }
  for (int i =0; i < num_nodes - 1; ++i) {
    graph.AddEdge(i, i+1);
  }
  Cover cover;
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 64, 101, &cover, &graph);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    visited.push_back(i);
  }
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(99).begin()));
}

TEST_F(TSkimTest, TSkimLongChainGreedy) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i =0; i < num_nodes; ++i) {
    graph.AddNode(i);
  }
  for (int i =0; i < num_nodes - 1; ++i) {
    graph.AddEdge(i, i+1);
  }
  Cover cover;
  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(101, &cover, &graph);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    visited.push_back(i);
  }
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(99).begin()));
}

TEST_F(TSkimTest, TSkimLongChain2) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i =0; i < num_nodes; ++i) {
    graph.AddNode(i);
  }
  for (int i =0; i < num_nodes - 1; ++i) {
    graph.AddEdge(i, i+1);
  }
  Cover cover;
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    if (i % 2 == 0) {
      visited.push_back(i);
    }
  }

  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 64, 101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(98).begin()));
}


TEST_F(TSkimTest, TSkimLongChainGreedy2) {
  graph::Graph< graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i =0; i < num_nodes; ++i) {
    graph.AddNode(i);
  }
  for (int i =0; i < num_nodes - 1; ++i) {
    graph.AddEdge(i, i+1);
  }
  Cover cover;
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    if (i % 2 == 0) {
      visited.push_back(i);
    }
  }

  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  EXPECT_TRUE( std::is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(98).begin()));
}


TEST_F(TSkimTest, TSkimFacebook) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.LoadGraphFromDir(GetSampleData());
  Cover cover;
  std::vector<int> visited;
  for (int i = 0; i < graph.GetMxNId(); ++i) {
    if (i % 2 == 0) {
      if (graph.IsNode(i)) {
        visited.push_back(i);
      }
    }
  }
  TSkim< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 64, 101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_TRUE(cover.Size() > 0);

  for (auto it=cover.Begin(); it != cover.End(); it++) {
    for (auto node_id : it->second) {
      EXPECT_TRUE( node_id % 2 == 0);
    }
  }
}

TEST_F(TSkimTest, TSkimGreedyRankCheck) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  Cover cover;
  TSkimGreedy< graph::TDirectedGraph > tskim_algo;
  tskim_algo.InitTSkimGreedy(2, &cover, &graph);
  tskim_algo.Run();
  std::vector<int> covered = {0 , 1, 2, 3, 4, 5};
  EXPECT_TRUE( std::is_permutation(covered.begin(), covered.end(), cover.GetSeedCover(0).begin()));
}


TEST_F(TSkimTest, TSkimReverseRankCheck) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);
  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);
  // 1 -> 0
  // 2 -> 0
  // 3 -> 0
  // 4 -> 0
  // 5 -> 0
  graph.AddEdge(1, 0);
  graph.AddEdge(2, 0);
  graph.AddEdge(3, 0);
  graph.AddEdge(4, 0);
  graph.AddEdge(5, 0);
  Cover cover;
  TSkim< graph::TDirectedGraph > tskim_algo;
  tskim_algo.InitTSkim(2, 64, 3, &cover, &graph);
  tskim_algo.Run();
  // TSkim< graph::TDirectedGraph >(2, 64, 3, &cover, &graph);
  std::vector<int> covered = {0 , 1, 2, 3, 4, 5};
  EXPECT_TRUE( std::is_permutation(covered.begin(), covered.end(), cover.GetSeedCover(0).begin()));
}


TEST_F(TSkimTest, TSkimGreedyFacebook) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph.LoadGraphFromDir(GetSampleData());
  Cover cover;
  std::vector<int> visited;
  for (int i = 0; i < graph.GetMxNId(); ++i) {
    if (i % 2 == 0) {
      if (graph.IsNode(i)) {
        visited.push_back(i);
      }
    }
  }
  TSkimGreedy< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkimGreedy(101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  for (auto it=cover.Begin(); it != cover.End(); it++) {
    for (auto node_id : it->second) {
      EXPECT_TRUE( node_id % 2 == 0);
    }
  }
}

}  // namespace all_distance_sketch