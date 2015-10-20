#include "sketch_calculation.h"

namespace all_distance_sketch {

std::ostream& operator<<(std::ostream& os, const PrunningAlgoStatistics& aStats) {
    os << " num Nodes Visited=" << aStats.num_visited_nodes <<
          " num Pruned Nodes=" << aStats.num_pruned_nodes <<
          " num Relaxed Edges=" << aStats.num_relaxed_edges;
    return os;
}

}  // namespace all_distance_sketch