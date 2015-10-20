#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_GENERIC_GRAPH_ADAPTOR_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_GENERIC_GRAPH_ADAPTOR_H_

#include "graph.h"

namespace all_distance_sketch {
namespace graph {

class GenericGraphAdaptor {
 public:
  class GenericIterator {
   public:
    int GetId() { return 0; }

    int GetDeg() { return -1; }

    int GetNbrNId(int n) { return -1; }

    int GetOutDeg() const { return -1; }

    bool HasMore() const { return false; }

    int GetOutNId(int n) const { return -1; }

    GenericIterator& operator++(int) { return *this; }

    inline bool operator==(const GenericIterator& rhs) { return false; }

    inline bool operator!=(const GenericIterator& rhs) { return false; }
  };

  class GenericNode {
   public:
    GenericNode(int dummy) {}
    int GetId() { return -1; }
  };

    typedef GenericNode TNode;
    typedef GenericIterator TNodeI;

    int AddNode(int aNid = -1) { return -1; }

    int AddEdge(const int& aSrcNId, const int& aDstNId, int aWeight = 1) {
        return -1;
    }

    TNodeI BegNI() {
        TNodeI iteratorDummy;
        return iteratorDummy;
    }

    TNodeI EndNI() {
      TNodeI iteratorDummy;
        return iteratorDummy;
    }

    bool IsEdge(const int& aSrcNId, const int& aDstNId) const {
        return false;
    }

    bool IsNode(const int& NId) const {
        return false;
    }

    int GetNodes() const { return 0; }

    int GetEdges() const { return 0; }

    TNodeI GetNI(const int& aNId) const {
      TNodeI iteratorDummy;
      return iteratorDummy;
    }

    int GetMxNId() const { return 0; }
};

typedef GenericGraphAdaptor TUnDirectedGraph;
typedef GenericGraphAdaptor TDirectedGraph;

template<>
struct GraphTrait< TUnDirectedGraph > {
    static const bool directed = false;
};

/*
template<>
struct GraphTrait< TDirectedGraph > {
    static const bool directed = true;
};
*/

}  // namespace graph
}  // namespace all_distance_sketch

#endif // ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_GENERIC_GRAPH_ADAPTOR_H_
