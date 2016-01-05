#include <limits.h>
#include "gtest/gtest.h"
#include "../../algorithms/dijkstra_shortest_paths.h"
#include "../../algorithms/reverse_rank.h"
#include "../../algorithms/sketch_calculation.h"
#include "../estimator.h"
#include "../../graph/snap_graph_adaptor.h"


class BasicGraph : public ::testing::Test {
 protected:
  BasicGraph() {
    
  }

  static void SetUpTestCase() {    
  }

  static void TearDownTestCase() {
    // myfile.close();
  }

  virtual ~BasicGraph() { 
    
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }
};

namespace all_distance_sketch {

long long COMUUNITY_INDEX = 114;

TEST_F(BasicGraph, BasicMinClassificationTest) {
  utils::SingleCommunity singleCom;
  singleCom.push_back(0);
  singleCom.push_back(1);
  singleCom.push_back(3);
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);

  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);

  int numSamples = 3;
  PriorVector prior;
  prior.resize(numSamples);
  prior[0].nodeId = 0;
  prior[0].order.push_back(0);
  prior[0].order.push_back(0);
  prior[0].order.push_back(0);
  prior[0].order.push_back(1);
  prior[0].order.push_back(2);
  prior[0].order.push_back(3);

  prior[1].nodeId = 1;
  prior[1].order.push_back(0);
  prior[1].order.push_back(0);
  prior[1].order.push_back(0);
  prior[1].order.push_back(10);
  prior[1].order.push_back(20);
  prior[1].order.push_back(30);

  prior[2].nodeId = 2;
  prior[2].order.push_back(0);
  prior[2].order.push_back(0);
  prior[2].order.push_back(0);
  prior[2].order.push_back(100);
  prior[2].order.push_back(200);
  prior[2].order.push_back(300);

  int threshold = 1;
  BinaryMinClassifier minDistanceClassifier;
  minDistanceClassifier.InitBinaryClassifier(&prior, threshold);
  
  ClassifierAggregator< all_distance_sketch::graph::TUnDirectedGraph > aggregator;
  aggregator.InitClassifierAggregator(&minDistanceClassifier, &singleCom, &graph);
  
  EXPECT_EQ(minDistanceClassifier.GetRank(0), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(1), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(2), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(3), 1);
  EXPECT_EQ(minDistanceClassifier.GetRank(4), 2);
  EXPECT_EQ(minDistanceClassifier.GetRank(5), 3);

}

TEST_F(BasicGraph, BasicAverageClassificationTest) {
  utils::SingleCommunity singleCom;
  singleCom.push_back(0);
  singleCom.push_back(1);
  singleCom.push_back(3);
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.AddNode(0);
  graph.AddNode(1);
  graph.AddNode(2);

  graph.AddNode(3);
  graph.AddNode(4);
  graph.AddNode(5);

  int numSamples = 3;
  PriorVector prior;
  prior.resize(numSamples);
  prior[0].nodeId = 0;
  prior[0].order.push_back(0);
  prior[0].order.push_back(0);
  prior[0].order.push_back(0);
  prior[0].order.push_back(1);
  prior[0].order.push_back(2);
  prior[0].order.push_back(3);

  prior[1].nodeId = 1;
  prior[1].order.push_back(0);
  prior[1].order.push_back(0);
  prior[1].order.push_back(0);
  prior[1].order.push_back(10);
  prior[1].order.push_back(20);
  prior[1].order.push_back(30);

  prior[2].nodeId = 2;
  prior[2].order.push_back(0);
  prior[2].order.push_back(0);
  prior[2].order.push_back(0);
  prior[2].order.push_back(100);
  prior[2].order.push_back(200);
  prior[2].order.push_back(300);

  int threshold = 1;
  BinaryAverageClassifier minDistanceClassifier;
  minDistanceClassifier.InitBinaryClassifier(&prior, threshold);
  
  ClassifierAggregator< all_distance_sketch::graph::TUnDirectedGraph > aggregator;
  aggregator.InitClassifierAggregator(&minDistanceClassifier, &singleCom, &graph);
  
  EXPECT_EQ(minDistanceClassifier.GetRank(0), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(1), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(2), 0);
  EXPECT_EQ(minDistanceClassifier.GetRank(3), (1 + 10 + 100) / 3);
  EXPECT_EQ(minDistanceClassifier.GetRank(4), (2 + 20 + 200) / 3 );
  EXPECT_EQ(minDistanceClassifier.GetRank(5), (3 + 30 + 300) / 3 );
}

void CreateRocCurve(std::string aBasePath,
                    std::string metric,
                    PriorVector * prior,
                    utils::SingleCommunity * singleCom,
                    all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph >  * graph) {
  int threshold = 1;
  BinaryMinClassifier minDistanceClassifier;
  minDistanceClassifier.InitBinaryClassifier(prior, threshold);

  ClassifierAggregator< all_distance_sketch::graph::TUnDirectedGraph > aggregator;
  aggregator.InitClassifierAggregator(&minDistanceClassifier, singleCom, graph);
  std::ofstream f;
  f.open(aBasePath + "/min_" + metric + "_roc_curve_" + std::to_string(COMUUNITY_INDEX) + ".csv");
  aggregator.PrintROCCurve(&f); 
  f.close();

  BinaryAverageClassifier averageDistanceClassifier;
  averageDistanceClassifier.InitBinaryClassifier(prior, threshold);

  aggregator.InitClassifierAggregator(&averageDistanceClassifier, singleCom, graph);
  std::ofstream f_a;
  f_a.open(aBasePath + "/average_" + metric + "_roc_curve_" + std::to_string(COMUUNITY_INDEX) + ".csv");
  aggregator.PrintROCCurve(&f_a); 
  f.close();

  BinaryHarmonicMeanClassifier harmonicDistanceClassifier;
  harmonicDistanceClassifier.InitBinaryClassifier(prior, threshold);

  aggregator.InitClassifierAggregator(&harmonicDistanceClassifier, singleCom, graph);
  std::ofstream f_h;
  f_h.open(aBasePath + "/harmonic_" + metric + "_roc_curve_"+ std::to_string(COMUUNITY_INDEX) +".csv");
  aggregator.PrintROCCurve(&f_h); 
  f_h.close();

}
#if 1
void CreateEdithReport() {
  // nodeID    for each seed s1..s_10:  (distance, rank, reverse rank)      member of community (y/n)   #of_communities_node_is_member_of
  std::cout << "NodeId,SeedId,Metric,NumCommunitiesMember" << std::endl;
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  
  utils::Community com;
  com.LoadCommunity("./data/youtube_com");

  GraphSketch graphAds;
  graphAds.InitGraphSketch(128, graph.GetMxNId());
  all_distance_sketch::CalculateGraphSketch< all_distance_sketch::graph::TUnDirectedGraph >(&graph, &graphAds);
  graphAds.CalculateAllDistanceNeighborhood();
  // utils::SingleCommunity * singleCom = com.GetCommunity(COMUUNITY_INDEX);
  
  std::vector<int> _com;
  _com.push_back(33788);
  _com.push_back(68028);
  _com.push_back(2711);
  _com.push_back(20328);
  _com.push_back(7682);
  _com.push_back(255);
  _com.push_back(665257);
  _com.push_back(480);
  _com.push_back(3394);
  _com.push_back(368749);
  std::vector<int> * singleCom = &_com;
  
  

  std::vector<int> dist_comm;
  dist_comm.resize(graph.GetMxNId(), 0);
  com.GetNodesDist(&dist_comm);

  unsigned int numSamples=10;
  DijkstraParams p[10];
  DijkstraRankCallBack< graph::TUnDirectedGraph > dijkstra_rank_call_backs[10];
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    PrunedDijkstra< graph::TUnDirectedGraph, DijkstraRankCallBack< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &dijkstra_rank_call_backs[i],
                                                                                      &p[i]);
  }  

  for (all_distance_sketch::graph::TUnDirectedGraph::TNodeI it = graph.BegNI(); it != graph.EndNI(); it++) {
      int nodeId = it.GetId();
      for (unsigned int i=0; i < numSamples; i++ ){
        std::cout << nodeId << "," << (*singleCom)[i] << "," << p[i].min_distance[nodeId] << ",distace," << dist_comm[nodeId] << std::endl;
      }
  }

  for (all_distance_sketch::graph::TUnDirectedGraph::TNodeI it = graph.BegNI(); it != graph.EndNI(); it++) {
      int nodeId = it.GetId();
      for (unsigned int i=0; i < numSamples; i++ ){
        std::cout << nodeId << "," <<
                    (*singleCom)[i] << "," <<
                    dijkstra_rank_call_backs[i].get_dijkstra_rank()[nodeId] <<
                    ",DijkstraRank," <<
                    dist_comm[nodeId] << std::endl;
      }
  }

  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    std::vector<int> ranking;
    DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
    CalculateReverseRank< all_distance_sketch::graph::TUnDirectedGraph >((*singleCom)[i], &graph, &graphAds, &ranking, &reverse_rank_call_backs);
    for (unsigned int k=0; k < ranking.size(); k++) {
      if (graph.IsNode(k) == false) {
        continue;
      }
      if ((int)k == (*singleCom)[i]) {
        continue;
      }
      std::cout << k << "," << (*singleCom)[i] << "," << ranking[k] << ",ReverseDijkstraRank," << dist_comm[k] << std::endl;
    }
  }

}

TEST_F(BasicGraph, EdithReport) {
  CreateEdithReport();
}

TEST_F(BasicGraph, LiveJournalROCCurve) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  graph.LoadGraphFromDir("./data/live_journal");
  utils::Community com;
  com.LoadCommunity("./data/live_journal_comm");
  unsigned int numSamples = 10;
  long long commIndex = 4883;
  utils::SingleCommunity * com1 = com.GetCommunity(commIndex);
  std::cout << "Community size=" << com1->size() << std::endl;

  DijkstraParams p[10];
  DijkstraRankCallBack< graph::TUnDirectedGraph > dijkstra_rank_call_backs[10];
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*com1)[i]);
    PrunedDijkstra< graph::TUnDirectedGraph, DijkstraRankCallBack< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &dijkstra_rank_call_backs[i],
                                                                                      &p[i]);
  }

  PriorVector prior;
  prior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < p[i].min_distance.size(); k++) {
      prior[i].order.push_back(p[i].min_distance[k]);
    }
  }


  CreateRocCurve("./out/graph/results/roc_curves", "_live_distance_" +std::to_string(commIndex) + "_comm", &prior, com1, &graph);
  
  PriorVector RNNprior;
  RNNprior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*com1)[i]);
    std::vector<int> ranking;
    DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
    CalculateReverseRank< all_distance_sketch::graph::TUnDirectedGraph >((*com1)[i], &graph, &graphAds, &ranking, &reverse_rank_call_backs);
    RNNprior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < ranking.size(); k++) {
      RNNprior[i].order.push_back(ranking[k]);
    }
  }

  CreateRocCurve("./out/graph/results/roc_curves", "rnn_live_"+std::to_string(commIndex)+"_comm", &RNNprior, com1, &graph);
  
  PriorVector dijkprior;
  dijkprior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < dijkstra_rank_call_backs[i].get_dijkstra_rank().size(); k++) {
      dijkprior[i].order.push_back(dijkstra_rank_call_backs[i].get_dijkstra_rank()[k]);
    }
  }

  CreateRocCurve("./out/graph/results/roc_curves", "dijkstra_live_"+std::to_string(commIndex)+"_comm", &dijkprior, com1, &graph);

}


TEST_F(BasicGraph, ROCCurveRNN) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  GraphSketch graphAds;
  graphAds.InitGraphSketch(128, graph.GetMxNId());
  all_distance_sketch::CalculateGraphSketch< all_distance_sketch::graph::TUnDirectedGraph >(&graph, &graphAds);
  graphAds.CalculateAllDistanceNeighborhood();


  PriorVector RNNprior;
  unsigned int numSamples = 10;
  RNNprior.resize(numSamples);

  utils::Community com;
  com.LoadCommunity("./data/youtube_com");

  utils::SingleCommunity * singleCom = com.GetCommunity(COMUUNITY_INDEX);
  std::cout << " community size=" << singleCom->size() << std::endl;

  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    std::vector<int> ranking;
    DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
    CalculateReverseRank< all_distance_sketch::graph::TUnDirectedGraph >((*singleCom)[i], &graph, &graphAds, &ranking, &reverse_rank_call_backs);
    RNNprior[i].nodeId = (*singleCom)[i];
    for (unsigned int k=0; k < ranking.size(); k++) {
      RNNprior[i].order.push_back(ranking[k]);
    }
  }

  CreateRocCurve("out/all_distance_sketch/experiments/results/RNN/", "rnn", &RNNprior, singleCom, &graph);

}

TEST_F(BasicGraph, ROCCurveDistance) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  utils::Community com;
  com.LoadCommunity("./data/youtube_com");
  unsigned int numSamples = 10;
  utils::SingleCommunity * singleCom = com.GetCommunity(COMUUNITY_INDEX);
  std::cout << " community size=" << singleCom->size() << std::endl;
  /*
  Distance
  */

  DijkstraParams p[10];
  DijkstraRankCallBack< graph::TUnDirectedGraph > dijkstra_rank_call_backs[10];
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    PrunedDijkstra< graph::TUnDirectedGraph, DijkstraRankCallBack< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &dijkstra_rank_call_backs[i],
                                                                                      &p[i]);
  }

  PriorVector prior;
  prior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*singleCom)[i];
    for (unsigned int k=0; k < p[i].min_distance.size(); k++) {
      prior[i].order.push_back(p[i].min_distance[k]);
    }
  }

  CreateRocCurve("./out/all_distance_sketch/experiments/", "distance", &prior, singleCom, &graph);
}


TEST_F(BasicGraph, ROCCurveCrossCommunity) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  GraphSketch graphAds;
  graph.LoadGraphFromDir("./data/youtube");
  utils::Community com;
  com.LoadCommunity("./data/youtube_comm");
  unsigned int numSamples = 10;
  
  long long community1 = 7;
  long long community2 = 313;
  /*
  com.GetCommunitiesWithIntersection(&community1, &community2, 50);
  std::cout << " community1="<< community1 << " community2=" << community2 << std::endl;
  return;
  */
  graph.LoadGraphFromDir("./data/youtube");
  
  
  utils::SingleCommunity * com1 = com.GetCommunity(community1);
  utils::SingleCommunity * com2 = com.GetCommunity(community2);
  std::cout << " community1 size=" << com1->size() << std::endl;
  std::cout << " community2 size=" << com2->size() << std::endl;
  /*
  Distance
  */

  DijkstraParams p[10];
  DijkstraRankCallBack< graph::TUnDirectedGraph > dijkstra_rank_call_backs[10];
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*com1)[i]);
    PrunedDijkstra< graph::TUnDirectedGraph, DijkstraRankCallBack< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &dijkstra_rank_call_backs[i],
                                                                                      &p[i]);
  }

  PriorVector prior;
  prior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < p[i].min_distance.size(); k++) {
      prior[i].order.push_back(p[i].min_distance[k]);
    }
  }

  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph2;
  for (unsigned int i=0 ; i < com2->size(); i++) {
    graph2.AddNode((*com2)[i]);
  }

  for (unsigned int i=0; i < com1->size(); i++) {
    if (graph2.IsNode((*com1)[i])) {
      std::cout << " Dup Node" << std::endl;
      continue;
    }
    graph2.AddNode((*com1)[i]);
  }

  CreateRocCurve("./out/graph/results/roc_curves", "distance_" +std::to_string(community1) +"_"+std::to_string(community2)+ "_comm", &prior, com1, &graph2);

  PriorVector RNNprior;
  RNNprior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*com1)[i]);
    std::vector<int> ranking;
    DefaultReverseRankCallBacks< graph::TUnDirectedGraph > reverse_rank_call_backs;
    CalculateReverseRank< all_distance_sketch::graph::TUnDirectedGraph >((*com1)[i], &graph, &graphAds, &ranking, &reverse_rank_call_backs);
    RNNprior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < ranking.size(); k++) {
      RNNprior[i].order.push_back(ranking[k]);
    }
  }

  CreateRocCurve("./out/graph/results/roc_curves", "rnn_"+std::to_string(community1) +"_"+std::to_string(community2)+"_comm", &RNNprior, com1, &graph2);

  PriorVector dijkprior;
  dijkprior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*com1)[i];
    for (unsigned int k=0; k < dijkstra_rank_call_backs[i].get_dijkstra_rank().size(); k++) {
      dijkprior[i].order.push_back(dijkstra_rank_call_backs[i].get_dijkstra_rank()[k]);
    }
  }

  CreateRocCurve("./out/graph/results/roc_curves", "dijkstra_"+std::to_string(community1) +"_"+std::to_string(community2)+"_comm", &dijkprior, com1, &graph2);

}



TEST_F(BasicGraph, ROCCurveDjkstraRank) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("./data/youtube");
  utils::Community com;
  com.LoadCommunity("./data/youtube_comm");
  unsigned int numSamples = 10;
  utils::SingleCommunity * singleCom = com.GetCommunity(COMUUNITY_INDEX);
  std::cout << " community size=" << singleCom->size() << std::endl;
  /*
  Distance
  */

  DijkstraParams p[10];
  DijkstraRankCallBack< graph::TUnDirectedGraph > dijkstra_rank_call_backs[10];
  for (unsigned int i=0; i < numSamples; i++) {
    all_distance_sketch::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    PrunedDijkstra< graph::TUnDirectedGraph, DijkstraRankCallBack< graph::TUnDirectedGraph > >
                                                                                     (source,
                                                                                      &graph,
                                                                                      &dijkstra_rank_call_backs[i],
                                                                                      &p[i]);
  }

  PriorVector prior;
  prior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*singleCom)[i];
    for (unsigned int k=0; k < dijkstra_rank_call_backs[i].get_dijkstra_rank().size(); k++) {
      prior[i].order.push_back(dijkstra_rank_call_backs[i].get_dijkstra_rank()[k]);
    }
  }

  CreateRocCurve("./out/graph/results/roc_curves", "dijkstra", &prior, singleCom, &graph);
}


#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
