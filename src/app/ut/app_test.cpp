#include "gtest/gtest.h"

#include "../../all_distance_sketch.h"
#include "../sketch_calculation_app.h"
#include "../reverse_rank_app.h"
#include "../t_skim_reverse_rank_app.h"

using namespace all_distance_sketch;

class AppData : public ::testing::Test {
 protected:
  AppData() : file_index_(0) {}

  virtual ~AppData() { 
    for (int j=0; j < file_names.size(); j++) {
      std::remove(file_names[j].c_str());
      std::cout << " removed=" << file_names[j] << std::endl;
      DeleteFilesWithPrefix(file_names[j]);
    }

  }

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

  std::string GetTempFileName() {
    const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
    std::string test_name = test_info->name();
    std::string suffix = test_name + std::to_string(file_index_);
    std::string file_name = myFileBaseName + suffix;
    file_index_ += 1;
    file_names.push_back(file_name);
    return file_name;
  }

  std::vector<std::string> file_names;
  std::string mySampleData;
  std::string myFileBaseName = "app_test_";
  int file_index_;
};

char* convert_string_to_char_pointer(std::string s) {
  char * writable = new char[s.size() + 1];
  std::copy(s.begin(), s.end(), writable);
  writable[s.size()] = '\0';
  return writable;
}


TEST_F(AppData, SketchAppTestUndirected) {
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "sketch_app", 
                        "--K=64", 
                        writable,
                        "--directed=false",
                        convert_string_to_char_pointer(output_file)};
  EXPECT_EQ(sketch_app_main(5, arguments), 0);

  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);

  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  EXPECT_EQ(graph_sketch_from_app, graph_sketch);
}

TEST_F(AppData, SketchAppTestDirected) {
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "sketch_app", 
                        "--K=64", 
                        writable, 
                        convert_string_to_char_pointer(output_file),
                        "--directed=true"};
  EXPECT_EQ(sketch_app_main(5, arguments), 0);

  graph::Graph< graph::TDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  CalculateGraphSketch<graph::TDirectedGraph>(&graph, &graph_sketch);

  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  EXPECT_EQ(graph_sketch_from_app, graph_sketch);
}

TEST_F(AppData, ReverseRankAppTestUndirected) {
  /*
  --source_id arg        id of source node
  --K arg                K = 1/epsilon^2 sets the precision
  --num_threads arg (=1) num_threads to use
  --directed arg         is the graph directed
  --graph_dir arg        Directory with the graph to calculate the sketch on
  --sketch_file arg      File with the calculated sketch
  --output_file arg
  */
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "app",
                        "--source_id=0", 
                        "--K=64",
                        "--directed=false",
                        writable, 
                        convert_string_to_char_pointer(output_file)};
  EXPECT_EQ(reverse_rank_app_main(6, arguments), 0);

  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  std::cout << "Max node id=" << graph.GetMxNId() << std::endl;
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);
  int node_id = 0;
  std::vector<int> ranking;
  CalculateReverseRank<graph::TUnDirectedGraph> (node_id,
                                                 &graph,
                                                 &graph_sketch,
                                                 &ranking);

  std::fstream input(file_name, std::ios::in | std::ios::binary);
  NodeRanksGpb node_ranks;
  EXPECT_TRUE(node_ranks.ParseFromIstream(&input));
  for (int i=0; i < node_ranks.node_ranks_size(); i++) {
    const NodeRankGpb& rank = node_ranks.node_ranks(i);
    int node_id = rank.node_id();
    int node_rank = rank.node_rank();
    EXPECT_EQ(node_rank, ranking[node_id]);
  }
}

TEST_F(AppData, ReverseRankAppTestDirected) {
  /*
  --source_id arg        id of source node
  --K arg                K = 1/epsilon^2 sets the precision
  --num_threads arg (=1) num_threads to use
  --directed arg         is the graph directed
  --graph_dir arg        Directory with the graph to calculate the sketch on
  --sketch_file arg      File with the calculated sketch
  --output_file arg
  */
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;

  graph::Graph< graph::TDirectedGraph> graph;
  graph.LoadGraphFromDir("./data/youtube");
  
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += "./data/youtube";
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "app",
                        "--source_id=32", 
                        "--K=4",
                        writable, 
                        convert_string_to_char_pointer(output_file),
                        "--directed=true"};
  EXPECT_EQ(reverse_rank_app_main(6, arguments), 0);
  
  GraphSketch graph_sketch;
  int k = 4;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  CalculateGraphSketch<graph::TDirectedGraph>(&graph, &graph_sketch);
  graph::Graph<graph::TDirectedGraph> graph_transpose;
  graph.Transpose(&graph_transpose);
  

  int node_id = 32;
  std::vector<int> ranking;
  CalculateReverseRank<graph::TDirectedGraph> (node_id,
                                               &graph_transpose,
                                               &graph_sketch,
                                               &ranking);
  std::fstream input(file_name, std::ios::in | std::ios::binary);
  NodeRanksGpb node_ranks;
  EXPECT_TRUE(node_ranks.ParseFromIstream(&input));
  std::cout << "size=" << node_ranks.node_ranks_size() << " ranking=" << ranking.size() << std::endl;
  for (int i=0; i < node_ranks.node_ranks_size(); i++) {
    const NodeRankGpb& rank = node_ranks.node_ranks(i);
    int node_id = rank.node_id();
    int node_rank = rank.node_rank();
    std::cout << node_rank << std::endl;
    EXPECT_EQ(node_rank, ranking[node_id]);
  }
}


TEST_F(AppData, TSkimAppTestUndirected) {
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;

  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "app", 
                        "--K=64", 
                        "--T=10",
                        "--directed=false",
                        "--min_influence_for_seed_set=10",
                        writable, 
                        convert_string_to_char_pointer(output_file)};
  EXPECT_EQ(t_skim_app_main(7, arguments), 0);
  
  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  int k = 64;
  int T=10;
  int min_influence_for_seed_set=10;
  Cover cover;
  TSkimReverseRank< graph::TUnDirectedGraph > t_skim_algo_un_directed;
  t_skim_algo_un_directed.InitTSkim(T, k, min_influence_for_seed_set, &cover, &graph);
  t_skim_algo_un_directed.Run();

  
  std::fstream input(file_name, std::ios::in | std::ios::binary);
  CoverGpb coverGpb;
  EXPECT_TRUE(coverGpb.ParseFromIstream(&input));

  for (int i=0; i < coverGpb.seeds_size(); i++) {
    const SeedInfoGpb& seed_info = coverGpb.seeds(i);
    int seed_node_id = seed_info.seed_node_id();
    for (int j=0; j < seed_info.node_ids_size(); j++) {
      int node_id = seed_info.node_ids(j);
      auto seed_cover = cover.GetSeedCover(seed_node_id);
      EXPECT_EQ(seed_cover.seed, seed_node_id);
      bool found = false;
      EXPECT_TRUE(seed_cover.covered_nodes.size() != 0);
      for (int k=0; k < seed_cover.covered_nodes.size(); k++) {
        if (seed_cover.covered_nodes[k] == node_id) {
          found = true;
          break;
        }
      }
      EXPECT_TRUE(found);
    }
  }
}

TEST_F(AppData, TSkimAppTestDirected) {
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;

  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "app", 
                        "--K=64", 
                        "--T=10",
                        "--min_influence_for_seed_set=10",
                        writable, 
                        convert_string_to_char_pointer(output_file),
                        "--directed=true"};
  EXPECT_EQ(t_skim_app_main(7, arguments), 0);
  
  graph::Graph< graph::TDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  int k = 64;
  int T=10;
  int min_influence_for_seed_set=10;
  Cover cover;
  TSkimReverseRank< graph::TDirectedGraph > t_skim_algo_directed;
  t_skim_algo_directed.InitTSkim(T, k, min_influence_for_seed_set, &cover, &graph);
  t_skim_algo_directed.Run();

  
  std::fstream input(file_name, std::ios::in | std::ios::binary);
  CoverGpb coverGpb;
  EXPECT_TRUE(coverGpb.ParseFromIstream(&input));

  for (int i=0; i < coverGpb.seeds_size(); i++) {
    const SeedInfoGpb& seed_info = coverGpb.seeds(i);
    int seed_node_id = seed_info.seed_node_id();
    for (int j=0; j < seed_info.node_ids_size(); j++) {
      int node_id = seed_info.node_ids(j);
      auto seed_cover = cover.GetSeedCover(seed_node_id);
      EXPECT_EQ(seed_cover.seed, seed_node_id);
      bool found = false;
      EXPECT_TRUE(seed_cover.covered_nodes.size() != 0);
      for (int k=0; k < seed_cover.covered_nodes.size(); k++) {
        if (seed_cover.covered_nodes[k] == node_id) {
          found = true;
          break;
        }
      }
      EXPECT_TRUE(found);
    }
  }
}


TEST_F(AppData, SketchAppTestZValueInputFailTest) {
  std::string output_file = "--output_file=";
  std::string file_name = GetTempFileName();
  output_file += file_name;

  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  char *arguments[] = { "sketch_app", 
                        "--K=64",
                        "--should_calc_insert_prob=true",
                        writable,
                        "--directed=false",
                        convert_string_to_char_pointer(output_file)};
  EXPECT_EQ(sketch_app_main(6, arguments), 0);

  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch.set_should_calc_zvalues(false);
  CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);
  std::cout << "graph_sketch.should_calc_insert_prob=" << graph_sketch.get_should_calc_zvalues() << std::endl;
  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  std::cout << "graph_sketch_from_app.should_calc_insert_prob=" << graph_sketch_from_app.get_should_calc_zvalues() << std::endl;
  EXPECT_TRUE(graph_sketch_from_app != graph_sketch);
}

TEST_F(AppData, SketchAppTestZValueTest) {
  std::string file_name = GetTempFileName();
  std:: string output_option = "--output_file=";
  output_option += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  
  char * writable = new char[graph_dir_arg.size() + 1];
  std::copy(graph_dir_arg.begin(), graph_dir_arg.end(), writable);
  writable[graph_dir_arg.size()] = '\0';
  
  char *arguments[] = { "sketch_app", 
                        "--K=64",
                        "--should_calc_insert_prob=true",
                        writable,
                        "--directed=false",
                        convert_string_to_char_pointer(output_option) };
  EXPECT_EQ(sketch_app_main(6, arguments), 0);

  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch.set_should_calc_zvalues(true);
  CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);
  // std::cout << "graph_sketch.should_calc_insert_prob=" << graph_sketch.get_should_calc_zvalues() << std::endl;
  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  // std::cout << "graph_sketch_from_app.should_calc_insert_prob=" << graph_sketch_from_app.get_should_calc_zvalues() << std::endl;
  EXPECT_TRUE(graph_sketch_from_app == graph_sketch);
}


TEST_F(AppData, SketchAppTestNodeDistributionTest) {
  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch.set_should_calc_zvalues(true);
  CalculateGraphSketch<graph::TUnDirectedGraph>(&graph, &graph_sketch);
  const std::vector<RandomId>* node_dist = graph_sketch.GetNodesDistributionLean();

  std::string distribution_file_name = GetTempFileName();
  ofstream dist_file;
  dist_file.open(distribution_file_name);
  for (int i=0; i < node_dist->size(); i++) {
    double random_id = (*node_dist)[i];
    int node_id = i;
    dist_file << node_id << "," << random_id << "\n";
  }
  dist_file.close();

  
  std::string file_name = GetTempFileName();
  std:: string output_option = "--output_file=";
  output_option += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  std::string node_distribution_option = "--nodes_distribution=";
  node_distribution_option += distribution_file_name;
  
  char *arguments[] = { "sketch_app", 
                        "--K=64",
                        "--should_calc_insert_prob=true",
                        convert_string_to_char_pointer(graph_dir_arg),
                        "--directed=false",
                        convert_string_to_char_pointer(output_option),
                        convert_string_to_char_pointer(node_distribution_option)};
  EXPECT_EQ(sketch_app_main(7, arguments), 0);
  

  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  const std::vector<RandomId>* node_dist_app = graph_sketch_from_app.GetNodesDistributionLean();
  EXPECT_EQ(node_dist_app->size(), node_dist->size());
  for (int i=0; i < node_dist->size(); i++) {
    EXPECT_TRUE(double_equals((*node_dist)[i], (*node_dist_app)[i]));
  }
}

TEST_F(AppData, SketchAppTestSelectiveRunning) {
  graph::Graph< graph::TUnDirectedGraph> graph;
  graph.LoadGraphFromDir(GetSampleData());
  GraphSketch graph_sketch;
  int k = 64;
  graph_sketch.InitGraphSketch(k, graph.GetMxNId());
  graph_sketch.set_should_calc_zvalues(true);
  const std::vector<RandomId>* node_dist = graph_sketch.GetNodesDistributionLean();

  std::string distribution_file_name = GetTempFileName();
  ofstream dist_file;
  dist_file.open(distribution_file_name);
  dist_file << 1 << "," << (*node_dist)[1] << "\n";
  dist_file << 10 << "," << (*node_dist)[10] << "\n";
  dist_file << 50 << "," << (*node_dist)[50] << "\n";
  dist_file.close();

  
  std::string file_name = GetTempFileName();
  std:: string output_option = "--output_file=";
  output_option += file_name;
  std::string graph_dir_arg = "--graph_dir=";
  graph_dir_arg += GetSampleData();
  std::string node_distribution_option = "--nodes_distribution=";
  node_distribution_option += distribution_file_name;
  /*
  std::vector<int> nodes_to_run = {1, 10, 50};

  std::string nodes_to_run_file_name = GetTempFileName();
  dist_file.open(nodes_to_run_file_name);
  for (int i=0; i < nodes_to_run.size(); i++) {
    int node_id = nodes_to_run[i];
    dist_file << node_id << "\n";
  }
  dist_file.close();
  std::string nodes_to_run_option = "--nodes_to_run=";
  nodes_to_run_option += nodes_to_run_file_name;
  */

  char *arguments[] = { "sketch_app", 
                        "--K=64",
                        "--should_calc_insert_prob=true",
                        convert_string_to_char_pointer(graph_dir_arg),
                        "--directed=false",
                        convert_string_to_char_pointer(output_option),
                        convert_string_to_char_pointer(node_distribution_option)};
                        // convert_string_to_char_pointer(nodes_to_run_option)};
  EXPECT_EQ(sketch_app_main(7, arguments), 0);
  

  GraphSketch graph_sketch_from_app;
  load_sketch(&graph_sketch_from_app, file_name);
  const std::vector<RandomId>* node_dist_app = graph_sketch_from_app.GetNodesDistributionLean();
  EXPECT_TRUE((*node_dist_app)[1] != ILLEGAL_RANDOM_ID);
  EXPECT_TRUE((*node_dist_app)[10] != ILLEGAL_RANDOM_ID);
  EXPECT_TRUE((*node_dist_app)[50] != ILLEGAL_RANDOM_ID);

  for (auto it=graph_sketch_from_app.Begin(); it != graph_sketch_from_app.End(); it++) {
    const NodeIdDistanceVector* vec = it->GetNodeAdsVector();
    EXPECT_TRUE(vec->size() == 3 || vec->size() == 0);
    for (auto element : *vec) {
      // All elements should be in the seed set
      EXPECT_TRUE(element.GetNId() == 1 || element.GetNId() == 10 || element.GetNId() == 50);
    }
  }
}