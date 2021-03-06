#ifndef SRC_GRAPH_GRAPH_H_
#define SRC_GRAPH_GRAPH_H_

#include "../common.h"
#include "../utils/utils.h"

/*! \file graph.h
    \brief Graph class
*/

namespace all_distance_sketch {
namespace graph {

struct EdgeDetails {
    EdgeDetails(EdgeWeight edge_weight, int node_id) :
      edge_weight_(edge_weight), node_id_(node_id) {}

    inline bool operator<(const EdgeDetails other) const {
        return other.node_id_ > node_id_;
    }

    inline int get_node_id() const { return node_id_; }
    inline EdgeWeight get_edge_weight() const { return edge_weight_; }
    inline void set_edge_weight(EdgeWeight weight) { edge_weight_ = weight; }
    EdgeWeight edge_weight_;
    int node_id_;
};

typedef std::vector<EdgeDetails> TEdgesWeights;
typedef std::vector<TEdgesWeights> TWeights;

struct WeightMap {
    void SetDirected() { is_directed_ = true;}
    void SetUnDirected() { is_directed_ = false;}

    bool AddEdge(int source_node_id, int dest_node_id, EdgeWeight weight) {
        if (is_directed_) {
            return _AddEdge(source_node_id, dest_node_id, weight);
        } else {
            return (_AddEdge(source_node_id, dest_node_id, weight) || _AddEdge(dest_node_id, source_node_id, weight));
        }
    }

    bool _AddEdge(int source_node_id, int dest_node_id, EdgeWeight weight) {
        if (weights_.size() <= (unsigned int)source_node_id) {
            weights_.resize(source_node_id + 1);
        }

        EdgeDetails details(weight, dest_node_id);
        TEdgesWeights::iterator it = \
        std::lower_bound( weights_[source_node_id].begin(), weights_[source_node_id].end(), details);
        if (it != weights_[source_node_id].end() && it->get_node_id() == dest_node_id) {
            it->set_edge_weight(weight);
            return true;
        }

        weights_[source_node_id].insert(it, details);
        return false;
    }

    TEdgesWeights * GetNodeWeights(int aNode) {
      if (aNode >= weights_.size()) {
        return NULL;
      }
      return &(weights_[aNode]);
    }

    bool GetEdgeWeight(int source_node_id, int dest_node_id, EdgeWeight * result_edge_weight) {
        int temp_node_1 = source_node_id;
        int temp_node_2 = dest_node_id;
        if (is_directed_ == false) {
            temp_node_1 = source_node_id < dest_node_id ? source_node_id : dest_node_id;
            temp_node_2 = source_node_id > dest_node_id ? source_node_id : dest_node_id;
        }

        source_node_id = temp_node_1;
        dest_node_id = temp_node_2;
        if (weights_.size() <= (unsigned int)source_node_id) {
            *result_edge_weight = constants::UNREACHABLE;
            return false;
        }

        EdgeDetails details(0, dest_node_id);
        TEdgesWeights::iterator it = std::lower_bound( weights_[source_node_id].begin(), weights_[source_node_id].end(), details);
        if (it != weights_[source_node_id].end() && it->get_node_id() == dest_node_id) {
            *result_edge_weight = it->get_edge_weight();
            return true;
        }

        *result_edge_weight = constants::UNREACHABLE;
        return false;
    }
 public:
    TWeights weights_;
    bool is_directed_;
};

template <class K>
struct GraphTrait {
    static const bool directed = false;
};

/*! \brief Graph data structure
    Thin wrapper over SNAP graph
*/
template <class T>
class Graph {
 public:
    Graph() {
        if (GraphTrait<T>::directed){
            weight_map_.SetDirected();
        } else {
            weight_map_.SetUnDirected();
        }
    }

    int AddNode(int node_id) {
        return graph_.AddNode(node_id);
    }

    int AddEdge(const int& aSrcNId, const int& aDstNId, double weight = 1) {
        int edgeId = graph_.AddEdge(aSrcNId, aDstNId);
        if (edgeId == -1) {
            LOG_M(DEBUG5, "Inserting edge, source node=" << aSrcNId
                        << " dest node=" << aDstNId
                        << " weight=" << weight);
            weight_map_.AddEdge(aSrcNId, aDstNId, weight);
        }
        return edgeId;
    }

    std::pair<bool, EdgeWeight> GetEdgeWeight(const int& aSrcNId, const int& aDstNId) {
        bool wasFound = false;
        EdgeWeight w = constants::UNREACHABLE;
        wasFound = weight_map_.GetEdgeWeight(aSrcNId, aDstNId, &w);
        std::pair<bool, EdgeWeight> result(wasFound, w);
        return result;
    }

    TEdgesWeights * GetNodeWeights(int aNode) {
        return weight_map_.GetNodeWeights(aNode);
    }

    typename T::TNodeI BegNI() {
        return graph_.BegNI();
    }

    typename T::TNodeI EndNI() {
        return graph_.EndNI();
    }

    bool IsEdge(const int& aSrcNId, const int& aDstNId) const {
        return graph_.IsEdge(aSrcNId, aDstNId);
    }

    bool IsNode(const int& node_id) const {
        return graph_.IsNode(node_id);
    }

    int GetNumNodes() const {
        return graph_.GetNodes();
    }

    int GetNumEdges() const {
        return graph_.GetEdges();
    }
    /*! \cond
    */
    WeightMap * utGeWeightMap() {
        return &weight_map_;
    }
    /*! \endcond
    */
    typename T::TNodeI GetNI(const int& node_id) const {
        return graph_.GetNI(node_id);
    }

    int GetMxNId() const {
        return graph_.GetMxNId();
    }
    /*! \brief Changes the direction of all edges
        For undirected graph this has no effect.
    */
    void Transpose(Graph<T> * transpose) {
      for (auto node_itr = this->BegNI(); node_itr != this->EndNI() /* node_itr.HasMore()*/ ; node_itr++) {
        transpose->AddNode(node_itr.GetId());
      }
      for (int node_id = 0; node_id < weight_map_.weights_.size(); ++node_id) {
        for (auto node_it = weight_map_.weights_[node_id].begin(); node_it != weight_map_.weights_[node_id].end(); ++node_it){
          transpose->AddEdge(node_it->get_node_id() , node_id, node_it->get_edge_weight());
        }
      }
    }
    /*! \brief Loads the graph from files located in a dir
        \param[in] aPath -  path to dir. The directory should contain a file with .txt suffix that contains
                            pairs of nodes in each line indicating an edge between the nodes, e.g.
                            
                            1   2
                            
                            1   268635
                            
                            The above example will create the nodes 1, 2, 268635 and edges (1, 2) and (1, 26835)
                            If you have nodes that have no edges to other nodes then insert them manually.
        \param[in] aTranspose - should load the graph with reverse edges.
                                If set to True the above example will insert (2, 1) and (268635, 1).
    */
    void LoadGraphFromDir(std::string aPath, bool aTranspose = false) {
        utils::FileUtils::NodePairList nodePairList;
        utils::FileUtils::GetNodePairListFromDir(aPath, &nodePairList);
        for (utils::FileUtils::NodePairList::iterator itr = nodePairList.begin(); itr != nodePairList.end(); ++itr) {
            double w = std::get<2>(*itr);
            int uNodeId = std::get<0>(*itr);
            LOG_M(DEBUG5, "Found node " << uNodeId);
            if (IsNode(uNodeId) == false) {
                LOG_M(DEBUG5, "inserting node " << uNodeId << " it does not exists");
                AddNode(uNodeId);
            }
            int vNodeId = std::get<1>(*itr);
            LOG_M(DEBUG5, "Found node " << vNodeId);
            if (IsNode(vNodeId) == false) {
                LOG_M(DEBUG5, "Inserting node " << vNodeId << " it does not exists");
                AddNode(vNodeId);
            }
            LOG_M(DEBUG5, "Node src " << uNodeId << " Node dest " << vNodeId);
            if (aTranspose == false) {
                AddEdge(uNodeId, vNodeId, w);
            } else {
                AddEdge(vNodeId, uNodeId, w);
            }
        }
    }

    void LoadGraphFromFile(std::string file_name, std::string delimiter, bool aTranspose = false) {
        utils::FileUtils::NodePairList nodePairList;
        LOG_M(NOTICE, "File name=" << file_name);
        utils::FileUtils::GetNodePairListFromFile(file_name, &nodePairList, delimiter);
        for (utils::FileUtils::NodePairList::iterator itr = nodePairList.begin(); itr != nodePairList.end(); ++itr) {
            double w = std::get<2>(*itr);
            int uNodeId = std::get<0>(*itr);
            LOG_M(DEBUG5, "Found node " << uNodeId);
            if (IsNode(uNodeId) == false) {
                LOG_M(DEBUG5, "inserting node " << uNodeId << " it does not exists");
                AddNode(uNodeId);
            }
            int vNodeId = std::get<1>(*itr);
            LOG_M(DEBUG5, "Found node " << vNodeId);
            if (IsNode(vNodeId) == false) {
                LOG_M(DEBUG5, "Inserting node " << vNodeId << " it does not exists");
                AddNode(vNodeId);
            }
            LOG_M(DEBUG1, "Node src " << uNodeId << " Node dest " << vNodeId);
            if (aTranspose == false) {
                AddEdge(uNodeId, vNodeId, w);
            } else {
                AddEdge(vNodeId, uNodeId, w);
            }
        }
    }

    /*! \cond
    */
    WeightMap weight_map_;
    T graph_;
    /*! \endcond
    */
};

}  // namespace graph

/** \example examples/graph.cpp
 * Examples on how to use graph::Graph class.
 */
}  // namespace all_distance_sketch

#endif  //  SRC_GRAPH_GRAPH_H_
