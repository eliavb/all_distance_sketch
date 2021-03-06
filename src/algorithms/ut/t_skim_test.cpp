#include "../t_skim.h"
#include "../t_skim_reverse_rank.h"
#include "../../graph/snap_graph_adaptor.h"
#include <algorithm>

namespace all_distance_sketch {

// EXAMPLE: With a test fixture and TEST_Fs
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

/*template <class InputIterator1, class InputIterator2>
  bool is_permutation (InputIterator1 first1, InputIterator1 last1,
                       InputIterator2 first2)
{
  if (first1==last1) return true;
  InputIterator2 last2 = first2; std::advance (last2,std::distance(first1,last1));
  for (InputIterator1 it1=first1; it1!=last1; ++it1) {
    if (std::find(first1,it1,*it1)==it1) {
      auto n = std::count (first2,last2,*it1);
      if (n==0 || std::count (it1,last1,*it1)!=n) return false;
    }
  }
  return true;
}*/

TEST_F(TSkimTest, TSkimLongChain) {
  graph::Graph< graph::TDirectedGraph > graph;
  graph::Graph< graph::TDirectedGraph > graph_transpose;
  int num_nodes = 100;
  for (int i =0; i < num_nodes; ++i) {
    graph.AddNode(i);
  }
  for (int i =0; i < num_nodes - 1; ++i) {
    graph.AddEdge(i, i+1);
  }
  
  Cover cover;
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 101, 101, &cover, &graph);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    visited.push_back(i);
  }
  
  std::sort(visited.begin(), visited.end());
  auto visited_during_run = cover.GetSeedCover(99).covered_nodes;
  std::sort(visited_during_run.begin(), visited_during_run.end());
  EXPECT_TRUE( visited == visited_during_run);
}

#if 1
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
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), call_backs.visited_nodes_including_self_.begin()));

  call_backs.InitTSkimDijkstraCallBacks(3);
  graph::TDirectedGraph::TNode source2(1);
  PrunedDijkstra< graph::TDirectedGraph,
                  TSkimDijkstraCallBacks< graph::TDirectedGraph > > (source2,
                                                                      &graph,
                                                                      &call_backs,
                                                                      &param);
  EXPECT_EQ(call_backs.visited_nodes_including_self_.size(), 3);
  std::vector<int> visited2 = {1, 3, 4};
  EXPECT_TRUE( is_permutation(visited2.begin(), visited2.end(), call_backs.visited_nodes_including_self_.begin()));

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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> expected_cover = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).covered_nodes.begin(), cover.GetSeedCover(0).covered_nodes.end(), expected_cover.begin()) );
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
    /*
    for (const auto& node : visited) {
      std::cout << node << std::endl;
    }   
    std::cout << "====" << std::endl;*/
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 3);
  std::vector<int> expected_cover0 = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).covered_nodes.begin(), cover.GetSeedCover(0).covered_nodes.end(), expected_cover0.begin()) );
  std::vector<int> expected_cover6 = {6, 7, 8, 9};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(6).covered_nodes.begin(), cover.GetSeedCover(6).covered_nodes.end(), expected_cover6.begin()) );
  std::vector<int> expected_cover10 = {10};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(10).covered_nodes.begin(), cover.GetSeedCover(10).covered_nodes.end(), expected_cover10.begin()) );
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 3);
  std::vector<int> expected_cover0 = {0, 1, 2, 3, 4, 5};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(0).covered_nodes.begin(), cover.GetSeedCover(0).covered_nodes.end(), expected_cover0.begin()) );
  std::vector<int> expected_cover6 = {6, 7, 8, 9};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(6).covered_nodes.begin(), cover.GetSeedCover(6).covered_nodes.end(), expected_cover6.begin()) );
  std::vector<int> expected_cover10 = {10};
  EXPECT_TRUE(is_permutation(cover.GetSeedCover(10).covered_nodes.begin(), cover.GetSeedCover(10).covered_nodes.end(), expected_cover10.begin()) );
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(100, 100, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).covered_nodes.size() == 1 || cover.GetSeedCover(0).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() == 1 || cover.GetSeedCover(6).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() != cover.GetSeedCover(0).covered_nodes.size() );
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).covered_nodes.size() == 1 || cover.GetSeedCover(0).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() == 1 || cover.GetSeedCover(6).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() != cover.GetSeedCover(0).covered_nodes.size() );
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 2, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  EXPECT_TRUE( cover.GetSeedCover(0).covered_nodes.size() == 1 || cover.GetSeedCover(0).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() == 1 || cover.GetSeedCover(6).covered_nodes.size() == 9 );
  EXPECT_TRUE( cover.GetSeedCover(6).covered_nodes.size() != cover.GetSeedCover(0).covered_nodes.size() );
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  for (auto it = cover.Begin(); it != cover.End(); ++it) {
    EXPECT_TRUE(it->second.covered_nodes.size() == 3);
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(3, 100, 100, &cover, &graph);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 2);
  for (auto it = cover.Begin(); it != cover.End(); ++it) {
    EXPECT_TRUE(it->second.covered_nodes.size() == 3);
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(3, 100, 100, &cover, &graph);
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.set_rankers_nodes(wanted);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
}

#if 1
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(10, 64, 3, &cover, &graph);
  t_skim_algo.set_wanted_cover_nodes(wanted);
  t_skim_algo.Run();
  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited = {0 , 1, 5};
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(0).covered_nodes.begin()));
}
#endif


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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 100, 100, &cover, &graph);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  std::vector<int> visited;
  for (int i = 0; i < num_nodes; ++i) {
    visited.push_back(i);
  }
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(99).covered_nodes.begin()));
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

  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 101, 101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(98).covered_nodes.begin()));
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

  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 100, 100, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_EQ(cover.Size(), 1);
  EXPECT_TRUE( is_permutation(visited.begin(), visited.end(), cover.GetSeedCover(98).covered_nodes.begin()));
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
  TSkimReverseRank< graph::TDirectedGraph > tskim_algo;
  tskim_algo.InitTSkim(2, 100, 100, &cover, &graph);
  tskim_algo.Run();
  std::vector<int> covered = {0 , 1, 2, 3, 4, 5};
  EXPECT_TRUE( is_permutation(covered.begin(), covered.end(), cover.GetSeedCover(0).covered_nodes.begin()));
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
  TSkimReverseRank< graph::TDirectedGraph > tskim_algo;
  tskim_algo.InitTSkim(2, 64, 3, &cover, &graph);
  tskim_algo.Run();
  // TSkimReverseRank< graph::TDirectedGraph >(2, 64, 3, &cover, &graph);
  std::vector<int> covered = {0 , 1, 2, 3, 4, 5};
  EXPECT_TRUE( is_permutation(covered.begin(), covered.end(), cover.GetSeedCover(0).covered_nodes.begin()));
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 64, 101, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  EXPECT_TRUE(cover.Size() > 0);

  for (auto it=cover.Begin(); it != cover.End(); it++) {
    for (int i=0;i<it->second.covered_nodes.size();i++) {
      int node_id = it->second.covered_nodes[i];
      EXPECT_TRUE( node_id % 2 == 0);
    }
  }
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
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo;
  t_skim_algo.InitTSkim(101, 100, 100, &cover, &graph);
  t_skim_algo.set_rankees_nodes(visited);
  t_skim_algo.set_wanted_cover_nodes(visited);
  t_skim_algo.Run();

  for (auto it=cover.Begin(); it != cover.End(); it++) {
    for (int i=0; i < it->second.covered_nodes.size(); i++) {
      int node_id = it->second.covered_nodes[i];
      EXPECT_TRUE( node_id % 2 == 0);
    }
  }
}
#endif
}  // namespace all_distance_sketch