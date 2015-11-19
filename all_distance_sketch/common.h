#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_H_

//  C headers
#include <limits.h>
#include <dirent.h>
// C++ Headers
#include <mutex>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <utility>
// Other libs
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/config.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/uniform_real_distribution.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/random/variate_generator.hpp"
#include "boost/random/linear_congruential.hpp"


#define PROTO_BUF 0

#define NOTICE 6
#define DEBUG1 5
#define DEBUG2 4
#define DEBUG3 3
#define DEBUG4 2
#define DEBUG5 1

#define CURRENT_LOG_LEVEL NOTICE

#define LOG_M(level, msg)\
    if ((level) >= CURRENT_LOG_LEVEL) {\
        std::cout << __FILE__ << ":" << __LINE__ << " " << msg << std::endl; \
    } \

#define _unused(x) ((void)x)

namespace all_distance_sketch {
typedef std::vector<bool> TBitSet;

namespace constants {
    const float UNREACHABLE = 999999999;
}  // namespace constants

namespace graph {
    typedef float EdgeWeight;
}  // namespace graph

}  // namespace all_distance_sketch

#endif  //  ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_GRAPH_H_
