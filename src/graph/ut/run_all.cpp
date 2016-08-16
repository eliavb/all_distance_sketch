
#include "gtest/gtest.h"
#include "../../sketch/ut/ads_test.cpp"
#include "../../app/ut/app_test.cpp"
#include "../../algorithms/ut/algo_test.cpp"
#include "../../algorithms/ut/test_dijkstra_min_weight_path.cpp"
#include "../../algorithms/ut/serialization_test.cpp"
#include "../../algorithms/ut/t_skim_test.cpp"
#include "basic_graph.cpp"
#include "../../labels/ut/labels_test.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
