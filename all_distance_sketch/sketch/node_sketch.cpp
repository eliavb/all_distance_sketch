#include "node_sketch.h"

namespace all_distance_sketch {

std::ostream& operator<<(std::ostream& os,
                         const NodeDistanceIdRandomIdData& node_details) {
  os << " NodeId=" << node_details.GetNId()
     << " Distance=" << node_details.GetDistance()
     << " HashId=" << node_details.GetRandomId();
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const NodeIdDistanceData& node_details) {
  os << " NodeId=" << node_details.GetNId()
     << " Distance=" << node_details.GetDistance();
    return os;
}

}  // namespace all_distance_sketch
