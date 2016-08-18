#ifndef SRC_GRAPH_LABELS_LABELS_H_
#define SRC_GRAPH_LABELS_LABELS_H_

#include "../common.h"
#include "../proto/embedding.pb.h"

namespace all_distance_sketch {

typedef std::vector<LABEL_WEIGHT> FEATURE_WEIGHTS_VECTOR;
typedef proto::EmbeddingGpb EmbeddingGpb;
typedef proto::NodeEmbeddingGpb NodeEmbeddingGpb;

class NodeFeature
{
public:
	NodeFeature() {}
	
	void InitNodeFeature(int node_id, const FEATURE_WEIGHTS_VECTOR& node_label) {
		node_id_ = node_id;
		labels_ = node_label;
	}
	
	int GetNId() const{
		return node_id_;
	}

	const FEATURE_WEIGHTS_VECTOR& GetFeatureWeights() {
		return labels_;
	}

	void SaveToGpb(NodeEmbeddingGpb* node_embedding) {
		node_embedding->set_node_id(node_id_);
		for (int i=0; i< labels_.size(); i++) {
			node_embedding->add_coordinate(labels_[i]);
		}
	}

private:
	int node_id_;
	FEATURE_WEIGHTS_VECTOR labels_;
};

struct compare_node_label_node_id_increasing {
  bool operator()(const NodeFeature& n1,
                  const NodeFeature& n2) const {
        return n1.GetNId() < n2.GetNId();
    }
};


typedef struct LabelRankInsertProb_t {
  double prob;
  int rank;
} LabelRankInsertProb;


class NodesFeaturesSortedContainer
{
public:
	NodesFeaturesSortedContainer() {}
	
	void AddNodeFeature(int node_id, const FEATURE_WEIGHTS_VECTOR& node_weight) {
		// Keep the seed nodes sorted according to the node id
		NodeFeature element_to_insert;
		element_to_insert.InitNodeFeature(node_id, node_weight);
		compare_node_label_node_id_increasing comp;
		nodes_labels_.insert(
					std::upper_bound(nodes_labels_.begin(), nodes_labels_.end(), element_to_insert, comp),
					element_to_insert
					);
	}

	const std::vector<NodeFeature>& GetNodesFeatures() {
		return nodes_labels_;
	}

	const FEATURE_WEIGHTS_VECTOR* GetSeedFeature(int node_id) {
		FEATURE_WEIGHTS_VECTOR empty_vec;
		NodeFeature element_to_find;
		element_to_find.InitNodeFeature(node_id, empty_vec);
		compare_node_label_node_id_increasing comp;
		auto it = std::lower_bound(nodes_labels_.begin(), nodes_labels_.end(), element_to_find, comp);
		if (it->GetNId() == node_id) {
			return &(it->GetFeatureWeights());
		}
		LOG_M(NOTICE, " was not able to find node in container, node id =" << node_id);
		return NULL;
	}

	int Size() {
		return nodes_labels_.size();
	}

private:
	std::vector<NodeFeature> nodes_labels_;
};


class NodesFeaturesContainer
{
public:
	NodesFeaturesContainer() {}
	
	void AddNodeFeature(int node_id, const FEATURE_WEIGHTS_VECTOR& node_weight) {
		// Keep the seed nodes sorted according to the node id
		NodeFeature element_to_insert;
		element_to_insert.InitNodeFeature(node_id, node_weight);
		nodes_labels_.push_back(element_to_insert);
	}

	const std::vector<NodeFeature>& GetNodesFeatures() {
		return nodes_labels_;
	}

	void SaveToGpb(EmbeddingGpb* embedding) {
		for (int i=0; i < nodes_labels_.size(); i++) {
			NodeEmbeddingGpb* node_embedding = embedding->add_nodes();
			nodes_labels_[i].SaveToGpb(node_embedding);
		}
	}

private:
	std::vector<NodeFeature> nodes_labels_;
};

}  //  all_distance_sketch
#endif  // SRC_GRAPH_LABELS_LABELS_H_