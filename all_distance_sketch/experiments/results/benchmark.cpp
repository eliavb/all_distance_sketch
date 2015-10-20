#include <limits.h>
#include <string>
#include "../../gtest/include/gtest/gtest.h"
#include "../algo.h"
#include "../estimator.h"
#include "../snap_adaptor.h"

static std::ofstream myfile;

typedef std::vector<int>  RandomNodes;

namespace a {
  class DataSetDetails {
  public:
    bool isDirected;
    std::string dirPath;
    RandomNodes randomNodes;
    unsigned int numSamples;
    std::string name;

    void InitDataSetDetails(bool aIsDirected, std::string aDirPath, unsigned int aNumSamples, std::string aName) {
      isDirected = aIsDirected;
      dirPath = aDirPath;
      numSamples = aNumSamples;
      name = aName;
    }
  };
}


template<class T>
RandomNodes * GetRandomNodes(a::DataSetDetails * dataSet, a::graph::Graph< T > * aGraph) {
  if (dataSet->randomNodes.size() == dataSet->numSamples) {
    return &dataSet->randomNodes;
  }
  srand(123);
  while (dataSet->randomNodes.size() < dataSet->numSamples) {
    int nodeId = rand() % (aGraph->GetMxNId());
    if (aGraph->IsNode(nodeId)) {
      bool found = false;
      for (unsigned int i=0; i < dataSet->randomNodes.size(); i++) {
        if (nodeId == dataSet->randomNodes[i]) {
          found = true;
          break;
        }
      }
      if (found == false) {
        dataSet->randomNodes.push_back(nodeId);
      }
    }
  }
  return &dataSet->randomNodes;
}

static std::vector<a::DataSetDetails > dataSetDetails;
static std::vector<int> KValues;
class BasicGraph : public ::testing::Test {
 protected:
  BasicGraph() {
    
  }

  static void SetUpTestCase() {

    int numSamples = 1000;
    dataSetDetails.resize(5);
    
    dataSetDetails[0].InitDataSetDetails(false, "../../data/facebook", numSamples, "facebook");
    dataSetDetails[1].InitDataSetDetails(true, "../../data/slashdot", numSamples, "slashdot");
    dataSetDetails[2].InitDataSetDetails(true, "../../data/tweeter", numSamples, "tweeter");
    dataSetDetails[3].InitDataSetDetails(false, "../../data/youtube", numSamples, "youtube");
    dataSetDetails[4].InitDataSetDetails(false, "../../data/live_journal", numSamples, "live_journal");
    
    // dataSetDetails[0].InitDataSetDetails(false, "../../data/live_journal", numSamples);
    
    
    
    KValues.push_back(16);
    KValues.push_back(64);
    KValues.push_back(128);
    
  }

  static void TearDownTestCase() {
    // myfile.close();
  }

  virtual ~BasicGraph() { 
    
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    mySampleData = "/users/eng/eliavb/TAU/RNN/data/facebook";
    
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

long long GetCurrentTimeMilliSec() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (long long) (( tp.tv_sec * 1000L ) + tp.tv_usec / 1000);
}

#if 0
template <class T>
void SearlizeADSCalculation(std::string aDirPath, unsigned int k, std::string fileName) {
  a::graph::Graph< T > graph;
  a::data::GraphADS graphAds;
  graph.LoadGraphFromDir(aDirPath);
  graphAds.initGraphADS(k, graph.GetMxNId());
  graphAds.CreateNodesDistribution(graph.GetMxNId());
  a::algo::ADSStats stats;  
  a::algo::CalculateGraphADS<T>(&graph, &graphAds);
  graphAds.CalculateAllDistanceNeighborhood();
  std::ofstream ofs(fileName);
  {
    boost::archive::text_oarchive oa(ofs);
    oa << graphAds;
  }

}


template <class T>
void LoadGraph(a::graph::Graph< T > * graph, a::data::GraphADS * aGraphAds, std::string aDirPath, int k, std::string fileName) {
  graph->LoadGraphFromDir(aDirPath);
  aGraphAds->initGraphADS(k, graph->GetMxNId());
  {
    std::ifstream ifs(fileName);
    boost::archive::text_iarchive ia(ifs);
    ia >> (*aGraphAds);
  }

}
#endif 

template <class T>
void BanchMarkDataSetADSCalculation(std::string aDirPath, int k, std::ostream * f = (&std::cout),  a::DataSetDetails * aDetails = NULL) {
  std::cout << " k= " << k << std::endl;
  (*f)  << aDirPath << "," << k << ",";
  /*
  Part 1 build the graph
  */
  long long mslongBefore = GetCurrentTimeMilliSec();
  
  a::graph::Graph< T > graph;
  a::data::GraphADS graphAds;
  graph.LoadGraphFromDir(aDirPath);
  graphAds.initGraphADS(k, graph.GetMxNId());
  graphAds.CreateNodesDistribution(graph.GetMxNId());
  a::algo::ADSStats stats;
  
  long long mslongAfter = GetCurrentTimeMilliSec();
  long long graphTime = mslongAfter - mslongBefore;
  (*f) <<  graphTime << ", " ;
  (*f) << graph.GetNumNodes() << ", " << graph.GetNumEdges() << ", " ;
  

  /*
  Part 2 compute the ADS
  */
  mslongBefore = GetCurrentTimeMilliSec();
  // int numVMUsed;
  // int numRSSUsed;
  a::algo::RunTimeResourcesStats resources;
  a::algo::CalculateGraphADS<T>(&graph, &graphAds, &stats, &resources);

  mslongAfter = GetCurrentTimeMilliSec();
  
  (*f) << "ADS=" << mslongAfter - mslongBefore << ",";
  // (*f) << numVMUsed << "," << numRSSUsed  << ",";
  /*
  Part 3 compute all the distances
  */
  mslongBefore = GetCurrentTimeMilliSec();
  graphAds.CalculateAllDistanceNeighborhood();
  mslongAfter = GetCurrentTimeMilliSec();
  (*f) << mslongAfter - mslongBefore << std::endl;

}

template <class T>
void ComputeRNNSamples(a::graph::Graph< T > * graph, a::data::GraphADS * aGraphAds, std::string aDirPath, int k, a::DataSetDetails * aDetails, std::ostream * f = (&std::cout)) {
  (*f)  << aDirPath << "," << k << ",";
  long long mslongBefore, mslongAfter;
  unsigned int numSamples = 0;
  mslongBefore = GetCurrentTimeMilliSec();
  std::vector<int> ranking;
  RandomNodes * randomNodes = GetRandomNodes< T > (aDetails, graph);
  for (unsigned int z=0; z < randomNodes->size(); z++) {
    int node = (*randomNodes)[z];
    if (graph->IsNode(node) == false) {
        continue;
    }
    numSamples += 1;
    a::algo::ComputeRNN<T>(node, graph, aGraphAds, &ranking);
  }  
  
  
  mslongAfter = GetCurrentTimeMilliSec();
  double timePerRequest = 0;
  timePerRequest = (double )(mslongAfter - mslongBefore) / numSamples;  
  
  (*f) << mslongAfter - mslongBefore << ","  << timePerRequest << std::endl;
}

template <class T>
void BanchMarkDataSetDijkstraUpToK(std::string aDirPath, int whenToStop, bool shouldUseRandomNodes = false, a::DataSetDetails * aDetails = NULL) {
  long long mslongBefore = GetCurrentTimeMilliSec();
  
  a::graph::Graph< T > graph;
  a::data::GraphADS graphAds;
  graph.LoadGraphFromDir(aDirPath);
  
  long long mslongAfter = GetCurrentTimeMilliSec();
  /*
  Part 2 compute the ADS
  */
  std::cout << "finished loading the graph " << std::endl;
  mslongBefore = GetCurrentTimeMilliSec();
  a::algo::DijkstraParams p;
  unsigned int numNodesRan = 0;
  RandomNodes * randomNodes = NULL;
  if (aDetails != NULL) {
    randomNodes = GetRandomNodes<T>(aDetails, &graph);
  }

  for (typename T::TNodeI it = graph.BegNI(); it != graph.EndNI(); it++) {
    int nodeId = it.GetId();
    if (shouldUseRandomNodes) {
      nodeId = (*randomNodes)[numNodesRan];
    }
    typename T::TNode source(nodeId);
    p.InitDijkstraParams(false);
    a::algo::CalculateNodeAdsPrunedDijkstra< T >(source, &graph, &p, graph.GetMxNId(), NULL, NULL, whenToStop);  
    numNodesRan++;
    if (shouldUseRandomNodes && numNodesRan == randomNodes->size()) {
      break;
    }
  }
  mslongAfter = GetCurrentTimeMilliSec();
  std::cout << " dataset=" << aDirPath <<
               " num nodes= " << numNodesRan <<
               " stop after =" << whenToStop <<
               " time average=" << (mslongAfter - mslongBefore) / (double)numNodesRan <<
               " num nodes=" << numNodesRan << 
               " Total time=" << (mslongAfter - mslongBefore) << std::endl;
}


template <class T>
void BanchMarkDataSetMultiADSCalculation(std::string aDirPath, int k, int aNumThreads, std::ostream * f = (&std::cout), double factor = 1.1) {
  (*f)  << aDirPath << "," << k << ",";
  /*
  Part 1 build the graph
  */
  long long mslongBefore = GetCurrentTimeMilliSec();
  
  a::graph::Graph< T > graph;
  a::data::GraphADS graphAds;
  graph.LoadGraphFromDir(aDirPath);
  graphAds.initGraphADS(k, graph.GetMxNId());
  graphAds.CreateNodesDistribution(graph.GetMxNId());
  a::algo::ADSStats stats;
  
  long long mslongAfter = GetCurrentTimeMilliSec();
  long long graphTime = mslongAfter - mslongBefore;
  (*f) <<  graphTime ;
  (*f) << "," << graph.GetNumNodes() << "," <<  graph.GetNumEdges() << ",";
  /*
  Part 2 compute the ADS
  */
  mslongBefore = GetCurrentTimeMilliSec();
  a::algo::RunTimeResourcesStats resources;
  a::algo::CalculateGraphADSMulti< T >(&graph, &graphAds, aNumThreads, factor, NULL, &resources);

  mslongAfter = GetCurrentTimeMilliSec();
  
  (*f) << mslongAfter - mslongBefore << ",";
  (*f) << aNumThreads << "," << resources.numVMUsed << "," << resources.numRSSUsed  << "," <<  factor << std::endl;
  
}

template<class T>
void ADSCalculationApproximationCheck(std::string aDirPath,
                                      int k,
                                      bool directed,
                                      a::DataSetDetails * aDetails,
                                      std::ostream * f = (&std::cout)) {
  std::vector<int> histo;
  histo.resize(101, 0);
  int numCompared = 0;
  bool transpose = directed == true;
  
  a::graph::Graph< T > graphTranspose;
  a::data::GraphADS graphAds;
  graphTranspose.LoadGraphFromDir(aDirPath, transpose);
  graphAds.initGraphADS(k, graphTranspose.GetMxNId());
  graphAds.CreateNodesDistribution(graphTranspose.GetMxNId());
  a::algo::CalculateGraphADS<T>(&graphTranspose, &graphAds);
  graphAds.CalculateAllDistanceNeighborhood();

  
  a::graph::Graph< T > graph;
  graph.LoadGraphFromDir(aDirPath);
  
  int d = 0;
  int numSampled = 0;
  RandomNodes * randomNodes = NULL;
  if (aDetails) {
    randomNodes = GetRandomNodes< T >(aDetails, &graph);
  }
  for (unsigned z=0; z < randomNodes->size(); z++) {
    numSampled++;
    int nodeId = (*randomNodes)[z];
    a::algo::DijkstraParams param;
    param.InitDijkstraParams(false);
    typename T::TNode source(nodeId);
    a::algo::CalculateNodeAdsPrunedDijkstra< T >( source,
                                          &graph,
                                          &param,
                                          graph.GetMxNId());
    
    a::graph::edge_weight_t max_distance = 0;
    for (unsigned int i =0 ; i < param.min_distance.size(); i++){
      if (param.min_distance[i] != a::Constants::UNREACHABLE){
        max_distance = max_distance > param.min_distance[i] ? max_distance : param.min_distance[i];
      }
    }

    a::data::NodeIdAdsData sourceDetails(nodeId, graphAds.GetNodeRandomId(nodeId));
    a::data::ADS * nodeAds = graphAds.GetNodeADS(sourceDetails);

    for (unsigned int i = 0; i < max_distance; i++){
      int num_neighbors = 0;
      for (unsigned int j=0; j < param.min_distance.size(); j++){
        if (param.min_distance[j] <= i && j != (unsigned int)nodeId){
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
      
      int a = (num_neighbors - num_neighbors_approximate) > 0 ? (num_neighbors - num_neighbors_approximate) : (-num_neighbors + num_neighbors_approximate);
      d = d > a ? d : a;
      
    }
    
  }
  (*f) << "Data= " << aDirPath << " k =" << k << std::endl;
  for (unsigned int i=0; i < 101; i++) {
      if (histo[i] == 0) {
        continue;
      }
      (*f) << " num errors of ratio=" << 0.01 * i << " is " << histo[i] << std::endl;
    }
  
}

TEST_F(BasicGraph, BenchMarkAll) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long long timeStamp = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000;
  std::string fileName = "./out/graph/results/result_" + std::to_string(timeStamp) + ".csv";
  std::ofstream f;
  f.open(fileName);
  f << "dataset, K, timeToLoadGraphMilliSec, numNodes, numEdges, timeToCalculateADSMilliSec, VMUsedKB, RSSUsedKB, timeToCalculateDistancesMilliSec, timePerRNNComputationMilliSec" << std::endl;
  int K[] = {16, 64, 128};
  for (volatile unsigned int i=0; i < 3; i++) {
      int k = K[i];
      for (volatile unsigned int j=0; j < dataSetDetails.size(); j++) {
        std::cout << dataSetDetails[j].dirPath << std::endl;
        if (dataSetDetails[j].isDirected) {
          ::BanchMarkDataSetADSCalculation< a::graph::TDirectedGraph >( dataSetDetails[j].dirPath,
                                                                        k,
                                                                        &f,
                                                                        &dataSetDetails[j]);
        } else {
          ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >( dataSetDetails[j].dirPath,
                                                                          k,
                                                                          &f,
                                                                          &dataSetDetails[j]);
        }

      }
  }
  f.close();
}


TEST_F(BasicGraph, BenchMarkExactAll) {
  int exact[] = {16, 64, 128 };
  for (unsigned int i=0; i < 1; i++) {
      int whenToStop = exact[i];
      for (unsigned int j=0; j < dataSetDetails.size(); j++) {
        if (dataSetDetails[j].isDirected) {
          ::BanchMarkDataSetDijkstraUpToK< a::graph::TDirectedGraph >(dataSetDetails[j].dirPath, whenToStop);
        } else {
          ::BanchMarkDataSetDijkstraUpToK< a::graph::TUnDirectedGraph >(dataSetDetails[j].dirPath, whenToStop);
        }

      }
  }
}


#if 0
TEST_F(BasicGraph, BenchMarkRNNComputation) {
  long long exact[] = {16, 64, 128 };
  for (unsigned int i=0; i < 3; i++) {
      long long k = exact[i];
      for (unsigned int j=0; j < dataSetDetails.size(); j++) {
        std::string f_name = "./out/graph/state/" + dataSetDetails[j].name + "_" + std::to_string(k);
        std::cout << "loading file=" << f_name << std::endl;
        if (dataSetDetails[j].isDirected) {
          a::graph::Graph < a::graph::TDirectedGraph > graph;
          a::data::GraphADS graphAds;
          ::LoadGraph< a::graph::TDirectedGraph> (&graph, &graphAds, dataSetDetails[j].dirPath, k, f_name);
          ::ComputeRNNSamples< a::graph::TDirectedGraph >(&graph, &graphAds, dataSetDetails[j].dirPath, k, &dataSetDetails[j]);
        } else {
          a::graph::Graph < a::graph::TUnDirectedGraph > graph;
          a::data::GraphADS graphAds;
          ::LoadGraph< a::graph::TUnDirectedGraph> (&graph, &graphAds, dataSetDetails[j].dirPath, k, f_name);
          ::ComputeRNNSamples< a::graph::TUnDirectedGraph >(&graph, &graphAds, dataSetDetails[j].dirPath, k, &dataSetDetails[j]);
        }

      }
  }
}
#endif

TEST_F(BasicGraph, BenchMarkDijkstraSampleAll) {  
  int whenToStop = -1;
  for (unsigned int j=0; j < dataSetDetails.size(); j++) {
    if (dataSetDetails[j].isDirected) {
      ::BanchMarkDataSetDijkstraUpToK< a::graph::TDirectedGraph >(dataSetDetails[j].dirPath , whenToStop, true, &dataSetDetails[j]);
    } else {
      ::BanchMarkDataSetDijkstraUpToK< a::graph::TUnDirectedGraph >(dataSetDetails[j].dirPath, whenToStop, true, &dataSetDetails[j]);
    }
  }
}


TEST_F(BasicGraph, BenchMarkMultiADS) {
  std::ofstream f;
  f.open("./out/graph/results/threads_performance.csv");
  f << "data,K,graphTime,nodes,edges,ADSCalcTime,Threads,VM,RSS" <<std::endl;
  for (unsigned int j=0; j < dataSetDetails.size(); j++) {
    std::string dataSet = dataSetDetails[j].dirPath;
    for (unsigned int i=1; i < 15; i++) {   
      if (dataSetDetails[j].isDirected) {
      ::BanchMarkDataSetMultiADSCalculation< a::graph::TDirectedGraph >(dataSet, 16, i, &f);
      } else {
      ::BanchMarkDataSetMultiADSCalculation< a::graph::TUnDirectedGraph >(dataSet, 16, i, &f);
      }  
    }
  }
  f.close();
}

TEST_F(BasicGraph, BenchMarkMultiADSIncreaseFactor) {
  std::string dataSets[] = { "./data/slashdot"}; //GetSampleData()}; 
  bool directed[] = {true }; //false};  // 
  std::ofstream f;
  double factors[] = {1.05, 1.1, 1.15, 1.2, 1.3, 1.4, 1.5, 1.7, 1.8, 1.9 , 2};
  // double factors[] = {2, 3, 4, 5, 10, 20};
  f.open("./out/graph/results/increase_factor.csv");
  f << "data,K,graphTime,nodes,edges,ADSCalcTime,Threads,VM,RSS,factor" <<std::endl;
  for (unsigned int z=0; z < 11; z++) {
    for (unsigned int j=0; j < 1; j++) {
      std::string dataSet = dataSets[j];
      for (unsigned int i=1; i < 15; i++) {   
        if (directed[j]) {
        ::BanchMarkDataSetMultiADSCalculation< a::graph::TDirectedGraph >(dataSet, 64, i, &f, factors[z]);
        } else {
        ::BanchMarkDataSetMultiADSCalculation< a::graph::TUnDirectedGraph >(dataSet, 64, i, &f, factors[z]);
        }  
      }
    }
  }
  f.close();
}


TEST_F(BasicGraph, BenchMarkCorrectness) {  
  long long exact[] = {16, 64, 128 };
  for (unsigned int z=0; z< 3; z++) {
    for (unsigned int j=0; j < dataSetDetails.size(); j++) {
      std::string fileName = "./out/graph/results/error_" + dataSetDetails[j].name + "_" + std::to_string(exact[z]) + ".csv";
      std::ofstream f;
      f.open(fileName);
      if (dataSetDetails[j].isDirected) {
        ::ADSCalculationApproximationCheck< a::graph::TDirectedGraph >(dataSetDetails[j].dirPath,
                                                                       exact[z],
                                                                       dataSetDetails[j].isDirected,
                                                                       &dataSetDetails[j],
                                                                       &f);
      } else {
        ::ADSCalculationApproximationCheck< a::graph::TUnDirectedGraph >(dataSetDetails[j].dirPath,
                                                                         exact[z],
                                                                         dataSetDetails[j].isDirected,
                                                                         &dataSetDetails[j],
                                                                         &f);
      }
    }
  }
}

#if 0
TEST_F(BasicGraph, SerializeGraphsADSSketch) {  
  long long exact[] = {16, 64, 128 };
  for (unsigned int j=0; j < dataSetDetails.size(); j++) {
    for (unsigned int z=0; z< 3; z++) {
      std::string filename = "./out/graph/state/" + dataSetDetails[j].name + "_" + std::to_string(exact[z]);
      std::cout << "Creating " << filename << std::endl;
      if (dataSetDetails[j].isDirected) {
        ::SearlizeADSCalculation< a::graph::TDirectedGraph >(dataSetDetails[j].dirPath, exact[z], filename );
      } else {
        ::SearlizeADSCalculation< a::graph::TUnDirectedGraph >(dataSetDetails[j].dirPath, exact[z], filename);
      }
    }
  }
}
#endif

TEST_F(BasicGraph, ADSCalculationExtremeFacebookGraphK16) {
  ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >(GetSampleData(), 16);
}

TEST_F(BasicGraph, ADSCalculationExtremeFacebookGraphK64) {
  ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >(GetSampleData(), 64);
}

TEST_F(BasicGraph, ADSCalculationExtremeFacebookGraphK128) {
  ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >(GetSampleData(), 128); 
}


TEST_F(BasicGraph, ADSCalculationSlashdotGraphK16) {
  ::BanchMarkDataSetADSCalculation< a::graph::TDirectedGraph >("../../data/slashdot", 16);
}


TEST_F(BasicGraph, ADSCalculationExtremeTweeterGraphK64) {
  ::BanchMarkDataSetADSCalculation< a::graph::TDirectedGraph >("../../data/tweeter", 64);
}


/* 
 Undirected graph: ../../data/output/lj.ungraph.txt
 LiveJournal
 Nodes: 3997962 Edges: 34681189
 FromNodeId    ToNodeId
*/

TEST_F(BasicGraph, ADSCalculationExtremeLiveJournalGraphK64) {
  ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >("../../data/live_journal", 64);
}

TEST_F(BasicGraph, ADSCalculationExtremeYoutubeGraph64) {
  ::BanchMarkDataSetADSCalculation< a::graph::TUnDirectedGraph >("../../data/youtube", 64);
}

TEST_F(BasicGraph, sizes) {
  a::graph::Graph< a::graph::TUnDirectedGraph > graph;
  std::cout << " graph=" << sizeof(graph) << std::endl;
//  std::cout << " node ADS=" << sizeof(graph.myNodesADSId) << std::endl;
//  std::cout << " sorted_v=" << sizeof(graph.sorted_v) << std::endl;
  std::cout << " myWeightMap=" << sizeof(graph.myWeightMap) << std::endl;
  std::cout << " myGraph=" << sizeof(graph.myGraph) << std::endl;
//   std::cout << " myGraphADS=" << sizeof(graph.myGraphADS) << std::endl;
  std::cout << " Dijkstra params " << sizeof(a::algo::DijkstraParams) << std::endl;
}



typedef struct rank_t {
    a::graph::edge_weight_t distance;
    int node;
  } rank;
  
  struct compare_rank {
    bool operator()(const rank& n1, const rank& n2) const {
        return n1.distance < n2.distance;
    }
  };

TEST_F(BasicGraph, EstimateCommunityUsingDistance) {

  a::graph::Graph< a::graph::TUnDirectedGraph > graph;
  graph.LoadGraphFromDir("../../data/youtube");

  a::utils::Community com;
  com.LoadCommunity("../../data/youtube_comm");

  unsigned int numSamples = 10;
  a::algo::DijkstraParams p[10];
  a::utils::SingleCommunity * singleCom = com.GetCommunity(0);
  std::cout << " community size=" << singleCom->size() << std::endl;
  
  for (unsigned int i=0; i < numSamples; i++) {
    a::graph::TUnDirectedGraph::TNode source( (*singleCom)[i]);
    p[i].InitDijkstraParams(false);
    a::algo::CalculateNodeAdsPrunedDijkstra< a::graph::TUnDirectedGraph >(source, &graph, &(p[i]), graph.GetMxNId());
  }

  a::est::PriorVector prior;
  prior.resize(numSamples);
  for (unsigned int i=0; i < numSamples; i++) {
    prior[i].nodeId = (*singleCom)[i];
    for (unsigned int k=0; k < p[i].min_distance.size(); k++) {
      prior[i].order.push_back(p[i].min_distance[k]);
    }
  }
  
  int threshold = 1;
  for (unsigned int i=1; i < 10; i++) {
    threshold = i;
    a::est::BinaryMinClassifier minDistanceClassifier;
    minDistanceClassifier.InitBinaryClassifier(&prior, threshold);

    a::est::ClassifierAggregator< a::graph::TUnDirectedGraph > aggregator;
    aggregator.InitClassifierAggregator(&minDistanceClassifier, singleCom, &graph);
    double TPR;
    double FPR;
    aggregator.ExtractStats(&TPR, &FPR);

    std::cout << " TPR=" << TPR << " FPR=" << FPR << " Threshold=" << threshold << std::endl;  
  }
  
  for (unsigned int i=1; i < 10; i++) {
    threshold = i;
    a::est::BinaryAverageClassifier minDistanceClassifier;
    minDistanceClassifier.InitBinaryClassifier(&prior, threshold);

    a::est::ClassifierAggregator< a::graph::TUnDirectedGraph > aggregator;
    aggregator.InitClassifierAggregator(&minDistanceClassifier, singleCom, &graph);
    double TPR;
    double FPR;
    aggregator.ExtractStats(&TPR, &FPR);

    std::cout << " TPR=" << TPR << " FPR=" << FPR << " Threshold=" << threshold << std::endl;  
  }

}

/*

  for (unsigned int i=0; i < numSamples; i++) {
      // Build rank vector
      std::cout << "checking results of node =" << (*singleCom)[i] << std::endl;
      std::vector< rank > ranks;
      for (unsigned int j=0; j < (unsigned int )graph.GetMxNId() ; j++) {
          rank r;
          r.distance = p[i].min_distance[j];
          r.node = j;
          ranks.push_back(r);
      }
      compare_rank obj;
      std::sort(ranks.begin(), ranks.end(), obj);
      double averagePercentile = 0;
      double averagePercentileRandom = 0;
      for (unsigned int l=0; l < singleCom->size(); l++) {
          int member = (*singleCom)[l];
          
          for (unsigned int z=0; z < ranks.size(); z++) {
            if (ranks[z].node == member) {
              // std::cout << " member=" << member << " rank= " << z << " percentile= " <<  ( (double) z ) / graph.GetNumNodes()  << " distance= " << ranks[z].distance << std::endl;
              averagePercentile += ( (double) z ) / graph.GetNumNodes();
              break;
            }
          }

          int randomMember = rand() % graph.GetMxNId();
          for (unsigned int z=0; z < ranks.size(); z++) {
            if (ranks[z].node == randomMember) {
              averagePercentileRandom += ( (double) z ) / graph.GetNumNodes();
              break;
            }
          }
      }
      std::cout << "Average percentile=" << averagePercentile / singleCom->size() << std::endl;
      std::cout << "Average percentile random= " << averagePercentileRandom / singleCom->size() << std::endl;
  }
*/

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
