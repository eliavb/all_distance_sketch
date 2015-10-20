#ifndef THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_RANK_CALCULATOR_H_
#define THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_RANK_CALCULATOR_H_
#include "../graph/graph.h"
#include "../common.h"

namespace all_distance_sketch {

class RankCalculatorInterface {
public:
	virtual ~RankCalculatorInterface() {};
	virtual double CalculateNodeRank(int node_id) = 0;

	virtual double GetNodeInsertProb(int node_id, double threshold_prob) = 0;
};

template <class T>
class DegreeRankCalculator : public RankCalculatorInterface {
 public:
    void InitDegreeRankCalculator(const graph::Graph<T>* graph) {
                m_graph = graph;
		m_rd.reset(new std::random_device());
		m_gen.reset(new std::mt19937((*m_rd)));
		m_dis.reset(new std::uniform_real_distribution<>(0, 1));
	}

	double CalculateNodeRank(int node_id) override {
		int node_degree = m_graph->GetNI(node_id).GetDeg();
		double degree_param = node_degree == 0 ? degree_param = 1 : (1 / static_cast<double>(node_degree));
		m_dis.reset(new std::uniform_real_distribution<>(0, degree_param));
    	return m_dis((*m_gen));
	}

	double GetNodeInsertProb(int node_id, double threshold_prob) override {
		int node_degree = m_graph->GetNI(node_id).GetDeg();
		double degree_param = node_degree == 0 ? degree_param = 1 : (1 / static_cast<double>(node_degree));
		if (degree_param < threshold_prob) {
			return 1;
		} 

		return threshold_prob / degree_param;
	}
 private:
    const graph::Graph<T>* m_graph;
    std::unique_ptr<std::random_device> m_rd;
    std::unique_ptr<std::mt19937 > m_gen;
    std::unique_ptr<std::uniform_real_distribution<> > m_dis;
};

typedef boost::minstd_rand base_generator_type;
class UniformRankCalculator : public RankCalculatorInterface {
 public:
    void InitUniformRankCalculator(double start_range = 0, double end_range = 1) {
        m_generator.reset(new base_generator_type(42u));
        m_uni_dist.reset(new boost::uniform_real<>(start_range, end_range));
        m_dis.reset(new boost::variate_generator<base_generator_type&, boost::uniform_real<> >((*m_generator), (*m_uni_dist)) );
	}

	double CalculateNodeRank(int node_id) override {
		return (*m_dis)();
	}

	double GetNodeInsertProb(int node_id, double threshold_prob) override {
		return threshold_prob;
	}

 private:
    std::unique_ptr<base_generator_type> m_generator;
    std::unique_ptr<boost::uniform_real<> > m_uni_dist;
    std::unique_ptr<boost::variate_generator<base_generator_type&, boost::uniform_real<> > > m_dis;
};

}  // namespace all_distance_sketch
#endif  // THIRD_PARTY_ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_SKETCH_RANK_CALCULATOR_H_
