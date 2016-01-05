
#include "gtest/gtest.h"
#include "../../sketch/ut/ads_test.cpp"
#include "../../algorithms/ut/algo_test.cpp"
#include "../../algorithms/ut/t_skim_test.cpp"
#include "basic_graph.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
