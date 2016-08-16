#ifndef SRC_ALGORITHMS_DISTANCE_DIFFUSION_H_
#define SRC_ALGORITHMS_DISTANCE_DIFFUSION_H_

#include "../algorithms/sketch_calculation.h"
#include "../labels/labels.h"
#include "../measures/decay_interface.h"
#include "../sketch/graph_sketch.h"

namespace all_distance_sketch {

template <class T>
void InitGraphSketches(graph::Graph<T>* graph,
				 		NodesFeaturesSortedContainer* seed_set,
				 		int K,
				 		GraphSketch* all_graph_sketch,
				 		GraphSketch* only_seed_nodes_sketch) {
	// Init both graph sketches
	all_graph_sketch->InitGraphSketch(K, graph->GetMxNId());
	only_seed_nodes_sketch->InitGraphSketch(K, graph->GetMxNId());
	all_graph_sketch->set_should_calc_zvalues(true);
	only_seed_nodes_sketch->set_should_calc_zvalues(true);
	// At this point the seed sketch distribution is equal to the full graph distribution (random ids)
	const std::vector<RandomId>* dist = all_graph_sketch->GetNodesDistributionLean();
	std::vector<RandomId> seed_set_distribution;
	seed_set_distribution.resize(graph->GetMxNId(), ILLEGAL_RANDOM_ID);
	for (auto node_info : seed_set->GetNodesFeatures()) {
		int seed_id = node_info.GetNId();
		seed_set_distribution[seed_id] = (*dist)[seed_id];
	}
	only_seed_nodes_sketch->SetNodesDistribution(&seed_set_distribution);
	// Calculated the graph sketch
	CalculateGraphSketch<T>(graph, all_graph_sketch);
	// Run the sketch calculation only from the seet set
	CalculateGraphSketch<T>(graph, only_seed_nodes_sketch);
}


template <class T>
void calculate_labels_distance_diffusion(graph::Graph<T>* graph,
												int feature_dim,
										 		NodesFeaturesSortedContainer* seed_set,
										 		DecayInterface* decay_func,
										 		NodesFeaturesContainer* node_labels,
										 		GraphSketch* all_graph_sketch,
										 		GraphSketch* only_seed_nodes_sketch) {
	bool should_delete_graph_sketches = false;
	// Iterate each node in the graph
	for (auto nodeItr = graph->BegNI();  nodeItr != graph->EndNI() ; nodeItr++ ){
		int node_id = nodeItr.GetId();
		// Get the node sketchs. 
		// One when we run the algorithm on all the nodes in the graph
		NodeSketch* node_sketch_all_graph  = all_graph_sketch->GetNodeSketch(node_id);
		node_sketch_all_graph->CalculateInsertProb();
		LOG_M(DEBUG3, " node id =" << node_id);
		// Second is when we run the algorithm only from seed nodes
		NodeSketch* node_sketch_only_seeds = only_seed_nodes_sketch->GetNodeSketch(node_id);
		LOG_M(DEBUG3, " node id" << node_id << " node_sketch_only_seeds=" << node_sketch_only_seeds);
		const NodeIdDistanceVector* node_ads_vector = node_sketch_only_seeds->GetNodeAdsVector();
		FEATURE_WEIGHTS_VECTOR node_feature_vector;
		node_feature_vector.resize(feature_dim, 0);
		double normalization_factor = 0;
		bool is_seed_node = false;
		for (auto node_seed_ads_itr = node_ads_vector->begin(); node_seed_ads_itr != node_ads_vector->end(); node_seed_ads_itr++) {
			// Seed node Id
			int seed_id = node_seed_ads_itr->GetNId();
			// ignore the seed nodes
			if (seed_id == node_id) {
				is_seed_node = true;
				break;
			}
			// Distance from seed to the source node
			graph::EdgeWeight seed_distance = node_seed_ads_itr->GetDistance();
			// The rank of the seed compared to all other nodes in the graph
			int seed_rank = node_sketch_all_graph->GetSizeNeighborhoodUpToDistance(seed_distance);
			// Insert probability based on the full graph
			double insert_prob = node_sketch_all_graph->GetInsertProbAccordingToDistance(seed_distance);
			// Get feature vector of seed label
			double estimated_rank = decay_func->Alpha(seed_rank) / insert_prob;
			LOG_M(DEBUG3, "node id=" << node_id <<  " seed node=" << seed_id << " seed_rank=" << seed_rank << " seed distance=" << seed_distance << 
						  " insert_prob=" << insert_prob << " estimated_rank=" << estimated_rank << " alpha(rank)=" << decay_func->Alpha(seed_rank));
			normalization_factor += (1 / insert_prob);
			const FEATURE_WEIGHTS_VECTOR* vec = seed_set->GetSeedFeature(seed_id);
			for (int i=0; i < feature_dim; i++) {
				node_feature_vector[i] += (*vec)[i] * estimated_rank;
				LOG_M(DEBUG5," post mul=" << node_feature_vector[i]);
			}
		}
		if (is_seed_node) {
			continue;
		}
		LOG_M(DEBUG3, "node_id=" << node_id << " normalization_factor=" << normalization_factor);
		for (int i=0; i < node_feature_vector.size(); i++) {
			node_feature_vector[i] = node_feature_vector[i] / normalization_factor;
		}
		node_labels->AddNodeFeature(node_id, node_feature_vector);
	}
	if (should_delete_graph_sketches) {
		delete all_graph_sketch;
		delete only_seed_nodes_sketch;
	}
}

}  // all_distance_sketch

#endif  // SRC_ALGORITHMS_DISTANCE_DIFFUSION_H_
