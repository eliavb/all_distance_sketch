# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -std=c++0x -Wall -O3 -DNDEBUG -Wno-deprecated -Wno-strict-overflow -Wno-sign-compare -Wno-write-strings -fPIC -ggdb -pthread
AM_CPPFLAGS += -I$(builddir)

AM_LDFLAGS = -lboost_system$(BOOST_LIB_SUFFIX) -lboost_program_options -lprotobuf
# AM_CPPFLAGS = -I./include/gtest

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
HDRS = $(wildcard ./*/*.h) $(wildcard ./*.h)
TEST_SRC = $(wildcard ./*/ut/*.cpp)
bin_PROGRAMS = test_all sketch_app reverse_rank_batch_app reverse_rank_app t_skim_reverse_rank_app t_skim_exact_app dijkstra_app 
PROTOS_TO_BUILD = $(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h $(builddir)/proto/ranking.pb.h $(builddir)/proto/ranking.pb.cc $(builddir)/proto/cover.pb.h $(builddir)/proto/cover.pb.cc 
ALL = $(HDRS) $(TEST_SRC) $(PROTOS_TO_BUILD)
test_all_SOURCES = graph/ut/run_all.cpp $(ALL)
sketch_app_SOURCES = app/sketch_calculation_app.cpp $(ALL)
reverse_rank_batch_app_SOURCES = app/reverse_rank_batch_app.cpp $(ALL)
reverse_rank_app_SOURCES = app/reverse_rank_app.cpp $(ALL)
t_skim_reverse_rank_app_SOURCES = app/t_skim_reverse_rank_app.cpp $(ALL)
t_skim_exact_app_SOURCES = app/t_skim_exact_app.cpp $(ALL)
dijkstra_app_SOURCES = app/dijkstra_app.cpp $(ALL)
lib_LIBRARIES = liball_distance_sketch.a
liball_distance_sketch_a_SOURCES = $(HDRS) $(PROTOS_TO_BUILD)
include_HEADERS = ./all_distance_sketch.h
BUILT_SOURCES = $(PROTOS_TO_BUILD)

# Build proto files first
$(builddir)/proto/all_distance_sketch.pb.cc $(builddir)/proto/all_distance_sketch.pb.h: $(srcdir)/proto/all_distance_sketch.proto
	@$(PROTOC) --cpp_out=$(builddir) --python_out=$(builddir) $(srcdir)/proto/all_distance_sketch.proto
$(builddir)/proto/ranking.pb.h $(builddir)/proto/ranking.pb.cc: $(srcdir)/proto/ranking.proto
	@$(PROTOC) --cpp_out=$(builddir) --python_out=$(builddir) $(srcdir)/proto/ranking.proto
$(builddir)/proto/cover.pb.cc $(builddir)/proto/cover.pb.h: $(srcdir)/proto/cover.proto
	@$(PROTOC) --cpp_out=$(builddir) --python_out=$(builddir) $(srcdir)/proto/cover.proto

dist_noinst_DATA = $(wildcard ./proto/*)
MOSTLYCLEANFILES = $(PROTOS_TO_BUILD)
