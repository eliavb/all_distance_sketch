#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++
RM = rm -rf
# define any compile-time flags
CFLAGS += -std=c++0x -Wall -O3 -DNDEBUG -Wno-deprecated -Wno-strict-overflow -Wno-sign-compare -fPIC -ggdb -pthread

# define any directories containing header files other than /usr/include
#
INCLUDES = -I./include/boost_1_57_0 -I./include/Snap-2.3/snap-core  -I./include/Snap-2.3/snap-adv -I./include/Snap-2.3/glib-core   -I./include/Snap-2.3/snap-exp -I./include/sparsehash-2.0.2/usr/local/include -I./include/gpref/include

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lrt
HDRS = $(all_distance_sketch/*/*.h)
COMMON = all_distance_sketch/common.h

TEST_SRC = all_distance_sketch/algorithms/ut/t_skim_test.cpp all_distance_sketch/algorithms/ut/algo_test.cpp all_distance_sketch/sketch/ut/ads_test.cpp all_distance_sketch/graph/ut/basic_graph.cpp all_distance_sketch/algorithms/ut/t_skim_test.cpp
TESTS_LIBS = ./libgtest.a ./include/Snap-2.3/snap-core/Snap.o
TEST_INCLUDE = -isystem ./include/gtest/include

TEST_OBJS = ./out/all_distance_sketch/ut/run_all
PROTO = out/all_distance_sketch/proto

.PHONY: depend clean all test src_only

all: $(PROTO)
	@echo  Thank you for building all distance sketch!

test: $(TEST_OBJS)
	@echo  Test distance sketch!


out/all_distance_sketch/proto: all_distance_sketch/proto/all_distance_sketch.proto
	@mkdir -p out/all_distance_sketch/proto
	protoc --cpp_out=./out  all_distance_sketch/proto/all_distance_sketch.proto

./out/all_distance_sketch/ut/run_all: all_distance_sketch/graph/ut/run_all.cpp $(OBJS) $(TEST_SRC) $(HDRS) $(COMMON)
	@mkdir -p out/all_distance_sketch/ut
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_INCLUDE) $(TEST_INCLUDE) -o $@  $(OBJS) $< $(BOOST) $(LIBS) $(TESTS_LIBS)

clean:
	$(RM) $(TEST_OBJS) $(PROTO)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
