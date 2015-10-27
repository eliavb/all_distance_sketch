#ifndef RNN_GRAPH_ESTIMATOR_H_
#define RNN_GRAPH_ESTIMATOR_H_

#include "../common.h"
#include "../utils/utils.h"

namespace all_distance_sketch {

typedef struct Prior_t {
    int nodeId;
    std::vector<double> order;
} Prior;

typedef std::vector<Prior> PriorVector;
typedef struct NodeRank_t {
    int nodeId;
    double rank;
    NodeRank_t() {
        nodeId = -1;
        rank = -1;
    }
} NodeRank;

struct compare_node_rank {
    bool operator()(const NodeRank& n1, const NodeRank& n2) const {
        return n1.rank < n2.rank;
    }
};

class BinaryClassifier {
public:
    void InitBinaryClassifier(PriorVector * v, double threshold) {
        prior = v;
        myThreshold = threshold;
        for (unsigned int i=0; i < v->size(); i++) {
            if (myCombinedRanks.size() < (*v)[i].order.size()){
                myCombinedRanks.resize((*v)[i].order.size());
            }
            for (unsigned int j=0; j < (*v)[i].order.size(); j++) {
                int nodeId = j;
                double rank = (*v)[i].order[j];
                myCombinedRanks[nodeId].nodeId = nodeId;
                myCombinedRanks[nodeId].rank = UpdateRank(myCombinedRanks[nodeId].rank, rank);
            }
        }

        /*
        for (unsigned int i=0; i < myCombinedRanks.size(); i++) {
            if (myCombinedRanks[i].nodeId == -1) {
                myCombinedRanks.erase(myCombinedRanks.begin() + i);
            }
        }*/
        ProcessRanks();
        // compare_node_rank obj;
        // std::sort(myCombinedRanks.begin(), myCombinedRanks.end(), obj);
    }

    virtual double UpdateRank(double oldValue, double newValue) = 0;

    virtual void ProcessRanks() = 0; 

    double GetRank(int node) {
        return myCombinedRanks[node].rank;
    }

    bool Classify(int node) {
        for (unsigned int j=0; j < prior->size(); j++) {
            if (node == (*prior)[j].nodeId ) {
                return true;
            }
        }
        if (myCombinedRanks[node].rank != -1 && myCombinedRanks[node].rank < myThreshold) {
            return true;
        }
        return false;
    }

    const std::vector<NodeRank> * GetRanks() {
        return &myCombinedRanks;
    }

    void SetThreshold(double newThreshold) {
        myThreshold = newThreshold;
    }

protected:
    std::vector<NodeRank> myCombinedRanks;
    double myThreshold;
    PriorVector * prior;
};


typedef std::vector<int> Community;

template <class T>
class ClassifierAggregator {
public:
    void InitClassifierAggregator(BinaryClassifier * classifier, utils::SingleCommunity * community, graph::Graph< T > * aGraph) {
        myBinaryClassifier = classifier;
        myCommunity = community;
        myGraph = aGraph;
        myCommunityMap.clear();
        for (unsigned int i=0; i < community->size(); i++) {
            myCommunityMap.insert( std::pair<int, bool>( (*community)[i], true) );
        }
        LOG_M(DEBUG3, "finished init Classify aggregator, size= " << myCommunityMap.size());
    }

    bool TrueMember(int nodeId) {
        if (myCommunityMap.find(nodeId) != myCommunityMap.end()) {
            return true;
        }
        return false;
    }

    void ExtractStats(double * TPR, double * FPR) {
        int truePositive = 0;
        int falsePositive = 0;
        LOG_M(DEBUG3, " Starting extracting stats");
        for (typename T::TNodeI it = myGraph->BegNI(); it != myGraph->EndNI(); it++) {
            int nodeId = it.GetId();
            bool inCommunity = myBinaryClassifier->Classify(nodeId);
            if (inCommunity) {
                if (TrueMember(nodeId)) {
                    truePositive++;
                } else {
                    falsePositive++;
                }
            } 
        }
        LOG_M(DEBUG3, " Finished extracting stats");
        (*TPR) = (double )truePositive / (double )myCommunity->size();
        (*FPR) = (double) falsePositive / (double )(myGraph->GetNumNodes() - myCommunity->size());
    }

    void PrintROCCurve(std::ostream * f = (&std::cout)) {
        const std::vector<NodeRank> * ranks = myBinaryClassifier->GetRanks();
        std::vector<NodeRank> ranksCopy = (*ranks);
        // compare_node_rank obj;
        // std::sort(ranksCopy.begin(), ranksCopy.end(), obj);
        std::set<double> distinct_ranks;
        for (unsigned int i=0; i < ranks->size(); i++) {
            distinct_ranks.insert((*ranks)[i].rank);
        }
        // std::cout << "Size of set=" << distinct_ranks.size() << std::endl;
        (*f) << "Threshold,TPR,FPR" << std::endl;
        double currentRank = 0;
        for (auto it = distinct_ranks.begin(); it != distinct_ranks.end(); it++) {
            if (distinct_ranks.size() > 10000 && currentRank > 1000) {
                if ((*it) - currentRank < 1000) {
                    continue;
                }
            }
            currentRank = (*it);
            double threshold = (*it);
            double TPR, FPR;
            myBinaryClassifier->SetThreshold(threshold);
            ExtractStats(&TPR, &FPR);
            (*f) << threshold << "," << TPR <<"," << FPR << std::endl;
        }
    }

private:
    BinaryClassifier * myBinaryClassifier;
    Community * myCommunity;
    std::unordered_map<int, bool> myCommunityMap;
    graph::Graph< T > * myGraph;
};

/*
    Classifiers
*/

class BinaryMinClassifier : public BinaryClassifier {
    
    double UpdateRank(double oldValue, double newValue) {
        if (oldValue == -1) {
            return newValue;
        }
        if (oldValue < newValue) {
            return oldValue;
        }
        return newValue;
    }

    void ProcessRanks() {
        return;
    }
};

class BinaryAverageClassifier : public BinaryClassifier {
    
    double UpdateRank(double oldValue, double newValue) {
        if (oldValue == -1) {
            return newValue;
        }
        return newValue + oldValue;
    }

    void ProcessRanks() {
        for (unsigned int i=0; i < myCombinedRanks.size(); i++) {
            myCombinedRanks[i].rank = myCombinedRanks[i].rank / prior->size();
        }
        return;
    }
};

class BinaryHarmonicMeanClassifier : public BinaryClassifier {
    
    double UpdateRank(double oldValue, double newValue) {
        if (oldValue == -1) {
            return 1 / (double )newValue;
        }
        return oldValue + (1 / (double)newValue);
    }

    void ProcessRanks() {
        for (unsigned int i=0; i < myCombinedRanks.size(); i++) {
            myCombinedRanks[i].rank = prior->size() / myCombinedRanks[i].rank;
        }
        return;
    }
};
} // namespace all_distance_sketch

#endif  //  RNN_GRAPH_ESTIMATOR_H_
