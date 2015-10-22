#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -std=c++0x -Wall -O3 -DNDEBUG -Wno-deprecated -Wno-strict-overflow -Wno-sign-compare -fPIC -ggdb -pthread

# define any directories containing header files other than /usr/include
#
INCLUDES = -I./include/boost_1_57_0 -I./include/Snap-2.3/snap-core  -I./include/Snap-2.3/snap-adv -I./include/Snap-2.3/glib-core   -I./include/Snap-2.3/snap-exp -I./include/sparsehash-2.0.2/usr/local/include -I./include/gpref/include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L./include/gpref/lib/ 

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lrt  -L./include/boost_1_57_0/boost/stage/lib
BOOST = -L./include/boost_1_57_0/boost/boost_build/lib/

# define the C source files
HDRS = $(all_distance_sketch/*/*.h)
COMMON = all_distance_sketch/common.h

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix

TEST_SRC = all_distance_sketch/algorithms/ut/t_skim_test.cpp all_distance_sketch/algorithms/ut/algo_test.cpp all_distance_sketch/sketch/ut/ads_test.cpp all_distance_sketch/graph/ut/basic_graph.cpp all_distance_sketch/algorithms/ut/t_skim_test.cpp

TESTS_LIBS = ./libgtest.a ./include/Snap-2.3/snap-core/Snap.o
TEST_INCLUDE = -isystem ./include/gtest/include
TEST_OBJS = ./out/all_distance_sketch/ut/run_all
#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean all test src_only

all:    $(TEST_OBJS)
	@echo  Thank you for building all distance sketch!

test: $(TEST_OBJS)
	@echo  Test distance sketch!

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

./out/all_distance_sketch/ut/run_all: all_distance_sketch/graph/ut/run_all.cpp $(OBJS) $(TEST_SRC) $(HDRS) $(COMMON)
	@mkdir -p out/all_distance_sketch/ut
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_INCLUDE) $(TEST_INCLUDE) -o $@  $(OBJS) $< $(BOOST) $(LIBS) $(TESTS_LIBS)

clean:
	$(RM) $(TEST_OBJS)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
