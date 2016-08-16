#include <limits.h>
#include "gtest/gtest.h"
#include "../labels.h"
#include "../../graph/snap_graph_adaptor.h"
#include "../../sketch/graph_sketch.h"
#include "../../algorithms/distance_diffusion.h"

using namespace all_distance_sketch;

class BasicLabel : public ::testing::Test {
 protected:
  BasicLabel() {}

  virtual ~BasicLabel() { }

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

TEST_F(BasicLabel, BuildLabelsSorted) {
  // Insert and iterate features + check that they are sorted in increasing node id
  NodesFeaturesSortedContainer nodes_label;
  int num_seeds = 100;
  srand(time(NULL));
  FEATURE_WEIGHTS_VECTOR vector_dummy;
  vector_dummy.resize(10);
  for (int i=0; i < num_seeds; i++) {
    int random_id = rand() % 1000000;
    nodes_label.AddNodeFeature(random_id, vector_dummy);
  }
  auto nodes_feature_vec = nodes_label.GetNodesFeatures();

  for (int i=0; i < nodes_feature_vec.size() - 1; i++) {
    auto node_id_1 = nodes_feature_vec[i].GetNId();
    auto node_id_2 = nodes_feature_vec[i+1].GetNId();
    // std::cout << " node_id_1=" << node_id_1 << " node_id_2=" << node_id_2 << std::endl;
    EXPECT_TRUE(node_id_1 <= node_id_2);
  }
}

/*
  See ~/all_distance_sketch/doc/Distance_diffusion_example.jpg for picture of the example
*/
void create_example_graph(all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph >* graph) {
  int num_nodes = 9;
  // Adding 9 nodes
  for (int i=0; i < num_nodes; i++) {
    graph->AddNode(i);
  }
  // Creating edges
  // 0->1, 2
  graph->AddEdge(0, 1);
  graph->AddEdge(0, 2);
  // 1->2, 3
  graph->AddEdge(1, 2);
  graph->AddEdge(1, 3);
  // 2->3
  graph->AddEdge(2, 3);
  // 3->0, 4
  graph->AddEdge(3, 0);
  graph->AddEdge(3, 4);
  // 4->5, 8
  graph->AddEdge(4, 5);
  graph->AddEdge(4, 8);
  // 5->6, 8
  graph->AddEdge(5, 6);
  graph->AddEdge(5, 8);
  // 6->4, 5, 8
  graph->AddEdge(6, 4);
  graph->AddEdge(6, 5);
  graph->AddEdge(6, 8);
  // 7->4, 8
  graph->AddEdge(7, 4);
  graph->AddEdge(7, 8);
}

TEST_F(BasicLabel, CheckExampleInsertProb) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > graph;
  GraphSketch graph_sketch;
  int k = 2;
  create_example_graph(&graph);
  std::vector<RandomId> probabilites = {0.06, 0.95, 0.32, 0.69, 0.77, 0.34, 0.12, 0.33, 0.93};
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch.SetNodesDistribution(&probabilites);
  graph_sketch.set_should_calc_zvalues(true);
  CalculateGraphSketch< graph::TDirectedGraph >(&graph, &graph_sketch);
  NodeSketch* node_sketch = graph_sketch.GetNodeSketch(3);
  std::vector<NodeProb> insert_prob;
  node_sketch->CalculateInsertProb();
  insert_prob = node_sketch->GetInsertProb();
  EXPECT_EQ(insert_prob.size(), 3);
  EXPECT_EQ(insert_prob[0].node_id, 0);
  EXPECT_EQ(insert_prob[1].node_id, 2);
  EXPECT_EQ(insert_prob[2].node_id, 3);

  EXPECT_EQ(insert_prob[0].prob, 0.69);
  EXPECT_EQ(insert_prob[1].prob, 0.95);
  EXPECT_EQ(insert_prob[2].prob, 1);

  EXPECT_EQ(insert_prob[0].distance, 2);
  EXPECT_EQ(insert_prob[1].distance, 1);
  EXPECT_EQ(insert_prob[2].distance, 0);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(0), 1);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(1), 0.95);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(2), 0.69);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(3), 0.06);
  /*
  std::cout << "Insert prob for node id" << 3 << std::endl;
  for (int i=0; i < insert_prob.size(); i++) {
    std::cout << "NodeId=" << insert_prob[i].node_id << " Prob=" << insert_prob[i].prob << " Distance=" << insert_prob[i].distance << std::endl;
  }
  */
  node_sketch = graph_sketch.GetNodeSketch(4);
  insert_prob.clear();
  node_sketch->CalculateInsertProb();
  insert_prob = node_sketch->GetInsertProb();
  /*
  std::cout << "Insert prob for node id" << 4 << std::endl;
  NodeId=0 Prob=0.32 Distance=3
  NodeId=2 Prob=0.33 Distance=2
  NodeId=7 Prob=0.69 Distance=1
  NodeId=6 Prob=0.69 Distance=1
  */
  EXPECT_EQ(insert_prob.size(), 5);
  EXPECT_EQ(insert_prob[0].node_id, 0);
  EXPECT_EQ(insert_prob[1].node_id, 2);
  EXPECT_EQ(insert_prob[2].node_id, 7);
  EXPECT_EQ(insert_prob[3].node_id, 6);
  EXPECT_EQ(insert_prob[4].node_id, 4);

  EXPECT_EQ(insert_prob[0].distance, 3);
  EXPECT_EQ(insert_prob[1].distance, 2);
  EXPECT_EQ(insert_prob[2].distance, 1);
  EXPECT_EQ(insert_prob[3].distance, 1);
  EXPECT_EQ(insert_prob[4].distance, 0);

  EXPECT_EQ(insert_prob[0].prob, 0.32);
  EXPECT_EQ(insert_prob[1].prob, 0.33);
  EXPECT_EQ(insert_prob[2].prob, 0.69);
  EXPECT_EQ(insert_prob[3].prob, 0.69);
  EXPECT_EQ(insert_prob[4].prob, 1);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(0), 1);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(1), 0.69);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(2), 0.33);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(3), 0.32);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(4), 0.06);
  EXPECT_EQ(node_sketch->GetInsertProbAccordingToDistance(400), 0.06);
}

TEST_F(BasicLabel, CheckSeedNodeExtraction) {
  NodesFeaturesSortedContainer feature_container;
  int num_nodes = 100;
  srand(time(NULL));
  int dim = 3;
  std::unordered_map<int, FEATURE_WEIGHTS_VECTOR> feature_map;
  for (int i=0; i < num_nodes; i++) {
    int node_id = rand();
    FEATURE_WEIGHTS_VECTOR vec;
    vec.resize(dim);
    for (int j=0; j < dim; j++) {
      vec[j] = rand();
    }
    // std::cout << "Adding node " << node_id << std::endl; 
    feature_map.insert(std::make_pair(node_id, vec));
    feature_container.AddNodeFeature(node_id, vec);
  }
  for (auto it=feature_map.begin(); it != feature_map.end(); it++) {
    // std::cout << "Iterating " << it->first << std::endl;
    auto vec_1 = feature_map[it->first];
    const FEATURE_WEIGHTS_VECTOR* vec_2 = feature_container.GetSeedFeature(it->first);
    ASSERT_TRUE(vec_2 != NULL);
    for (int i=0; i < dim; i++) {
      EXPECT_EQ(vec_1[i], (*vec_2)[i]);
    }
  }
}

TEST_F(BasicLabel, CheckDistanceDiffusion) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > graph;
  create_example_graph(&graph);
  int feature_dim = 10;
  FEATURE_WEIGHTS_VECTOR vec;
  vec.resize(feature_dim, 1);
  NodesFeaturesSortedContainer seed_set;
  seed_set.AddNodeFeature(0, vec);
  seed_set.AddNodeFeature(2, vec);
  int K=2;
  InverseDecay decay_func;
  NodesFeaturesContainer output_features;
  GraphSketch all_graph_sketch;
  GraphSketch only_seed_nodes_sketch;
  InitGraphSketches(&graph, &seed_set, K, &all_graph_sketch, &only_seed_nodes_sketch);
  calculate_labels_distance_diffusion<all_distance_sketch::graph::TDirectedGraph>(
                                      &graph, 
                                      feature_dim,
                                      &seed_set,
                                      &decay_func,
                                      &output_features,
                                      &all_graph_sketch,
                                      &only_seed_nodes_sketch);
  for (auto node_feature_info : output_features.GetNodesFeatures()) {
    if (node_feature_info.GetNId() == 3) {
      for (auto feature_weight : node_feature_info.GetFeatureWeights()) {
        NodeSketch* node_sketch = all_graph_sketch.GetNodeSketch(3);
        auto insert_prob_1 = node_sketch->GetInsertProbAccordingToDistance(1);
        auto insert_prob_2 = node_sketch->GetInsertProbAccordingToDistance(2);
        //std::cout << " insert_prob_1=" << insert_prob_1 << " estimated rank=" << 1/(insert_prob_1 * 2) << std::endl;
        //std::cout << " insert_prob_2=" << insert_prob_2 << " estimated rank=" << 1/(insert_prob_2 * 3 ) << std::endl;
        //std::cout << " base=" << (1 / insert_prob_1) + (1 / insert_prob_2) << std::endl;
        double expected_result = 1/(insert_prob_1 * 2) + 1/(insert_prob_2 * 3 );
        expected_result = expected_result / ((1 / insert_prob_1) + (1 / insert_prob_2));
        EXPECT_EQ( static_cast<int>(feature_weight * 1000), static_cast<int>(expected_result * 1000));
      }
    }
  }
}

TEST_F(BasicLabel, CheckSeedOnly) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > graph;
  create_example_graph(&graph);
  GraphSketch all_graph_sketch;
  GraphSketch only_seed_nodes_sketch;
  int K = 64;
  NodesFeaturesSortedContainer seed_set;
  FEATURE_WEIGHTS_VECTOR vec;
  vec.resize(3, 1);
  seed_set.AddNodeFeature(0, vec);
  seed_set.AddNodeFeature(3, vec);
  seed_set.AddNodeFeature(6, vec);
  seed_set.AddNodeFeature(7, vec);
  InitGraphSketches(&graph, &seed_set, K,
                    &all_graph_sketch, &only_seed_nodes_sketch);
  for (auto it=only_seed_nodes_sketch.Begin(); it != only_seed_nodes_sketch.End(); it++) {
    const NodeIdDistanceVector* vec = it->GetNodeAdsVector();
    for (auto element : *vec) {
      // All elements should be in the seed set
      EXPECT_TRUE(seed_set.GetSeedFeature(element.GetNId()) != NULL);
    }
  }
}

TEST_F(BasicLabel, CheckVectorCalculation1) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }
  // Edges between every node and the two nodes 0,1
  // every node will have an edge between them to i -> 0 and i -> 1
  for (int i=2; i < num_nodes ;i++) {
    graph.AddEdge(0, i);
    graph.AddEdge(1, i);
  }
  GraphSketch all_graph_sketch;
  GraphSketch only_seed_nodes_sketch;
  int K = 64;
  int feature_dim = 3;
  NodesFeaturesSortedContainer seed_set;
  FEATURE_WEIGHTS_VECTOR vec_1;
  FEATURE_WEIGHTS_VECTOR vec_2;
  // Adding two seed nodes 0, 2
  // Every node will give node 0 rank 1
  // Every node will give node 2 rank 2
  // We add both nodes as seed nodes
  vec_1.resize(feature_dim, 1);
  vec_2.resize(feature_dim, 2);
  seed_set.AddNodeFeature(0, vec_1);
  seed_set.AddNodeFeature(1, vec_2);
  // Calculate the graph sketches according to the seed sets
  InitGraphSketches(&graph, &seed_set, K,
                    &all_graph_sketch, &only_seed_nodes_sketch);
  // We need to first make sure that they are the only ones in the ADS
  for (auto it=only_seed_nodes_sketch.Begin(); it != only_seed_nodes_sketch.End(); it++) {
    const NodeIdDistanceVector* vec = it->GetNodeAdsVector();
    for (auto element : *vec) {
      // All elements should be in the seed set
      EXPECT_TRUE(seed_set.GetSeedFeature(element.GetNId()) != NULL);
    }
  }
  // We run the algorithm and make sure that the outcome is the wanted one
  InverseDecay decay_func;
  NodesFeaturesContainer output_features;
  calculate_labels_distance_diffusion<all_distance_sketch::graph::TDirectedGraph>(
                                      &graph,
                                      feature_dim,
                                      &seed_set,
                                      &decay_func,
                                      &output_features,
                                      &all_graph_sketch,
                                      &only_seed_nodes_sketch);
  // Iterate over the output features and check that we have the correct
  // features
  for (auto node_feature_info : output_features.GetNodesFeatures()) {
    int node_id = node_feature_info.GetNId();
    for (auto feature_weight : node_feature_info.GetFeatureWeights()) {
      NodeSketch* node_sketch = all_graph_sketch.GetNodeSketch(node_id);
      auto insert_prob_0 = node_sketch->GetInsertProbAccordingToDistance(1);
      auto insert_prob_2 = node_sketch->GetInsertProbAccordingToDistance(1);
      // std::cout << " node id=" << node_id << std::endl;
      // std::cout << " insert_prob_0=" << insert_prob_0 << " insert_prob_2=" << insert_prob_2 << std::endl;
      // std::cout << " feature weight=" << feature_weight << std::endl;
      double expected_result = 1/(insert_prob_0 * 2) + 2/(insert_prob_2 * 2);
      expected_result = expected_result / ((1 / insert_prob_0) + (1 / insert_prob_2));
      EXPECT_EQ( static_cast<int>(feature_weight * 1000), static_cast<int>(expected_result * 1000));
      // std::cout << static_cast<int>(feature_weight * 1000) << std::endl;
    }
  }
}


TEST_F(BasicLabel, CheckVectorCalculation2) {
  all_distance_sketch::graph::Graph< all_distance_sketch::graph::TDirectedGraph > graph;
  int num_nodes = 100;
  for (int i=0; i < num_nodes; i++) {
    graph.AddNode(i);
  }
  int num_seeds = 10;
  // Edges between every node and the seed set
  for (int i=num_seeds; i < num_nodes ;i++) {
    for (int j=0; j < num_seeds; j++) {
      graph.AddEdge(j, i);
    }
  }
  GraphSketch all_graph_sketch;
  GraphSketch only_seed_nodes_sketch;
  int K = 64;
  int feature_dim = 3;
  NodesFeaturesSortedContainer seed_set;
  FEATURE_WEIGHTS_VECTOR vec_1;
  FEATURE_WEIGHTS_VECTOR vec_2;
  // Adding num_seeds seed nodes
  vec_1.resize(feature_dim, 1);
  for (int i=0; i < num_seeds; i++) {
    seed_set.AddNodeFeature(i, vec_1);
  }
  // Calculate the graph sketches according to the seed sets
  InitGraphSketches(&graph, &seed_set, K,
                    &all_graph_sketch, &only_seed_nodes_sketch);
  // We need to first make sure that they are the only ones in the ADS
  for (auto it=only_seed_nodes_sketch.Begin(); it != only_seed_nodes_sketch.End(); it++) {
    const NodeIdDistanceVector* vec = it->GetNodeAdsVector();
    for (auto element : *vec) {
      // All elements should be in the seed set
      EXPECT_TRUE(seed_set.GetSeedFeature(element.GetNId()) != NULL);
    }
  }
  // We run the algorithm and make sure that the outcome is the wanted one
  InverseDecay decay_func;
  NodesFeaturesContainer output_features;
  calculate_labels_distance_diffusion<all_distance_sketch::graph::TDirectedGraph>(
                                      &graph,
                                      feature_dim,
                                      &seed_set,
                                      &decay_func,
                                      &output_features,
                                      &all_graph_sketch,
                                      &only_seed_nodes_sketch);
  // Iterate over the output features and check that we have the correct
  // features
  for (auto node_feature_info : output_features.GetNodesFeatures()) {
    int node_id = node_feature_info.GetNId();
    for (auto feature_weight : node_feature_info.GetFeatureWeights()) {
      NodeSketch* node_sketch = all_graph_sketch.GetNodeSketch(node_id);
      double expected_results = 0;
      double base = 0;
      for (int i=0; i < num_seeds; i++) {
        double insert_prob = node_sketch->GetInsertProbAccordingToDistance(1);
        expected_results += 1/(insert_prob * num_seeds);
        base += 1 / insert_prob;
      }
      expected_results = expected_results / base;
      EXPECT_TRUE( std::abs(feature_weight - expected_results) < 0.001) ;
      // std::cout << static_cast<int>(feature_weight * 1000) << std::endl;
    }
  }
}


