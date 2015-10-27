#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_NODE_SKETCH_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_NODE_SKETCH_H_

#include <math.h>
#include "../common.h"
#include "rank_calculator.h"
#if PROTO_BUF
#include "../../out/all_distance_sketch/proto/all_distance_sketch.pb.h"
#endif

namespace all_distance_sketch {

typedef double RandomId;

#if PROTO_BUF
typedef proto::AllDistanceSketchGpb AllDistanceSketchGpb;
typedef proto::AllDistanceSketchGpb::NodeThresholdGpb NodeThresholdGpb;
typedef proto::SingleNodeSketchGpb SingleNodeSketchGpb;
typedef proto::SingleNodeSketchGpb::NodeSummaryDetailsGpb NodeSummaryDetailsGpb;
#endif

class NodeIdDistanceData {
 public:
    NodeIdDistanceData() {};
    NodeIdDistanceData(int node_id, graph::EdgeWeight distance)
        : node_id_(node_id), distance_(distance) {}

    inline int GetNId() const { return node_id_; }

    inline graph::EdgeWeight GetDistance() const { return distance_; }

    bool operator==(const NodeIdDistanceData& other) const{
        return (GetNId() == other.GetNId()) && 
               (GetDistance() == other.GetDistance());
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const NodeIdDistanceData& node_details) {
      os << " NodeId=" << node_details.GetNId()
         << " Distance=" << node_details.GetDistance();
      return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar& node_id_;
      ar& distance_;
    }

 private:
    int node_id_;
    graph::EdgeWeight distance_;
};

class NodeIdRandomIdData {
 public:
  NodeIdRandomIdData() {}
  NodeIdRandomIdData(int node_id, RandomId random_id)
      : node_id_(node_id), random_id_(random_id) {}
  inline int GetNodeId() const { return node_id_; }

  inline RandomId GetRandomId() const { return random_id_; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar& node_id_;
      ar& random_id_;
    }

 private:
    int node_id_;
    RandomId random_id_;
};

class NodeDistanceIdRandomIdData {
 public:
  NodeDistanceIdRandomIdData() {}
  NodeDistanceIdRandomIdData(NodeIdRandomIdData aDetails,
                             graph::EdgeWeight distance)
      : details(aDetails), distance_(distance) {}

  NodeDistanceIdRandomIdData(graph::EdgeWeight distance, int node_id,
                             RandomId random_id)
      : details(node_id, random_id), distance_(distance) {}

  inline graph::EdgeWeight GetDistance() const { return distance_; }

  inline int GetNId() const { return details.GetNodeId(); }

  inline RandomId GetRandomId() const { return details.GetRandomId(); }

  inline const NodeIdRandomIdData& GetDetails() { return details; }

  bool operator==(const NodeDistanceIdRandomIdData& other) const {
        return (GetNId() == other.GetNId()) &&
               (GetRandomId() == other.GetRandomId()) &&
               (GetDistance() == other.GetDistance());
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const NodeDistanceIdRandomIdData& node_details) {
          os << " NodeId=" << node_details.GetNId()
             << " Distance=" << node_details.GetDistance()
             << " HashId=" << node_details.GetRandomId();
      return os;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& details;
      ar& distance_;
    }

 private:
    NodeIdRandomIdData details;
    graph::EdgeWeight distance_;
};

struct compare_node_randomid_decreasing {
  bool operator()(const NodeDistanceIdRandomIdData& n1,
                  const NodeDistanceIdRandomIdData& n2) const {
        return n1.GetRandomId() < n2.GetRandomId();
    }
};

struct compare_node_distance_increasing {
  bool operator()(const NodeIdDistanceData& n1,
                  const NodeIdDistanceData& n2) const {
        return n1.GetDistance() > n2.GetDistance();
    }
};

struct compare_node_distance_decreasing {
  bool operator()(const NodeIdDistanceData& n1,
                  const NodeIdDistanceData& n2) const {
        return n1.GetDistance() < n2.GetDistance();
    }
};

class Neighbourhood {
 public:
  Neighbourhood() {}
  Neighbourhood(int distance, int size) : distance_(distance), size_(size) {}

  int GetSize() const { return size_; }

  int GetDistance() const { return distance_; }

  bool operator==(const Neighbourhood& other) const {
    return (GetSize() == other.GetSize()) &&
           (GetDistance() == other.GetDistance());
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar& distance_;
      ar& size_;
    }

 private:
    int distance_;
    int size_;
};

struct compare_neighbourhood_distance {
  bool operator()(const Neighbourhood& n1, const Neighbourhood& n2) const {
        return n1.GetDistance() < n2.GetDistance();
    }
};

struct compare_neighbourhood_size {
  bool operator()(const Neighbourhood& n1, const Neighbourhood& n2) const {
        return n1.GetSize() < n2.GetSize();
    }
};

class PrunningThreshold {
 public:
  PrunningThreshold()
      : is_distance_equal_to_following_node_distance_in_ads(false),
        distance_(constants::UNREACHABLE) {}

  graph::EdgeWeight GetDistance() const { return distance_; }

  void SetDistance(graph::EdgeWeight distance) { distance_ = distance; }

  bool GetIsEqualToNext() const {
    return is_distance_equal_to_following_node_distance_in_ads;
  }

  void SetIsEqualToNext(bool aIsEqual) {
    is_distance_equal_to_following_node_distance_in_ads = aIsEqual;
  }
  bool operator==(const PrunningThreshold& other) const {
    return GetDistance() == other.GetDistance();
  }

 private:
  bool is_distance_equal_to_following_node_distance_in_ads;
  graph::EdgeWeight distance_;
};

typedef struct NodeProb_t {
  double prob;
  int node_id;
} NodeProb;

typedef std::vector<Neighbourhood> NeighbourhoodVector;
typedef std::unordered_map<int, NodeIdDistanceData> ZValues;
typedef std::vector<NodeIdDistanceData> NodeIdDistanceVector;
typedef std::vector<NodeIdDistanceData>::iterator NodeIdDistanceVectorItr;
typedef std::vector<NodeDistanceIdRandomIdData>
    NodeDistanceIdRandomIdDataVector;
typedef std::vector<NodeDistanceIdRandomIdData>::iterator
    NodeDistanceIdRandomIdDataVectorItr;

class NodeSketch {
 public:
  NodeSketch() { was_init_ = false; }

  void InitNodeSketch(
      int K, int node_id, RandomId random_id,
      std::vector<PrunningThreshold>* prunning_thresholds = NULL,
      unsigned int reserve_size = 1) {
    K_ = K;
    node_id_ = node_id;
    random_id_ = random_id;
    was_init_ = true;
    nodes_id_distance_.reserve(K_);
    prunning_thresholds_ = prunning_thresholds;
    }

    bool IsInit() const { return was_init_; }

    bool AddToCandidates(NodeDistanceIdRandomIdData node_details) {
      candidate_nodes_.push_back(node_details);
        return true;
    }

    int InsertCandidatesNodes() {
      compare_node_randomid_decreasing obj;
      sort(candidate_nodes_.begin(), candidate_nodes_.end(), obj);
        // Sort according to random ID.
        for (unsigned int i = 0; i < candidate_nodes_.size(); i++) {
          Add(candidate_nodes_[i]);
        }
        int numInserted = candidate_nodes_.size();
        candidate_nodes_.clear();
        return numInserted;
    }

    bool Add(NodeDistanceIdRandomIdData node_details) {
      NodeIdDistanceData node_id_distance(node_details.GetNId(),
                                          node_details.GetDistance());
      return Add(node_id_distance);
    }

    void ShouldInsert(NodeIdDistanceData node_details, bool* should_insert,
                      bool* is_zvalue) {
      NodeIdDistanceVector::reverse_iterator it_k =
          nodes_id_distance_.rbegin() + (K_ - 1);
      if (node_details.GetDistance() > it_k->GetDistance()) {
        (*is_zvalue) = false;
        (*should_insert) = false;
            return;
        }
        if (node_details.GetDistance() == it_k->GetDistance()) {
          if (z_values_.find(node_details.GetDistance()) != z_values_.end()) {
            (*should_insert) = false;
            (*is_zvalue) = false;
            } else {
              (*should_insert) = false;
              (*is_zvalue) = true;
            }
            return;
        }
        (*should_insert) = true;
        (*is_zvalue) = false;
        return;
    }

    bool Add(NodeIdDistanceData node_details) {
      //  If the NodeSketch size is smaller than K we will always add

      if (nodes_id_distance_.size() < K_) {
        LOG_M(DEBUG4, " Adding to NodeSketch since size < k, "
                          << " size=" << nodes_id_distance_.size()
                          << " k=" << K_);
        // nodes_id_distance_.push_back(node_details);
            compare_node_distance_increasing obj;
            NodeIdDistanceVectorItr up =
                std::upper_bound(nodes_id_distance_.begin(),
                                 nodes_id_distance_.end(), node_details, obj);
            nodes_id_distance_.insert(up, node_details);
            return true;
        }

        // Are you Z value?
        bool is_z_value;
        bool should_insert;
        ShouldInsert(node_details, &should_insert, &is_z_value);

        if (is_z_value) {
          z_values_.insert(
              std::make_pair(node_details.GetDistance(), node_details));
            return false;
        }

        if (!should_insert) {
            return false;
        }

        // Size of the vector is bigger than K
        // The vector is sorted according to distance
        // Checking if distnace is inside the possible distance
        compare_node_distance_decreasing obj;

        NodeIdDistanceVector::reverse_iterator up = std::upper_bound(
            nodes_id_distance_.rbegin(), nodes_id_distance_.rbegin() + K_,
            node_details, obj);
        unsigned int position_upper = up - nodes_id_distance_.rbegin();

        LOG_M(DEBUG4, " Size of array "
                          << nodes_id_distance_.size()
                          << " upper position=" << position_upper
                          << " up=" << *(up) << " First element "
                          << nodes_id_distance_[0] << " Last element "
                          << nodes_id_distance_[nodes_id_distance_.size() - 1]);

        /*
        if (position_upper > K_) {
            LOG_M(NOTICE," Distance of node=" << node_details.GetDistance() <<
                         " K threshold=" << (nodes_id_distance_.rbegin() + (K_ -
        1))->GetDistance());
            return false;
        }
        */

        LOG_M(DEBUG5, "Before inserting");

        if (position_upper <= K_) {
          unsigned int position_relative_to_begin =
              nodes_id_distance_.rend() - up;
          NodeIdDistanceVectorItr it;
          if (position_relative_to_begin == nodes_id_distance_.size()) {
            it = nodes_id_distance_.end();
            } else {
              it = nodes_id_distance_.begin() + position_relative_to_begin;
            }
            nodes_id_distance_.insert(it, node_details);
            LOG_M(DEBUG4, " Inserting to NodeSketch at location "
                              << position_relative_to_begin);
            if (prunning_thresholds_ != NULL) {
              (*prunning_thresholds_)[node_id_].SetDistance(
                  (nodes_id_distance_.rbegin() + (K_ - 1))->GetDistance());
              if (nodes_id_distance_.size() >= K_) {
                bool is_distance_equal_to_following_node_distance_in_ads =
                    ((nodes_id_distance_.rbegin() + (K_ - 1))->GetDistance() ==
                     (nodes_id_distance_.rbegin() + (K_))->GetDistance());
                (*prunning_thresholds_)[node_id_].SetIsEqualToNext(
                    is_distance_equal_to_following_node_distance_in_ads);
                }
            }

            return true;
        }

        return false;
    }

    void Get(graph::EdgeWeight distance,
             NodeIdDistanceVector* nodes_id_distance_vector) {
      nodes_id_distance_vector->clear();
      for (NodeIdDistanceVector::reverse_iterator iter =
               nodes_id_distance_.rbegin();
           iter != nodes_id_distance_.rend(); iter++) {
        if (iter->GetDistance() <= distance) {
                LOG_M(DEBUG5, "Inserting to vector node " << *iter);
                nodes_id_distance_vector->push_back(*(iter));
            }
        }
    }

    void GetAllDistances(NodeIdDistanceVector* nodes_id_distance_vector) {
      nodes_id_distance_vector->clear();
      *nodes_id_distance_vector = nodes_id_distance_;
    }

    int GetSketchSize() { return nodes_id_distance_.size(); }
    /*
    * What is the distance such that all |nodes < distance| >= neighborhood_size
    */
    double GetDistanceCoverNeighborhood(int neighborhood_size) {
      compare_neighbourhood_size obj;
      Neighbourhood entry(0, neighborhood_size);
      NeighbourhoodVector::iterator up = std::upper_bound(
          neighbourhoods_.begin(), neighbourhoods_.end(), entry, obj);
      
      if (neighbourhoods_.size() == 0) {
        LOG_M(DEBUG3, "neighbourhoods vector is empty, returning 0");
        return 0;
      }
      if (up == neighbourhoods_.begin()) {
          LOG_M(DEBUG3, "Match first element");
          if (up->GetSize() == neighbourhoods_.begin()->GetSize()) {
              return up->GetDistance();
          }
          return 0;
      }
      if (up == neighbourhoods_.end()) {
        LOG_M(DEBUG3, "Match last element");
        return neighbourhoods_.back().GetDistance() + 1;
      }
      LOG_M(DEBUG3, " Distance= " << up->GetDistance() <<
                    " Size=" << up->GetSize() <<
                    " Wanted size=" << neighborhood_size);
      return up->GetDistance();
    }
    /*
     * Gets the first distance index such that the distance is <= distance
     */
    int GetNeighborhoodDistanceIndex(graph::EdgeWeight distance) {
        compare_neighbourhood_distance obj;
        Neighbourhood entry(distance, 0);
        NeighbourhoodVector::iterator up = std::upper_bound(
            neighbourhoods_.begin(), neighbourhoods_.end(), entry, obj);
        if (neighbourhoods_.size() == 0) {
          return -1;
        }
        if (up == neighbourhoods_.begin()) {
            if (up->GetDistance() == neighbourhoods_.begin()->GetDistance()) {
                return 0;
            }
            // The distance is smaller than the smallest distance
            return -1;
        }
        if (up == neighbourhoods_.end()) {
          return neighbourhoods_.size() - 1;
        }
        if (up->GetDistance() > distance) {
            up -= 1;
        }
        return up - neighbourhoods_.begin();
    }

    // Case fail -1
    double GetNeighborhoodDistanceByIndex(int index) {
        if (index < 0) {
          return 0;
        }
        if (index >= neighbourhoods_.size()) {
          return neighbourhoods_.back().GetSize();
        }
        return (neighbourhoods_.begin() + index)->GetSize();
    }

    /*
     *  Return the size of the neighborhood <= distance
     */
    int GetSizeNeighborhoodUpToDistance(
        graph::EdgeWeight distance,
        const std::vector<RandomId>* node_distribution) {
        compare_neighbourhood_distance obj;
        Neighbourhood entry(distance, 0);
        NeighbourhoodVector::iterator up = std::upper_bound(
            neighbourhoods_.begin(), neighbourhoods_.end(), entry, obj);

        if (up == neighbourhoods_.begin()) {
            LOG_M(DEBUG3, "Distance is in the begin of the vector" <<
                          " Distance= " << up->GetDistance() <<
                          " Size=" << up->GetSize());
            if (up->GetDistance() == neighbourhoods_.begin()->GetDistance()) {
                return up->GetSize();
            }
            // The distance is smaller than the smallest distance
            return 0;
        }

        if (up == neighbourhoods_.end()) {
          LOG_M(DEBUG3, "Distance is in the end of the vector" <<
                        " Size=" << neighbourhoods_.back().GetSize());
          return neighbourhoods_.back().GetSize();
        }
        if (up->GetDistance() > distance) {
            up -= 1;
        }
        LOG_M(DEBUG3, "Distance is in the midddle of the vector" <<
                         " Distance= " << up->GetDistance() <<
                         " Size=" << up->GetSize());
        return up->GetSize();
    }
    /*
     * Calculate for each distance the neighborhood size.
     */
    void CalculateAllDistanceNeighborhood(const std::vector<RandomId>* node_distribution) {
        neighbourhoods_.clear();
        if (nodes_id_distance_.size() == 0) {
            return;
        }

        std::vector<NodeDistanceIdRandomIdData> neighborhoodVector;
        NodeIdDistanceVector::reverse_iterator it;
        neighbourhoods_.clear();
        unsigned int currentDistance = 0;
        compare_node_randomid_decreasing obj;
        for (it = nodes_id_distance_.rbegin(); it != nodes_id_distance_.rend();
             it++) {
            LOG_M(DEBUG3, "Iterting " << *it << " Current distance=" << currentDistance);
            if (it->GetDistance() != currentDistance) {
                LOG_M(DEBUG3, " Changing distance to " << it->GetDistance());
                // Calculate neighbour
                int size = 0;
                if (neighborhoodVector.size() < K_) {
                    if (neighborhoodVector.size() == 0) {
                        size = 0;
                    } else {
                        size = neighborhoodVector.size() - 1;
                    }
                } else {
                  RandomId omega = neighborhoodVector[K_ - 1].GetRandomId();
                  size = (K_ - 1) / omega;
                }
                LOG_M(DEBUG3, " Size of neighborhood " << size);
                Neighbourhood entry(currentDistance, size);
                neighbourhoods_.push_back(entry);
                currentDistance = it->GetDistance();
            }

            NodeDistanceIdRandomIdData t(it->GetDistance(), it->GetNId(),
                                         (*node_distribution)[it->GetNId()]);
            std::vector<NodeDistanceIdRandomIdData>::iterator up =
                std::upper_bound(neighborhoodVector.begin(),
                                 neighborhoodVector.end(), t, obj);
            neighborhoodVector.insert(up, t);
        }

        // Another iteration for the last element
        int size;
        if (neighborhoodVector.size() < K_) {
            if (neighborhoodVector.size() == 0) {
                size = 0;
            } else {
                size = neighborhoodVector.size() - 1;    
            }
            
        } else {
          RandomId omega = neighborhoodVector[K_ - 1].GetRandomId();
          size = (K_ - 1) / omega;
        }
        Neighbourhood entry(nodes_id_distance_.begin()->GetDistance(), size);
        neighbourhoods_.push_back(entry);
    }

    NeighbourhoodVector* UTGetNeighbourhoodVector() { return &neighbourhoods_; }

    NodeIdDistanceVector* UTGetNodeAdsVector() { return &nodes_id_distance_; }
    void SetPrunningThresholds(std::vector<PrunningThreshold>* athresholds) {
      prunning_thresholds_ = athresholds;
    }
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar& was_init_;
      ar& K_;
      ar& node_id_;
      ar& random_id_;
      ar& nodes_id_distance_;
      ar& candidate_nodes_;
      ar& neighbourhoods_;
    }

    const NodeIdDistanceVector* GetNodeAdsVector() const {
      return &nodes_id_distance_;
    }

    int GetK() const { return K_; }

    int GetNId() const { return node_id_; }

    RandomId GetRandomId() const { return random_id_; }

    const NeighbourhoodVector& GetNeighbourHoodVector() const {
      return neighbourhoods_;
    }

    const std::vector<NodeDistanceIdRandomIdData>& GetCandidates() const {
      return candidate_nodes_;
    }

    bool operator==(const NodeSketch& other) const {
        if (IsInit() == false && other.IsInit() == false) {
            return true;
        }
        if (GetK() != other.GetK()) {
            LOG_M(NOTICE, " K is not euqal!" <<
                          " lhs K = " << GetK() <<
                          " rhs K = " << other.GetK());
            return false;
        }

        if (GetNId() != other.GetNId()) {
            LOG_M(NOTICE, " Node id is not equal!");
            return false;
        }

        if (GetRandomId() != other.GetRandomId()) {
            LOG_M(NOTICE, " RandomId is not equal!");
        }
        if (*GetNodeAdsVector() != (*other.GetNodeAdsVector())) {
            LOG_M(NOTICE, " Node Sketches are not equal!");
            return false;
        }
        if (GetNeighbourHoodVector() != other.GetNeighbourHoodVector()) {
            LOG_M(NOTICE, " NeighbourHood Vector are not equal!");
            return false;
        }
        if (GetCandidates() != other.GetCandidates()) {
            LOG_M(NOTICE, " Candidates Vector are not equal!");
            return false;
        }
        return true;
    }

    const ZValues& GetZValues() { return z_values_; }

    bool HasZValue(graph::EdgeWeight distance) {
      return z_values_.find(distance) != z_values_.end();
    }

    void CalculateInsertProb(const std::vector<RandomId>* node_distribution,
                             std::vector<NodeProb>* insert_prob) {
      // Easy case - All nodes insert thus the threshold is 1 for all
      insert_prob->clear();
      if (nodes_id_distance_.size() < K_) {
        for (int i = 0; i < nodes_id_distance_.size(); i++) {
          NodeProb node_thresh_prob;
          node_thresh_prob.prob = 1;
          node_thresh_prob.node_id = nodes_id_distance_[i].GetNId();
          insert_prob->push_back(node_thresh_prob);
        }
        return;
      }
      // Any node below this distance gets automatic entrence thus prob == 1
      std::vector<NodeDistanceIdRandomIdData> moving_threshold;
      graph::EdgeWeight distance_covered = -1;
      compare_node_randomid_decreasing comparator;
      for (NodeIdDistanceVector::reverse_iterator r_it =
               nodes_id_distance_.rbegin();
           r_it < nodes_id_distance_.rend(); r_it++) {
        LOG_M(DEBUG3,
              "id:" << r_it->GetNId() << " distane:" << r_it->GetDistance()
                    << " random id: " << (*node_distribution)[r_it->GetNId()]);

          if (distance_covered != r_it->GetDistance()) {
            for (NodeIdDistanceVector::reverse_iterator runner_it = r_it;
                 (runner_it->GetDistance() == r_it->GetDistance()) &&
                 runner_it != nodes_id_distance_.rend();
                 runner_it++) {
              NodeDistanceIdRandomIdData node_details(
                  runner_it->GetDistance(), runner_it->GetNId(),
                  (*node_distribution)[runner_it->GetNId()]);

                  moving_threshold.insert(std::upper_bound(moving_threshold.begin(),
                                                           moving_threshold.end(),
                                                           node_details,
                                                           comparator),
                                          node_details);
              }
              // Is there Z value?
              ZValues::iterator z_it = z_values_.find(r_it->GetDistance());
              if (z_it != z_values_.end()) {
                NodeDistanceIdRandomIdData node_details(
                    z_it->second.GetDistance(), z_it->second.GetNId(),
                    (*node_distribution)[z_it->second.GetNId()]);

                moving_threshold.insert(std::upper_bound(moving_threshold.begin(),
                                                       moving_threshold.end(),
                                                       node_details,
                                                       comparator),
                                        node_details);
              }

              distance_covered = r_it->GetDistance();
          }
          NodeProb node_thresh_prob;
          node_thresh_prob.node_id = r_it->GetNId();
          if (moving_threshold.size() <= K_) {
            LOG_M(DEBUG3, "node id:" << r_it->GetNId() << " prob: 1");
            node_thresh_prob.prob = 1;
          } else {
            std::vector<NodeDistanceIdRandomIdData>::iterator it_k_plus_1 =
                moving_threshold.begin() + K_;
            if (node_thresh_prob.node_id == it_k_plus_1->GetNId()) {
                // This is Z value! not one of the K smallest!
                // We don't want to consider
                LOG_M(DEBUG3, " node id:" << r_it->GetNId() << " Skip Z value");
                continue;
            }
            node_thresh_prob.prob = it_k_plus_1->GetRandomId();
          }
          insert_prob->push_back(node_thresh_prob);
      }
      std::reverse(insert_prob->begin(), insert_prob->end());
    }

 private:
    friend class boost::serialization::access;
    friend class GraphSketch;
    bool was_init_;
    unsigned int K_;
    int node_id_;
    RandomId random_id_;
    NodeIdDistanceVector nodes_id_distance_;
    std::vector<NodeDistanceIdRandomIdData> candidate_nodes_;
    std::vector<PrunningThreshold>* prunning_thresholds_;
    NeighbourhoodVector neighbourhoods_;
    ZValues z_values_;
};


}  // namespace all_distance_sketch

#endif  // ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_NODE_SKETCH_H_
