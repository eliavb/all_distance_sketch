#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_UTILS_THREAD_UTILS_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_UTILS_THREAD_UTILS_H_

#include "../common.h"

namespace all_distance_sketch {
namespace thread {

class ModuloLock {
public:
    void InitModuloLock(unsigned int anum_locks_ = 1048576) {
        locks_ = new std::mutex[anum_locks_];
        num_locks_ = anum_locks_;
    }
    void Lock(unsigned int aObjectId) {
        while ( (locks_ + (aObjectId % num_locks_))->try_lock() == false ) {
            usleep(1000);
        }
    }
    void UnLock(unsigned int aObjectId) {
        (locks_ + (aObjectId % num_locks_))->unlock();
    }
    int GetNumCollitions() {
        return 0;// numCollitions;
    }
    ~ModuloLock() {
        delete [] locks_;
    }
private:
    std::mutex * locks_;
    unsigned int num_locks_;
    // std::atomic_uint numCollitions;
};

struct Message {
    Message() : start_index(0), end_index(0), insert_to_candidate_list(false), clear_candidate_list(false) {}
    unsigned int start_index;
    unsigned int end_index;
    bool insert_to_candidate_list;
    bool clear_candidate_list;
};

class MessageChannel {
public:
    void InitMessageChannel(unsigned int aNumThreads) {
        shouldStop = false;
        threadStatus.resize(aNumThreads, false);
    }

    void stop() {
        shouldStop = true;
    }

    bool get_should_stop() {
        return shouldStop;
    }

    void AddBatch(unsigned int aStartIndex, unsigned int aEndIndex, bool aExec, bool aClean) {
        locks_.lock();
        Message m;
        m.start_index = aStartIndex;
        m.end_index = aEndIndex;
        m.insert_to_candidate_list = aExec;
        m.clear_candidate_list = aClean;
        myMessages.push_back(m);
        locks_.unlock();
    }
    bool GetMessage(Message * m, unsigned int id) {
        bool ret;
        locks_.lock();
        if (myMessages.size() > 0) {
            (*m) = myMessages.front();
            myMessages.pop_front();
            threadStatus[id] = true;
            ret = true;
        }else {
            ret = false;
        }
        locks_.unlock();
        return ret;
    }

    bool AllFinished() {
        bool ret = true;
        locks_.lock();
        if (myMessages.size() != 0) {
            locks_.unlock();
            return false;
        }
        for (unsigned int i=0; i < threadStatus.size(); i++) {
            if (threadStatus[i] == true) {
                ret = false;
            }
        }
        locks_.unlock();
        return ret;
    }

    void Finished(unsigned int id) {
        locks_.lock();
        threadStatus[id] = false;
        locks_.unlock();
    }

private:
    std::list< Message > myMessages;
    unsigned int end_index;
    std::mutex  locks_;
    bool shouldStop;
    std::vector<bool> threadStatus;
};

} // namesapce thread
} // namespace all_distance_sketch

#endif
