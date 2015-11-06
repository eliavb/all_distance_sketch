#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_GRAPH_SKETCH_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_GRAPH_SKETCH_H_

#include "node_sketch.h"

namespace all_distance_sketch {

typedef std::vector<NodeSketch> NodesSketch;

class GraphSketch {
 public:
  void InitGraphSketch(unsigned int K, int max_node_id,
                       UniformRankCalculator* calculator = NULL,
                       const std::vector<int>* nodes_id = NULL) {
    K_ = K;
    should_calc_z_value_ = false;
    nodes_ads_.resize(max_node_id);
    reserve_size_ = K_ * log2(max_node_id);
    prunning_thresholds_.resize(max_node_id);
    CreateNodesDistribution(max_node_id, calculator, nodes_id);
  }

  int GetK() const { return K_; }

  void set_should_calc_zvalues(bool should_calc) {
    should_calc_z_value_ = should_calc;
  }

#if PROTO_BUF
    bool LoadThresholdsAndRandomId(const AllDistanceSketchGpb& all_distance_sketch) {
        int num_nodes = all_distance_sketch.node_thresholds_size();
        prunning_thresholds_.resize(num_nodes);
        nodes_random_id_.resize(num_nodes);
        int node_id;
        for (int i=0; i < num_nodes; i++) {
            if (all_distance_sketch.node_thresholds(i).has_node_id() == false ||
                all_distance_sketch.node_thresholds(i).has_threshold() == false ||
                all_distance_sketch.node_thresholds(i).has_random_id() == false){
                LOG_M(NOTICE, "Load from bad file!!!, missing data " <<
                " node id ==  " << all_distance_sketch.node_thresholds(i).has_node_id() <<
                " threshold == " << all_distance_sketch.node_thresholds(i).has_threshold() <<
                " random id == " << all_distance_sketch.node_thresholds(i).has_random_id());
                return false;
            }
            node_id = all_distance_sketch.node_thresholds(i).node_id();
            if (node_id >= prunning_thresholds_.size() ||
                node_id >= nodes_random_id_.size()) {
              prunning_thresholds_.resize(node_id);
              nodes_random_id_.resize(node_id);
            }
            prunning_thresholds_[node_id].SetDistance(
                all_distance_sketch.node_thresholds(i).threshold());
            nodes_random_id_[node_id] =
                all_distance_sketch.node_thresholds(i).random_id();
        }
        ExtractSortedVersionFromDist();
        return true;
    }

    bool LoadNodesSketch(const AllDistanceSketchGpb& all_distance_sketch) {
        for (int i=0; i < all_distance_sketch.nodes_sketches_size(); i++) {
            if (all_distance_sketch.nodes_sketches(i).has_node_id() == false ||
                all_distance_sketch.nodes_sketches(i).has_node_random_id() == false) {
              LOG_M(NOTICE,
                    "Loading nodes NodeSketch, Load from bad file!!! missing "
                    "data "
                        << " node id == "
                        << all_distance_sketch.nodes_sketches(i).has_node_id()
                        << " random id == "
                        << all_distance_sketch.nodes_sketches(i)
                               .has_node_random_id());
                return false;

            }
            int node_id = all_distance_sketch.nodes_sketches(i).node_id();
            double node_random_id = all_distance_sketch.nodes_sketches(i).node_random_id();
            if (node_id >= nodes_ads_.size()) {
              nodes_ads_.resize(node_id + 1);
            }
            // Init the NodeSketch
            nodes_ads_[node_id].InitNodeSketch(K_, node_id, node_random_id,
                                               &prunning_thresholds_,
                                               reserve_size_);

            // Load All nodes in the sketch
            for (int k=0; 
                k < all_distance_sketch.nodes_sketches(i).node_in_sketch_size(); 
                k++) {
                const NodeSummaryDetailsGpb& node_in_sketch = \
                    all_distance_sketch.nodes_sketches(i).node_in_sketch(k);
                if (node_in_sketch.has_node_id() == false ||
                    node_in_sketch.has_distance() == false) {
                  LOG_M(
                      NOTICE,
                      "Loading nodes NodeSketch, Load from bad file!!! missing "
                      "data "
                          << " node id == " << node_in_sketch.has_node_id()
                          << " distance == " << node_in_sketch.has_distance());
                    return false;    
                }
                int sketch_node_id = node_in_sketch.node_id();
                graph::EdgeWeight distance = node_in_sketch.distance();
                NodeIdDistanceData node_in_sketch_details(sketch_node_id, distance);
                nodes_ads_[node_id].nodes_id_distance_.push_back(
                    node_in_sketch_details);
            }
        }
        return true;
    }

    bool LoadGraphSketchFromGpb(
        const AllDistanceSketchGpb& all_distance_sketch) {
        if (all_distance_sketch.has_k()) {
          K_ = all_distance_sketch.k();
        }
        // Load the thresholds
        int num_nodes = all_distance_sketch.node_thresholds_size();
        prunning_thresholds_.resize(num_nodes);
        nodes_random_id_.resize(num_nodes);
        if (LoadThresholdsAndRandomId(all_distance_sketch) == false) {
          LOG_M(NOTICE, "Unable to load threshold and random ID");
          return false;
        }
        reserve_size_ = K_ * log2(prunning_thresholds_.size());
        if (LoadNodesSketch(all_distance_sketch) == false) {
          LOG_M(NOTICE, "Unable to load nodes NodeSketch");
            return false;   
        }

        CalculateAllDistanceNeighborhood();
        return true;
    }

    void SaveThresholdAndRandomId(AllDistanceSketchGpb* all_distance_sketch) {
      if (nodes_random_id_.size() != prunning_thresholds_.size()) {
            LOG_M(NOTICE," PROBLEM threshold and random ids are not equal ");
            return;
        }
        for (int i = 0; i < prunning_thresholds_.size(); i++) {
          NodeThresholdGpb *node_threshold = all_distance_sketch->add_node_thresholds();
          node_threshold->set_node_id(i);
          node_threshold->set_threshold(prunning_thresholds_[i].GetDistance());
          node_threshold->set_random_id(nodes_random_id_[i]);
        }
    }

    void SaveNodesSketch(AllDistanceSketchGpb* all_distance_sketch) {
      for (int i = 0; i < nodes_random_id_.size(); i++) {
        if (nodes_ads_[i].IsInit() == false) {
            continue;
          }
          // Setting up the node details
          SingleNodeSketchGpb* single_node_sketch = all_distance_sketch->add_nodes_sketches();
          single_node_sketch->set_node_id(i);
          single_node_sketch->set_node_random_id(nodes_random_id_[i]);
          // Getting the node sketch
          const NodeIdDistanceVector* nodes_in_sketch =
              &(nodes_ads_[i].nodes_id_distance_);
          for (int j=0; j < nodes_in_sketch->size(); j++) {
            NodeSummaryDetailsGpb * node_details = single_node_sketch->add_node_in_sketch();
            node_details->set_node_id((*nodes_in_sketch)[j].GetNId());
            node_details->set_distance((*nodes_in_sketch)[j].GetDistance());
          }
        }
    }

    void SaveGraphSketchToGpb(AllDistanceSketchGpb* all_distance_sketch) {
      all_distance_sketch->set_k(K_);
      // Save the prunning_thresholds_ & random ids
        SaveThresholdAndRandomId(all_distance_sketch);
        // Save each node NodeSketch
        SaveNodesSketch(all_distance_sketch);
    }
#endif
    bool ShouldPrune(graph::EdgeWeight distance, int node_id) {
      if (distance > prunning_thresholds_[node_id].GetDistance()) {
          return false;
      }
      if (should_calc_z_value_) {
      return (distance == prunning_thresholds_[node_id].GetDistance() &&
              nodes_ads_[node_id].HasZValue(distance));
      }
      return distance >= prunning_thresholds_[node_id].GetDistance();
    }

    void CalculateInsertProb(int node_id, std::vector<NodeProb> * insert_prob) {
      nodes_ads_[node_id].CalculateInsertProb(&nodes_random_id_, insert_prob);
    }

    void SetPrunningThresholds() {
      for (unsigned int i = 0; i < nodes_ads_.size(); i++) {
        nodes_ads_[i].SetPrunningThresholds(&prunning_thresholds_);
        }
    }

    NodeSketch* UTGetNodeSketch(int node_id) { return &nodes_ads_[node_id]; }

    NodeSketch* GetNodeSketch(const NodeIdRandomIdData& node_details) {
      if (nodes_ads_.size() <= (unsigned int)node_details.GetNodeId()) {
            return NULL;
        }
        if (nodes_ads_[node_details.GetNodeId()].IsInit() == false) {
          nodes_ads_[node_details.GetNodeId()].InitNodeSketch(
              K_, node_details.GetNodeId(), node_details.GetRandomId(),
              &prunning_thresholds_, reserve_size_);
        }
        return &nodes_ads_[node_details.GetNodeId()];
    }

    void Printthresholds() {
      for (unsigned int i = 0; i < prunning_thresholds_.size(); i++) {
        std::cout << "i=" << i
                  << " thresholds=" << prunning_thresholds_[i].GetDistance()
                  << std::endl;
        }
    }

    NodeSketch* GetNodeSketch(NodeDistanceIdRandomIdData node_details) {
      return GetNodeSketch(node_details.GetDetails());
    }

    int InsertCandidatesNodes() {
        int numCleaned = 0;
        for (unsigned int i = 0; i < nodes_ads_.size(); i++) {
          numCleaned += nodes_ads_[i].InsertCandidatesNodes();
        }
        return numCleaned;
    }

    int InsertCandidatesNodes(unsigned int start, unsigned int end) {
        int numCleaned = 0;
        for (unsigned int i = start; (i < nodes_ads_.size() && i < end); i++) {
          numCleaned += nodes_ads_[i].InsertCandidatesNodes();
        }
        return numCleaned;
    }

    void CalculateAllDistanceNeighborhood() {
      for (unsigned int i = 0; i < nodes_ads_.size(); i++) {
        nodes_ads_[i].CalculateAllDistanceNeighborhood(&nodes_random_id_);
        }
    }

    const std::vector<NodeDistanceIdRandomIdData>* GetNodesDistribution()
        const {
      return &nodes_random_id_sorted_increasing_;
    }

    const std::vector<RandomId>* GetNodesDistributionLean() const {
      return &nodes_random_id_;
    }

    RandomId GetNodeRandomId(const int& node_id) {
      if ((unsigned int)node_id > nodes_random_id_.size()) {
            return 2;
        }
        return nodes_random_id_[node_id];
    }

    void SetNodesDistribution(const std::vector<RandomId>* nodes_random_id) {
      nodes_random_id_.clear();
      nodes_random_id_.resize(nodes_random_id->size());
      std::copy(nodes_random_id->begin(), nodes_random_id->end(),
                nodes_random_id_.begin());
      ExtractSortedVersionFromDist();
    }

    void CreateNodesDistribution(unsigned int max_node_id,
                                 UniformRankCalculator* calculator = NULL,
                                 const std::vector<int>* nodes_id = NULL) {
        UniformRankCalculator c;
        c.InitUniformRankCalculator();
        if (calculator == NULL ) {
          calculator = &c;
        }
        typedef boost::minstd_rand base_generator_type;
        base_generator_type generator(42u);
        boost::uniform_real<> uni_dist(0, 1);
        boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni(generator, uni_dist);
        nodes_random_id_.resize(max_node_id);
        int num_nodes = nodes_id == NULL ? max_node_id : nodes_id->size();
        for (unsigned int i = 0 ; i < num_nodes; i++) {
          int node_id = nodes_id == NULL ? i : (*nodes_id)[i];
          nodes_random_id_[node_id] =
              calculator->CalculateNodeRank(node_id);  // uni();
        }

        for (unsigned int i=0; i < num_nodes; i++) {
          int node_id = nodes_id == NULL ? i : (*nodes_id)[i];
          NodeDistanceIdRandomIdData b(0, node_id, nodes_random_id_[node_id]);
          nodes_random_id_sorted_increasing_.push_back(b);
        }
        std::sort(nodes_random_id_sorted_increasing_.begin(),
                  nodes_random_id_sorted_increasing_.end(),
                  compare_node_randomid_decreasing());
    }

    const std::vector<PrunningThreshold>* GetThresholds() const {
      return &prunning_thresholds_;
    }

    const NodesSketch& GetNodesSketch() const { return nodes_ads_; }

    bool operator==(const GraphSketch& other) const {
        if  (GetK() != other.GetK()) {
            LOG_M(NOTICE, "K is different!" <<
                          "lhs = " << GetK() << 
                          "rhs = " << other.GetK());
            return false;
        }
        if ((*GetNodesDistributionLean()) != (*other.GetNodesDistributionLean())) {
            LOG_M(NOTICE, "Distribution are not euqal!");
            return false;
        }

        if ( (*GetThresholds()) != (*other.GetThresholds())) {
            LOG_M(NOTICE, "thresholds are not equal!");
            return false;
        }

        if ((*GetNodesDistribution()) != (*GetNodesDistribution())) {
            LOG_M(NOTICE, "Distribution sorted are not equal!");
            return false;
        }

        if (GetNodesSketch() != other.GetNodesSketch()) {
          LOG_M(NOTICE, "Nodes NodeSketch are not equal!");
            return false;
        }
        return true;
    }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& K_;
      ar& nodes_ads_;
      ar& prunning_thresholds_;
      ar& nodes_random_id_;
      ar& nodes_random_id_sorted_increasing_;
      SetPrunningThresholds();
    }

 private:

    void ExtractSortedVersionFromDist() {
      nodes_random_id_sorted_increasing_.clear();
      for (unsigned int i = 0; i < nodes_random_id_.size(); i++) {
        NodeDistanceIdRandomIdData b(0, i, nodes_random_id_[i]);
        nodes_random_id_sorted_increasing_.push_back(b);
        }

        std::sort(nodes_random_id_sorted_increasing_.begin(),
                  nodes_random_id_sorted_increasing_.end(),
                  compare_node_randomid_decreasing());
    }

    friend class boost::serialization::access;
    bool should_calc_z_value_;
    unsigned int reserve_size_;
    unsigned int K_;
    NodesSketch nodes_ads_;
    std::vector<PrunningThreshold> prunning_thresholds_;
    std::vector<RandomId> nodes_random_id_;
    std::vector<NodeDistanceIdRandomIdData> nodes_random_id_sorted_increasing_;
};

}  //  namespace all_distance_sketch

#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_GRAPH_SKETCH_H_
