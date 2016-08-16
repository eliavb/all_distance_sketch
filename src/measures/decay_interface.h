#ifndef SRC_MEASURES_DECAY_INTERFACE_H_
#define SRC_MEASURES_DECAY_INTERFACE_H_

namespace all_distance_sketch {

class DecayInterface {
 public:
  virtual ~DecayInterface() {}
  virtual double Alpha(double distance) = 0;
};

class DefaultDecay : public DecayInterface {
 public:
  double Alpha(double distance) { return distance; }
};

class InverseDecay : public DecayInterface {
 public:
  double Alpha(double distance) { return 1 / distance; }
};

}  //  namespace all_distance_sketch

#endif  // SRC_MEASURES_DECAY_INTERFACE_H_