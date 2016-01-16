using namespace all_distance_sketch;
/*
	Calculate TSkim and iterate the result cover
*/
void calculate_t_skim_cover(graph::Graph< graph::TDirectedGraph >* graph) {
  Cover cover;
  // Drop line to help tagging
  TSkimForwardRank
  				< graph::TDirectedGraph > t_skim_algo;
  int T = 100;
  int K_all_distance_sketch=128;
  int min_influence_for_seed=500;
  t_skim_algo.InitTSkim(T, K_all_distance_sketch, min_influence_for_seed, &cover, graph);
  t_skim_algo.Run();
  for (auto it = cover.Begin(); it != cover.End(); ++it) {
  	std::cout << " node id of seed=" << it->second.seed << 
  				 " was selected at the " << it->second.index << std::endl;
    for (int i=0; i < it->second.covered_nodes.size(); i++) {
      int node_id = it->second.covered_nodes[i];
      std::cout << " node id=" << node_id <<
      			   " is covered by=" << it->second.seed << std::endl;
    }
  }
}