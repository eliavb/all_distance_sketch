#include <limits.h>
// #include "third_party/gtest/include/gtest/gtest.h"
#include "gtest/gtest.h"
#include "../node_sketch.h"

class BasicADS : public ::testing::Test {
 protected:
  BasicADS() {}

  virtual ~BasicADS() {}

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {}

  virtual void TearDown() {}
};

TEST_F(BasicADS, ADSBasicK1) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(1, 2, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData v(1, 4, 0.4);
  EXPECT_EQ(ads.Add(v), true);
  all_distance_sketch::NodeDistanceIdRandomIdData v1(1, 5, 0.5);
  EXPECT_EQ(ads.Add(v1), false);
  all_distance_sketch::NodeDistanceIdRandomIdData v2(1, 3, 0.6);
  EXPECT_EQ(ads.Add(v2), false);
}

TEST_F(BasicADS, ADSBasicK2) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(2, 1, 0.9);
  // Should be inserted
  all_distance_sketch::NodeDistanceIdRandomIdData v(10, 2, 0.4);
  EXPECT_EQ(ads.Add(v), true);
  // Should be inserted since in distance 3 he is the smallest
  all_distance_sketch::NodeDistanceIdRandomIdData v1(5, 3, 0.5);
  EXPECT_EQ(ads.Add(v1), true);
  // Should be inserted since in distance 4 he is one of the 2 smallest
  all_distance_sketch::NodeDistanceIdRandomIdData v2(3, 4, 0.6);
  EXPECT_EQ(ads.Add(v2), true);
}

TEST_F(BasicADS, ADSBasicK100) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(100, 1, 0.9);
  for (int i = 2 ; i < 100; i ++){
    all_distance_sketch::NodeDistanceIdRandomIdData v(1, i, 0.5);
    EXPECT_EQ(ads.Add(v), true);
  }
}

TEST_F(BasicADS, ADSBasicK3) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(1, 1, 0.9);
  // Should be inserted k == 1
  all_distance_sketch::NodeDistanceIdRandomIdData v(20, 2, 0.1);
  EXPECT_EQ(ads.Add(v), true);
  
  // Not
  all_distance_sketch::NodeDistanceIdRandomIdData v1(20, 3, 0.2);
  EXPECT_EQ(ads.Add(v1), false);
  
  // Should be inserted with distance 19
  all_distance_sketch::NodeDistanceIdRandomIdData v2(19, 4, 0.3);
  EXPECT_EQ(ads.Add(v2), true);
  // Should be inserted with distance 18
  all_distance_sketch::NodeDistanceIdRandomIdData v3(18, 5, 0.4);
  EXPECT_EQ(ads.Add(v3), true);
  // Should be inserted with distance 15
  all_distance_sketch::NodeDistanceIdRandomIdData v4(15, 6, 0.5);
  EXPECT_EQ(ads.Add(v4), true);
  // Should be inserted with distance 10
  all_distance_sketch::NodeDistanceIdRandomIdData v5(10, 7, 0.6);
  EXPECT_EQ(ads.Add(v5), true);
  
  all_distance_sketch::NodeIdDistanceVector vNodes;
  // Get all nodes up to distance 9 (None)
  ads.Get(9, &vNodes);
  EXPECT_EQ(vNodes.size(), 0);  
  // Get all nodes up to disntace 10 ( Only 1)
  ads.Get(10, &vNodes);
  EXPECT_EQ(vNodes.size(), 1);
  // Get all nodes up to disntace 15 ( Only 2)
  ads.Get(15, &vNodes);
  EXPECT_EQ(vNodes.size(), 2);

  ads.Get(16, &vNodes);
  EXPECT_EQ(vNodes.size(), 2);

  ads.Get(18, &vNodes);
  EXPECT_EQ(vNodes.size(), 3);

  ads.Get(19, &vNodes);
  EXPECT_EQ(vNodes.size(), 4);

  ads.Get(20, &vNodes);
  EXPECT_EQ(vNodes.size(), 5);
  
}


TEST_F(BasicADS, ADSSmallest) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(1, 1, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData v1(1, 2, 0.0001);
  EXPECT_EQ(ads.Add(v1), true);
  for (unsigned int i = 100; i > 100; --i ){
    all_distance_sketch::NodeDistanceIdRandomIdData vTemp(1, i, 1.0 / ((double)i) );
    EXPECT_EQ(ads.Add(vTemp), false);
  }
}


TEST_F(BasicADS, ADSVector1) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(1, 1, 0.9);
  
  for (unsigned int i = 0; i < 7; i++ ){
    // At each distance only one node should reside
    all_distance_sketch::NodeIdDistanceVector vNodes;
    ads.Get(i, &vNodes);  
    EXPECT_EQ(vNodes.size(), 0);
  }
}

TEST_F(BasicADS, ADSVector2) {
  all_distance_sketch::NodeSketch ads;
  ads.InitNodeSketch(1, 1, 0.9);
  
  for (unsigned int i = 7; i < 2; i++ ){
    all_distance_sketch::NodeIdDistanceVector vNodes;
    all_distance_sketch::NodeDistanceIdRandomIdData vTemp(1, i, 1.0 / ((double)i) );
    EXPECT_EQ(ads.Add(vTemp), true);
    ads.Get(i, &vNodes);  
    EXPECT_EQ(vNodes.size(), 1);
  }
}


TEST_F(BasicADS, ADSNeighboordhood1) {
  all_distance_sketch::NodeSketch ads;
  std::vector<double> v;
  v.resize(20, 0);
  v[1]=0.9; v[2] = 0.2; v[3] = 0.01;
  ads.InitNodeSketch(3, 1, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp(1, 2, 0.2);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp1(4, 3, 0.01);
  EXPECT_EQ(ads.Add(vTemp), true);
  EXPECT_EQ(ads.Add(vTemp1), true);
  ads.CalculateAllDistanceNeighborhood(&v);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(1, &v), 0);
  
  
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(1, &v), 0);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(5, &v), 1);
}

TEST_F(BasicADS, ADSNeighboordhood2) {
  all_distance_sketch::NodeSketch ads;
  std::vector<double> v;
  v.resize(20, 0);
  v[1] = 0.9; v[2] = 0.001; v[3] = 0.01; v[4] = 0.2;
  ads.InitNodeSketch(2, 1, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp(10, 2, 0.001);
  EXPECT_EQ(ads.Add(vTemp), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp1(4, 3, 0.01);
  EXPECT_EQ(ads.Add(vTemp1), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp2(1, 4, 0.2);
  EXPECT_EQ(ads.Add(vTemp2), true);
  ads.CalculateAllDistanceNeighborhood(&v);
  // Only one node is relevant
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(1, &v), 0);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(4, &v), 5);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(10, &v), 100);
}

TEST_F(BasicADS, ADSNeighboordhood3) {
  all_distance_sketch::NodeSketch ads;
  std::vector<double> v;
  v.resize(20, 0);
  v[1]=0.9; v[5] = 0.0001; v[2] = 0.001; v[3] = 0.01; v[4] = 0.2;
  ads.InitNodeSketch(2, 1, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp3(11, 5, 0.0001);
  EXPECT_EQ(ads.Add(vTemp3), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp(10, 2, 0.001);
  EXPECT_EQ(ads.Add(vTemp), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp1(4, 3, 0.01);
  EXPECT_EQ(ads.Add(vTemp1), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp2(1, 4, 0.2);
  EXPECT_EQ(ads.Add(vTemp2), true);
  ads.CalculateAllDistanceNeighborhood(&v);
  // Only one node is relevant
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(1, &v), 0);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(4, &v), 5);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(10, &v), 100);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(11, &v), 1000);
}


TEST_F(BasicADS, ADSNeighboordhood4) {
  all_distance_sketch::NodeSketch ads;
  std::vector<double> v;
  v.resize(20, 0);
  v[1]=0.9; v[5] = 0.0001; v[2] = 0.001; v[3] = 0.01; v[4] = 0.2;
  ads.InitNodeSketch(100, 1, 0.9);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp3(11, 5, 0.0001);
  EXPECT_EQ(ads.Add(vTemp3), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp(10, 2, 0.001);
  EXPECT_EQ(ads.Add(vTemp), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp1(4, 3, 0.01);
  EXPECT_EQ(ads.Add(vTemp1), true);
  all_distance_sketch::NodeDistanceIdRandomIdData vTemp2(1, 4, 0.2);
  EXPECT_EQ(ads.Add(vTemp2), true);
  ads.CalculateAllDistanceNeighborhood(&v);
  // Only one node is relevant
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(1, &v), 0);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(4, &v), 1);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(10, &v), 2);
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(11, &v), 3);

  ads.CalculateAllDistanceNeighborhood(&v);
  // all_distance_sketch::NeighbourhoodVector * s = ads.UTGetNeighbourhoodVector();
  /*
  for (unsigned int i=0; i < s->size(); i++) {
      std::cout << " Distance= " << (*s)[i].GetDistance() << " size= " << (*s)[i].GetSize() << std::endl;
  }*/
}

TEST_F(BasicADS, ADSRankCheck) {
  all_distance_sketch::NodeSketch ads;
  std::vector<double> v;
  ads.InitNodeSketch(100, 200, 0.9);
  v.resize(100, 0);
  for (int i=0; i < 100; i++) {
    double rank = 1 / (100 - i);
    v[i] = rank;
    all_distance_sketch::NodeDistanceIdRandomIdData vTemp(1, i, rank);
    EXPECT_EQ(ads.Add(vTemp), true);
  }
  ads.CalculateAllDistanceNeighborhood(&v);
  // Only one node is relevant
  EXPECT_EQ(ads.GetSizeNeighborhoodUpToDistance(2, &v), 99);
}

TEST_F(BasicADS, NodeSketchNeighborhoodDistance) {
  all_distance_sketch::NodeSketch node_sketch;
  std::vector<double> v;
  node_sketch.InitNodeSketch(100, 200, 0.9);
  v.resize(100, 0);
  for (int i=0; i < 100; i++) {
    double rank = 1 / (100 - i);
    v[i] = rank;
    all_distance_sketch::NodeDistanceIdRandomIdData vTemp(i + 1, i, rank);
    EXPECT_EQ(node_sketch.Add(vTemp), true);
  }
  node_sketch.CalculateAllDistanceNeighborhood(&v);
  
  for (int i =0 ; i < 99; i++) {
    EXPECT_EQ(node_sketch.GetDistanceCoverNeighborhood(i), i+2);
  }
}
