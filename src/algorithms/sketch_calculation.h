#ifndef SRC_ALGORITHMS_SKETCH_CALCULATION_H_
#define SRC_ALGORITHMS_SKETCH_CALCULATION_H_

#include "../common.h"
#include "../sketch/graph_sketch.h"
#include "../graph/graph.h"
#include "../utils/thread_utils.h"

#include "./dijkstra_shortest_paths.h"

/*! \file sketch_calculation.h
    \brief Sketch calculation algorithms
*/

namespace all_distance_sketch {

/*! \brief Calculate the bottom K graph sketch in parallel
  This implementation is faster than the single threaded implementation but
  the cost is in memory.
  \param[in] graph - Graph data structure to calculate his sketch
  \param[out] graph_sketch - The result graph sketch
  \param[in] num_threads - The number of threads to use in the calculation
  \param[in] increase_factor - The calculation divides the work to batchs of increasing size.
                               This factor will determine the size of each batch when the starting 
                               batch size is defined to be of size K received from the graph_sketch.
  \see graph::Graph
  \see GraphSketch
*/
template <class T>
static void CalculateGraphSketchMultiCore(graph::Graph<T>* graph,
                                   GraphSketch * graph_sketch,
                                   unsigned int num_threads = 5,
                                   double increase_factor = 1.1);

/*! \brief Calculate the bottom K single threaded
  \param[in] graph - Graph data structure to calculate his sketch
  \param[out] graph_sketch - The result graph sketch
  \see graph::Graph
  \see GraphSketch
*/
template <class T>
static void CalculateGraphSketch(graph::Graph<T> *graph,
                                 GraphSketch * graph_sketch);

/*! \cond
*/
struct PrunningAlgoStatistics;

/*
* Forward declaration
*/

template <class T>
static void AssignTask(unsigned int start_index,
                       unsigned int end_index,
                       bool insert_to_candidate_list,
                       bool clear_candidate_list,
                       thread::MessageChannel * communication_channel,
                       int num_threads);

template <class T>
static void ThreadLoop(thread::MessageChannel * communication_channel,
                       graph::Graph<T> *graph,
                       GraphSketch * graph_sketch,
                       SketchDijkstraCallBacks<T>* call_backs,
                       DijkstraParams * dijkstra_param,
                       unsigned int id);

template <class T>
static void CalculateSketchBatch(graph::Graph<T> *graph,
                                 SketchDijkstraCallBacks<T>* call_backs,
                                 DijkstraParams * dijkstra_param,
                                 const std::vector<NodeDistanceIdRandomIdData> * distribution,
                                 unsigned int start_index,
                                 unsigned int end_index);

template <class T>
static void CalculateNodeSketch(typename T::TNode source,
                                 graph::Graph<T> *graph,
                                 SketchDijkstraCallBacks<T>* call_backs,
                                 DijkstraParams * param);



template <class T>
static void CalculateNodeSketch(typename T::TNode source,
                                 graph::Graph<T> *graph,
                                 SketchDijkstraCallBacks<T>* call_backs,
                                 DijkstraParams * param) {
    PrunedDijkstra<T, SketchDijkstraCallBacks<T> >(source,
                                                  graph,
                                                  call_backs,
                                                  param);
}

template <class T>
static void CalculateGraphSketch(graph::Graph<T> *graph,
                                 GraphSketch * graph_sketch) {
    // The vector is sorted
    DijkstraParams param;
    SketchDijkstraCallBacks<T> call_backs;
    call_backs.InitSketchDijkstraCallBacks(graph_sketch);
    const std::vector<NodeDistanceIdRandomIdData> * distribution = graph_sketch->GetNodesDistribution();
    for (unsigned int i=0; i < distribution->size(); i++) {
      typename T::TNode source((*distribution)[i].GetNId());
      if ((*distribution)[i].GetRandomId() == ILLEGAL_RANDOM_ID) {
        continue;
      }
      std::cout << "running from " << (*distribution)[i].GetNId() << std::endl;
      if (graph->IsNode((*distribution)[i].GetNId())) {
          CalculateNodeSketch<T>(source, graph, &call_backs, &param);
      }
    }
    graph_sketch->CalculateAllDistanceNeighborhood();
}

static int GetBatchSize(int numthreads, int approxSize) {
    int mod = approxSize % numthreads;
    if (mod == 0) {
        return approxSize;
    }
    return approxSize + (numthreads - mod);
}

template <class T>
static void ThreadLoop(thread::MessageChannel * communication_channel,
                       graph::Graph<T> *graph,
                       GraphSketch * graph_sketch,
                       SketchDijkstraCallBacks<T>* call_backs,
                       DijkstraParams * dijkstra_param,
                       unsigned int id) {
    while (communication_channel->get_should_stop() == false) {
        thread::Message message;
        if (communication_channel->GetMessage(&message, id)) {
            if (message.insert_to_candidate_list) {
                LOG_M(DEBUG2, "has work start_index " << message.start_index << " end_index " << message.end_index);
                CalculateSketchBatch<T>(graph, call_backs, dijkstra_param, graph_sketch->GetNodesDistribution(), message.start_index, message.end_index);
            }
            if (message.clear_candidate_list) {
                graph_sketch->InsertCandidatesNodes(message.start_index, message.end_index);
            }
            communication_channel->Finished(id);
            LOG_M(DEBUG2, "Finished work start_index " << message.start_index << " end_index " << message.end_index);
        }
    }
    LOG_M(DEBUG2, "  Exiting Finished ");
}

template <class T>
static void CalculateGraphSketchMultiCore(graph::Graph<T> *graph,
                                   GraphSketch * graph_sketch,
                                   unsigned int num_threads,
                                   double increase_factor) {
    const std::vector<NodeDistanceIdRandomIdData> * distribution = graph_sketch->GetNodesDistribution();
    std::vector<std::thread> threads;
    std::vector<DijkstraParams> params;
    std::vector<SketchDijkstraCallBacks<T>* > call_backs;
    call_backs.resize(num_threads);
    params.resize(num_threads);
    thread::ModuloLock lock;
    lock.InitModuloLock();
    thread::MessageChannel communication_channel;
    communication_channel.InitMessageChannel(num_threads);
    for (int i=0; i < num_threads; ++i) {
      call_backs[i] = new SketchDijkstraCallBacks<T>();
      call_backs[i]->InitSketchDijkstraCallBacks(graph_sketch);
      call_backs[i]->set_multi_threaded_params(true, &lock);
    }
    for (unsigned int i=0; i < num_threads; i++) {
        threads.push_back( std::thread( ThreadLoop<T>,
                                       &communication_channel,
                                       graph,
                                       graph_sketch,
                                       (call_backs[i]),
                                       &(params[i]),
                                       i) );

    }
    for (unsigned int i=0; i < threads.size(); i++) {
        while (threads[i].joinable() == false) {}
    }

    LOG_M(DEBUG2, "Num nodes " << distribution->size() );
    unsigned int batch_size = GetBatchSize(num_threads, graph_sketch->GetK());
    for (unsigned int i=0; i < distribution->size();) {
        int batchEnd = i + batch_size;
        if (batchEnd > distribution->size()) {
            batchEnd = distribution->size() + 1;
        }
        AssignTask<T>(i, batchEnd, true, false, &communication_channel, num_threads);

        AssignTask<T>(0, distribution->size() + 1, false, true,
                      &communication_channel, num_threads);

        i = i + batch_size;
        batch_size *= increase_factor;
        batch_size = GetBatchSize(num_threads, batch_size);
    }
    // Ask all to stop
    communication_channel.stop();
    for (unsigned int i=0; i < threads.size(); i++) {
        threads[i].join();
    }
    graph_sketch->CalculateAllDistanceNeighborhood();
}

template <class T>
static void AssignTask(unsigned int start_index,
                       unsigned int end_index,
                       bool insert_to_candidate_list,
                       bool clear_candidate_list,
                       thread::MessageChannel * communication_channel,
                       int num_threads) {
    unsigned int workPerThread = (end_index - start_index) / num_threads;
    std::vector<std::thread> threads;
    LOG_M(DEBUG1, " Allocating work " << start_index << " end=" << end_index);
    for (unsigned int i=start_index, threadIndex=0; i < end_index; i += workPerThread, threadIndex++) {
        communication_channel->AddBatch(i, i+workPerThread, insert_to_candidate_list, clear_candidate_list);
    }
    while (communication_channel->AllFinished() == false) {}
    LOG_M(DEBUG1, "Finished Allocating work " << start_index << " end=" << end_index);
}

template <class T>
static void CalculateSketchBatch(graph::Graph<T> *graph,
                                 SketchDijkstraCallBacks<T>* call_backs,
                                 DijkstraParams * dijkstra_param,
                                 const std::vector<NodeDistanceIdRandomIdData> * distribution,
                                 unsigned int start_index,
                                 unsigned int end_index) {
    for (unsigned int i=start_index; (i < distribution->size() && i < end_index ); i++) {
        typename T::TNode source((*distribution)[i].GetNId());
        if ((*distribution)[i].GetRandomId() == ILLEGAL_RANDOM_ID) {
          continue;
        }
        if (graph->IsNode((*distribution)[i].GetNId())) {
            CalculateNodeSketch<T>(source, graph, call_backs, dijkstra_param);
        }
    }
}

/*! \endcond
*/

}  //  namespace all_distance_sketch
#endif  //  SRC_ALGORITHMS_SKETCH_CALCULATION_H_
