# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -std=c++0x -Wall -O3 -DNDEBUG -Wno-deprecated -Wno-strict-overflow -Wno-sign-compare -fPIC -ggdb -pthread
AM_CPPFLAGS += -I$(builddir)

AM_LDFLAGS = -lboost_system$(BOOST_LIB_SUFFIX) -lprotobuf
# AM_CPPFLAGS = -I./include/gtest

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
HDRS = $(wildcard ./*/*.h) $(wildcard ./*.h)
TEST_SRC = $(wildcard ./*/ut/*.cpp)
bin_PROGRAMS = test_all # estimate benchmark test_all
#estimate_SOURCES = experiments/results/estimate.cpp $(HDRS)
#benchmark_SOURCES = experiments/results/benchmark.cpp $(HDRS)
test_all_SOURCES = graph/ut/run_all.cpp $(HDRS) $(TEST_SRC) $(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h
lib_LIBRARIES = liball_distance_sketch.a
liball_distance_sketch_a_SOURCES = $(HDRS) $(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h
include_HEADERS = ./all_distance_sketch.h
BUILT_SOURCES = $(builddir)/proto/all_distance_sketch.pb.h $(builddir)/proto/all_distance_sketch.pb.cc

# Build proto files first
$(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h: $(srcdir)/proto/all_distance_sketch.proto
	@$(PROTOC) --cpp_out=$(builddir) $(srcdir)/proto/all_distance_sketch.proto

dist_noinst_DATA = $(wildcard ./proto/*)
MOSTLYCLEANFILES = $(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h