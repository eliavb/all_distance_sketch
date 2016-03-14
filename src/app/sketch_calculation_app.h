#ifndef ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_
#define ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include <boost/program_options.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <glob.h>

#include "../all_distance_sketch.h"
#include "utils.h"
using namespace std;
using namespace boost;
using namespace all_distance_sketch;
namespace po = boost::program_options;


static const int kMaxFileSize = 256000000;

inline std::vector<std::string> glob(const std::string& pat){
    using namespace std;
    glob_t glob_result;
    glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> ret;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}

bool WriteMessage(const google::protobuf::MessageLite& message,
                  CodedOutputStream* coded_output,
                  int* size_written) {
    const int size = message.ByteSize();
    coded_output->WriteVarint32(size);

    uint8_t* buffer = coded_output->GetDirectBufferForNBytesAndAdvance(size);
    if (buffer != NULL) {
        // Optimization:  The message fits in one buffer, so use the faster
        // direct-to-array serialization path.
        message.SerializeWithCachedSizesToArray(buffer);
    } else {
        // Slightly-slower path when the message is multiple buffers.
        message.SerializeWithCachedSizes(coded_output);
        if (coded_output->HadError()) return false;
    }
    (*size_written) += size;
    return true;
}

bool ReadMessage(google::protobuf::MessageLite* message,
                 google::protobuf::io::CodedInputStream* coded_input) {
    // Read the size.
    uint32_t size;
    if (!coded_input->ReadVarint32(&size)) return false;

    // Tell the stream not to read beyond that size.
    google::protobuf::io::CodedInputStream::Limit limit =
      coded_input->PushLimit(size);

    // Parse the message.
    if (!message->MergeFromCodedStream(coded_input)) return false;
    if (!coded_input->ConsumedEntireMessage()) return false;

    // Release the limit.
    coded_input->PopLimit(limit);

    return true;
}

void OpenFileWrite(const std::string& file_name,
              int* fd,
              ZeroCopyOutputStream** raw_output,
              CodedOutputStream** coded_output) {
    (*fd) = open(file_name.c_str(), O_CREAT | O_RDWR);
    chmod(file_name.c_str(), 777);
    (*raw_output) = new FileOutputStream(*fd);
    (*coded_output) = new CodedOutputStream(*raw_output);
}


void CloseFile(int* fd,
              ZeroCopyOutputStream* raw_output,
              CodedOutputStream* coded_output) {
    delete coded_output;
    delete raw_output;
    close(*fd);
}

void DumpGraphSketchToFile(const AllDistanceSketchGpb& graph_sketch,
                           std::string output_file) {
    int fd;
    ZeroCopyOutputStream* raw_output = NULL;
    CodedOutputStream* coded_output = NULL;
    output_file += "_";
    std::string file_name = output_file + std::to_string(0);
    OpenFileWrite(file_name, &fd, &raw_output, &coded_output);
    int size_written = 0;
    int num_files = 1;
    coded_output->WriteLittleEndian32(1);
    WriteMessage(graph_sketch.configuration(), coded_output, &size_written);
    for (int i=0; i < graph_sketch.nodes_sketches_size(); i++) {
        coded_output->WriteLittleEndian32(2);
        WriteMessage(graph_sketch.nodes_sketches(i), coded_output, &size_written);
        if (size_written >= kMaxFileSize) {
            CloseFile(&fd, raw_output, coded_output);
            file_name = output_file + std::to_string(num_files);
            num_files += 1;
            size_written = 0;
            OpenFileWrite(file_name, &fd, &raw_output, &coded_output);
        }
    }
    for (int i=0; i < graph_sketch.node_thresholds_size(); i++) {
        coded_output->WriteLittleEndian32(3);
        WriteMessage(graph_sketch.node_thresholds(i), coded_output, &size_written);
        if (size_written >= kMaxFileSize) {
            CloseFile(&fd, raw_output, coded_output);
            file_name = output_file + std::to_string(num_files);
            num_files += 1;
            size_written = 0;
            OpenFileWrite(output_file, &fd, &raw_output, &coded_output);
        }
    }
    CloseFile(&fd, raw_output, coded_output);
}

void LoadGraphSketchFromFiles(AllDistanceSketchGpb* graph_sketch,
                             std::vector<std::string> files) {
    for (const auto file_name : files) {
        std::cout << file_name << std::endl;
        int fd = open(file_name.c_str(), O_RDONLY);
        ZeroCopyInputStream* raw_input = new FileInputStream(fd);
        CodedInputStream* coded_input = new CodedInputStream(raw_input);
        coded_input->SetTotalBytesLimit(kMaxFileSize*2, kMaxFileSize*2);
        while (coded_input->ExpectAtEnd() == false) {
            uint32_t message_number;
            if (coded_input->ReadLittleEndian32(&message_number) == false) {
                std::cout << "Unable to read! num=" << message_number << std::endl;
                break;
            }
            bool ableToRead = true;
            if (message_number == 1) {
                ableToRead = ReadMessage(graph_sketch->mutable_configuration(), coded_input);
                std::cout << graph_sketch->configuration().DebugString() << std::endl;
            }
            if (message_number == 2) {
                ableToRead = ReadMessage(graph_sketch->add_nodes_sketches(), coded_input);
            }
            if (message_number == 3) {
                ableToRead = ReadMessage(graph_sketch->add_node_thresholds(), coded_input);
            }
            if (ableToRead == false) {
                break;
            }
        }
    }
}

bool parse_command_line_args(int ac, char* av[], int* K,
                                                 int* num_threads,
                                                 bool* directed,
                                                 std::string* graph_dir,
                                                 std::string* output_file) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("K", po::value<int>(K)->required(), 
                  "K = 1/epsilon^2 sets the precision of the approximation")
            ("num_threads", po::value<int>(num_threads)->default_value(1), 
                  "num_threads to use")
            ("directed", po::value<bool>(directed), 
                  "is the graph directed")
            ("graph_dir", po::value< std::string >(graph_dir)->required(),
                  "Directory with the graph to calculate the sketch on")
            ("output_file", po::value< std::string > (output_file)->required(), 
                  "output file path, here the sketch Gpb will be saved (Gpb defined in src/proto/all_distance_sketch.proto)")
        ;

        po::positional_options_description p;
        
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
    
        if (vm.count("help")) {
            cout << "Usage: options_description [options]\n";
            cout << desc;
            return true;
        }
        po::notify(vm);
        /*
        cout << "output_file=" << *output_file << endl;
        cout << "graph_dir=" << *graph_dir << endl;
        cout << "directed=" << *directed << endl;
        cout << "K=" << *K << endl;
        cout << "#threads=" << *num_threads << endl;
        */
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return true;
    }    
    return false;
}

int sketch_app_main(int ac, char*av[]) {
    int K, num_threads;
    bool directed;
    std::string output_file, graph_dir;
    if (parse_command_line_args(ac, av, &K, &num_threads, &directed, &graph_dir, &output_file)) {
        return 1;
    }

    GraphSketch graph_sketch;
#if 1
    graph::Graph< graph::TDirectedGraph> directed_graph;
    graph::Graph< graph::TUnDirectedGraph> un_directed_graph;
    load_graph(directed, graph_dir, &directed_graph, &un_directed_graph);
    std::cout << "maxNid=" << un_directed_graph.GetMxNId() << std::endl;
    calc_graph_sketch(K, num_threads, directed, &graph_sketch,
                      graph_dir, &directed_graph, &un_directed_graph);
    AllDistanceSketchGpb all_distance_sketch;
    graph_sketch.SaveGraphSketchToGpb(&all_distance_sketch);
    std::cout << "max threshold proto=" << all_distance_sketch.node_thresholds_size() << std::endl;
    std::cout << "max sketch size proto=" << all_distance_sketch.nodes_sketches_size() << std::endl;

    DumpGraphSketchToFile(all_distance_sketch, output_file);
#endif    
    AllDistanceSketchGpb all_distance_sketch2;
    {
        std::string pattern = output_file + "_*";
        auto files = glob(pattern);
        for (const auto file: files) {
            std::cout << file << std::endl;
        }
        LoadGraphSketchFromFiles(&all_distance_sketch2, files);
    }
    
#if 1
    GraphSketch graph_sketch_after;
    graph_sketch_after.LoadGraphSketchFromGpb(all_distance_sketch2);
    
    const std::vector<RandomId>* a = graph_sketch.GetNodesDistributionLean();
    const std::vector<RandomId>* b = graph_sketch_after.GetNodesDistributionLean();
    std::cout << "a size=" << a->size() << std::endl;
    std::cout << "b size=" << b->size() << std::endl;
    /*for (int i=0; i < a->size(); i++) {
        if ((*a)[i] != (*b)[i]) {
            std::cout << "diff i=" << i << " (*a)[i]" << (*a)[i] <<  " (*b)[i]" << (*b)[i] << std::endl;
        }
    }*/
    
    if (graph_sketch_after == graph_sketch) {
        std::cout << "GREAT!!!" << std::endl;
    } else {
        std::cout << "BAD!!!" << std::endl;
    }
#endif
    return 0;
}

#endif // ALL_DISTANCE_SKETCH_SRC_APP_SKETCH_CALCULATION_APP_H_