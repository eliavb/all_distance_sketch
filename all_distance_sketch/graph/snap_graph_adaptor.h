#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_SNAP_ADAPTOR_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_SNAP_ADAPTOR_H_

#include <Snap.h>
#undef min
#undef max
#undef Fail
#undef __Bool
#include "graph.h"

namespace all_distance_sketch {
namespace graph {

typedef TUNGraph TUnDirectedGraph;
typedef TNGraph TDirectedGraph;

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

#endif // ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_SNAP_ADAPTOR_H_
