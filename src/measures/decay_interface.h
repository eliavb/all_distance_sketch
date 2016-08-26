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
  double Alpha(double distance) { 
  	if (distance == 0) {
  		return 0;
  	}
  	return 1 / distance; 
  }
};

class InverseRootDecay : public DecayInterface {
 public:
  double Alpha(double distance) { 
  	if (distance == 0) {
  		return 0;
  	}
  	return 1 / std::sqrt(distance); }
};

class InverseLogDecay : public DecayInterface {
 public:
  double Alpha(double distance) { 
  	if (distance == 0) {
  		return 0;
  	}
  	return 1 / std::log(distance); }
};

class InverseRootLogDecay : public DecayInterface {
 public:
  double Alpha(double distance) { 
  	if (distance == 0) {
  		return 0;
  	}
  	return 1 / std::sqrt(std::log(distance)); }
};

}  //  namespace all_distance_sketch

#endif  // SRC_MEASURES_DECAY_INTERFACE_H_