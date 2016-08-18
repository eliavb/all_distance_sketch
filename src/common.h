#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

//  C headers
#include <limits.h>
#include <dirent.h>
// C++ Headers
#include <mutex>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <cstdio>
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



#define PROTO_BUF 1

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

bool double_equals(double a, double b, double epsilon = 0.000001) {
    return std::abs(a - b) < epsilon;
}

/*! \cond
*/
typedef float LABEL_WEIGHT;
typedef std::vector<bool> TBitSet;
/*! \endcond
*/

/*! Constants */
namespace constants {
    const float UNREACHABLE = 999999999; /*!< Constant to mark unreachable */
    const float INF = 99999999;
}  // namespace constants

/*! Graph namesapce */
namespace graph {
    typedef float EdgeWeight;
}  // namespace graph

}  // namespace all_distance_sketch

#endif  //  SRC_COMMON_H_
