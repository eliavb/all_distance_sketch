#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_LEMON_GRAPH_ADAPTOR_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_LEMON_GRAPH_ADAPTOR_H_

#include <lemon/smart_graph.h>
#include "graph.h"

namespace all_distance_sketch {
namespace graph {

typedef std::vector<bool> NodeList;
typedef std::vector<int> DegVector;
typedef std::unordered_map<int, NodeList > EdgesList;
typedef std::unordered_map<int, int>  ConvertionMap;


template <class T>
class GenericGraphBaseAdaptor {
 public:

 class GenericIterator {
 public:
    GenericIterator(const T* g, DegVector * _deg_v) : m_graph(g), nodeIt((*g)), out_neighbor_index(0), _e((*m_graph), nodeIt), _deg_vector(_deg_v) { } 
    
    GenericIterator(const T* g, int aNId, const DegVector * _deg_v) : m_graph(g) { 
      typename T::Node n = g->nodeFromId(aNId);
      typename T::NodeIt _nodeIt((*g), n);
      nodeIt = _nodeIt;
      out_neighbor_index = 0;
      typename T::OutArcIt e((*m_graph), nodeIt);
      _e = e;
      _deg_vector = _deg_v;
    }

    int GetId() const {
      return m_graph->id(nodeIt); // parent->ConvertIntToExt(m_graph->id(nodeIt));
    }

    int GetInDeg() const {
      return lemon::countInArcs((*m_graph), nodeIt) + lemon::countOutArcs((*m_graph), nodeIt);
    }

    int GetDeg() const {
      return lemon::countInArcs((*m_graph), nodeIt) + lemon::countOutArcs((*m_graph), nodeIt);
    }

    int GetNbrNId(int n) {
      int cnt = 0;
      for (typename T::OutArcIt e((*m_graph), nodeIt); e != lemon::INVALID; ++e) {
        if (cnt == n) {
          return _GetOther(&e);
        }
        cnt++;
      }

      for (typename T::InArcIt e((*m_graph), nodeIt); e != lemon::INVALID; ++e) {
        if (cnt == n) {
          return _GetOther(&e);
        }
        cnt++;
      }
      return -1;
    }

    int GetOutDeg() const {
      if (m_graph->id(nodeIt) >= _deg_vector->size()) {
        return 0;
      }
      return (*_deg_vector)[m_graph->id(nodeIt)];
      //return lemon::countOutArcs((*m_graph), nodeIt);
    }

    int _GetOther(typename T::OutArcIt* e) const {
      int source = m_graph->id(m_graph->source(*e));
      int target = m_graph->id(m_graph->target(*e));
      source = ( (source == GetId()) ? target : source);
      return source; // parent->ConvertIntToExt(source);
    }

    int _GetOther(typename T::InArcIt* e) const {
      int source = m_graph->id(m_graph->source(*e));
      int target = m_graph->id(m_graph->target(*e));
      source = ( (source == GetId()) ? target : source);
      return source; // parent->ConvertIntToExt(source);
    }

    int GetOutNId(int n) {
      if (n == out_neighbor_index) {
        int _id = _GetOther(&_e);
        ++out_neighbor_index;
        ++_e;
        return _id;
      }

      int cnt = 0;
      int node_id = -1;
      for (typename T::OutArcIt e((*m_graph), nodeIt); e != lemon::INVALID; ++e) {
        if (cnt == n) {
          node_id = _GetOther(&e);
          break;
        }
        cnt++;
      }
      
      return node_id;
    }

    bool HasMore() {
      return nodeIt != lemon::INVALID;
    }

    GenericIterator& operator++(int) {
      ++nodeIt;
      return *this;
    }

    inline bool operator==(const GenericIterator& rhs) {
      return (nodeIt == rhs.nodeIt);
    }

    inline bool operator!=(const GenericIterator& rhs) {
      return !(*this == rhs);
    }
    const T* m_graph;
    // const GenericGraphBaseAdaptor<T>* parent;
    typename T::NodeIt nodeIt;
    int out_neighbor_index;
    typename T::OutArcIt _e;
    const DegVector* _deg_vector;
  };

  class GenericNode {
  public:
    GenericNode(int id) : node_id(id) {}
    int GetId() {
      return node_id;
    }
    int node_id;
  };

  typedef GenericNode TNode;
  typedef GenericIterator TNodeI;

    int AddNode(int aNId = -1) {
      int assigned_id = -1;
      if (aNId >= GetMxNId()) {
        for (int i = GetMxNId(); i <= aNId; i++) {
        graph.addNode();    
        }
        _AddNodeToVec(&(nodes_list), aNId);
      } else if (aNId == -1) {
        typename T::Node node = graph.addNode();    
        assigned_id = graph.id(node);
        _AddNodeToVec(&(nodes_list), assigned_id);
      } else {
        _AddNodeToVec(&(nodes_list), aNId);
      }
      LOG_M(DEBUG5, " Called add node with node_id = " << aNId);
      
      // _AddNode(node_id, aNid);
      return assigned_id;
    }

    TNodeI BegNI() {
      TNodeI iteratorDummy(&graph, &deg_vector);
      return iteratorDummy;
    }

    TNodeI EndNI() {
      TNodeI iteratorDummy(&graph, &deg_vector);
      return iteratorDummy;
    }

    bool IsEdge(const int& aSrcNId, const int& aDstNId) const {
      // int srcNId = externalToInternal.GetMapping(aSrcNId);
      // int dstNId = externalToInternal.GetMapping(aDstNId);
      int srcNId = aSrcNId;
      int dstNId = aDstNId;
      auto edge_it = edges_list.find(srcNId);
      if (edge_it == edges_list.end()) {
        return false;
      } else {
        if (dstNId >= edge_it->second.size()) {
          return false;
        }
        return ((edge_it->second)[dstNId] == 1);
      }
      return false;
    }

    bool IsNode(const int& NId) const {
      if (NId >= nodes_list.size()) {
        return false;
      }
      return nodes_list[NId]; // ConvertExtToInt(NId) != -1;
    }

    int GetNodes() const {
      return countNodes(graph);
    }

    int GetEdges() const {
      return countEdges(graph);
    }

    TNodeI GetNI(const int& aNId) const {
      // int node_id = externalToInternal.GetMapping(aNId);
      int node_id = aNId;
      TNodeI iteratorDummy(&graph, node_id, &deg_vector);
      return iteratorDummy;
    }

    int GetMxNId() const {
      return graph.maxNodeId() + 1; // externalToInternal.GetMaxSrc() + 1;
    }

protected:
    void _CountEdge(int aSrcNId, int aDstNId) {
      if (aSrcNId >= deg_vector.size()) {
        deg_vector.resize(aSrcNId + 1, 0);
      }
      deg_vector[aSrcNId]++;
    }

    void _AddNode(int aInt, int aExt) {
      if (aExt == -1) {
        aExt = aInt;
      }
      LOG_M(DEBUG5, " Adding node int= " << aInt << " ext=" << aExt);
      // InsertIntToExt(aInt, aExt);
      // InsertExtToInt(aExt, aInt);
    }

    void _AddNodeToVec(NodeList * aVec, int aDstNId) {
      if (aDstNId >= aVec->size()) {
        aVec->resize(aDstNId+1);
      }
      (*aVec)[aDstNId] = true;
    }

    void _AddEdge(const int& aSrcNId, const int& aDstNId) {
      NodeList emptyList;
      auto it = edges_list.insert(std::make_pair(aSrcNId, emptyList));
      // Element exists
      _AddNodeToVec(&(it.first->second), aDstNId);
    }

    T graph;
    EdgesList edges_list;
    NodeList nodes_list;
  public:
    DegVector deg_vector;
};


class GenericGraphDirectedAdaptor : public GenericGraphBaseAdaptor<lemon::SmartDigraph>  {
 public:
  int AddEdge(const int& aSrcNId, const int& aDstNId, int aWeight = 1) {
    int srcNId = aSrcNId; // ConvertExtToInt(aSrcNId);
    int dstNId = aDstNId; // ConvertExtToInt(aSrcNId);
    if (IsEdge(srcNId, dstNId) == false) {
      lemon::SmartDigraph::Node source = graph.nodeFromId(srcNId);
      lemon::SmartDigraph::Node dest = graph.nodeFromId(dstNId);
      graph.addArc(source, dest);
      _AddEdge(srcNId, dstNId);
      _CountEdge(srcNId, dstNId);
      return -1;
    } else {
      return 1;
    }
    return 1;
  }
};


class GenericGraphUnDirectedAdaptor : public GenericGraphBaseAdaptor<lemon::SmartGraph> {
 public:
   bool IsEdge(const int& aSrcNId, const int& aDstNId) const {
      LOG_M(DEBUG5, " IsEdge? lookup src == " << aSrcNId 
                    << " dst == " << aDstNId);
      
      int _srcNId = aSrcNId; // ConvertExtToInt(aSrcNId);
      int _destNId =  aDstNId; // ConvertExtToInt(aDstNId);
      int srcNId = std::min(_srcNId, _destNId);
      int destNId = std::max(_srcNId, _destNId);
      
      LOG_M(DEBUG5, " lookup src == " << srcNId 
                    << " dst == " << destNId);

      auto edge_it = edges_list.find(srcNId);
      if (edge_it == edges_list.end()) {
        LOG_M(DEBUG5, " key not found in map, key = " << srcNId);
        return false;
      } else {
        if (destNId >= edge_it->second.size()) {
          LOG_M(DEBUG5, " dest not found in map, dest = " << destNId);
          return false;
        }
        LOG_M(DEBUG5, " value in map, dest = " << (edge_it->second)[destNId]);
        return ((edge_it->second)[destNId] == 1);
      }
      return false;
    }
    
    int AddEdge(const int& aSrcNId, const int& aDstNId, int aWeight = 1) {
      LOG_M(DEBUG5, "Start: Source node Id = " << aSrcNId
                    << " Dest node Id = " << aDstNId);
      int _srcNId =  aSrcNId; // ConvertExtToInt(aSrcNId);
      int _destNId = aDstNId; // ConvertExtToInt(aDstNId);
      LOG_M(DEBUG5, "Convert: Source node Id = " << _srcNId
                    << " Dest node Id = " << _destNId);
      int srcNId = std::min(_srcNId, _destNId);
      int destNId = std::max(_srcNId, _destNId);
      if (IsEdge(srcNId, destNId) == false) {
        lemon::SmartGraph::Node source = graph.nodeFromId(srcNId);
        lemon::SmartGraph::Node dest = graph.nodeFromId(destNId);
        graph.addEdge(source, dest);
        _AddEdge(srcNId, destNId);
        _CountEdge(srcNId, destNId);
        _CountEdge(destNId, srcNId);
        return -1;
      } else {
        return 1;
      }
      return 1;
    }
};


typedef GenericGraphUnDirectedAdaptor TUnDirectedGraph;
typedef GenericGraphDirectedAdaptor TDirectedGraph;

template<>
struct GraphTrait< TUnDirectedGraph > {
    static const bool directed = false;
};

template<>
struct GraphTrait< TDirectedGraph > {
    static const bool directed = true;
};


}  // namespace graph
}  // namespace all_distance_sketch

#endif // ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_LEMON_GRAPH_ADAPTOR_H_
