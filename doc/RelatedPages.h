/*!
	\page Install Installation

	\section Prerequisites
	The package is designed to have fairly minimal requirements to build
	and use with your projects, but there are some. Currently, we support
	Linux based paltforms.

	### Linux Requirements ###

	These are the base requirements to build and use the package (as described below):
	  1. GNU-compatible Make
	  2. GCC compiler (version >= 4.8.3)
	  3. SNAP (https://snap.stanford.edu/snap/ version >= 2.3)
	  4. Boost (http://www.boost.org/ version >= 1.58)
	  5. Gtest (https://github.com/google/googletest)
	  6. Protobuf compiler (https://developers.google.com/protocol-buffers/docs/downloads version >= 2.6.0)

	\section install_sec Installation
	
	### Build Instructions for root users###
	After installing all the other packages as root.

		>./configure
  		>make
  		>make install
  		>make check

	### Build Instructions for non root users###

	After you installed Gtest (in ${GTEST_DIR}), SNAP (in ${SNAP_DIR}) and PROTOBUF (in ${PROTO_DEST})
	create a file named config.site (I ommit Boost but if you need to install it just do the same as the others).

	The file should contain the location of SNAP and Gtest:

	#### config.site content ####


  		CPPFLAGS="${CPPFLAGS} -I${GTEST_DIR}/include/ -I{SNAP_DIR}/snap-adv/ \
  		-I{SNAP_DIR}/snap-core/ -I{SNAP_DIR}/snap-exp/ -I{SNAP_DIR}/glib-core/" 
  		LIBS="${LIBS} {SNAP_DIR}/snap-core/Snap.o ${GTEST_DIR}/libgtest.a -lpthread"
  		LDFLAGS="${LDFLAGS} -L${PROTO_DEST}/lib"
		PROTOC="${PROTO_DEST}/bin/protoc"
		PKG_CONFIG_PATH="${PROTO_DEST}/lib/pkgconfig/" 

	#### CONFIG_SITE ####
	After the file was created we need to set an env variable named 
	CONFIG_SITE to point to the config.site:
  		
  		>export CONFIG_SITE=$CONFIG_SITE:<path>/config.site
	
	#### BUILD! ####
	Suppose you placed the package in ${ADS_DIR} and want to install it in ${ADS_DEST}.

	Enter to the ${ADS_DIR} and run:


  		>./configure --prefix=${ADS_DEST}
  		>make
  		>make install

  	### Checking the install was a success ###
  	Since this is not a root install we need to add the location of the proto lib
  		
  		>export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${PROTO_DEST}/lib
  		>make check


*/

/*!
	\page usage Usage
	## Integrating with your code ##
	To use the package in your code you must include ${ADS_DEST}/include/all_distance_sketch.h
	and link with ${ADS_DEST}/lib/liball_distance_sketch.a

	Compiling your_code.cpp:


		>g++ --std=c++11  your_code.cpp -I${SNAP_DEST}/snap-adv/ -I${SNAP_DEST}/snap-core/ -I${SNAP_DEST}/snap-exp/ -I${SNAP_DEST}/glib-core/  ${ADS_DEST}/lib/liball_distance_sketch.a ${SNAP_DEST}/snap-core/Snap.o  -o your_code
	
	## Using as command line app ##
	TODO
*/

/*!
	\page Overview Overview over the algorithms

	\section ADS All distance sketch

	\ref all_distance_sketch::GraphSketch

	\section RRank Reverse rank
	
	\ref all_distance_sketch::CalculateReverseRank

	\section TSkim TSkim influence maximization

*/

/*!
	\page GraphTut Loading your graph
	 
	\subpage Building a graph
	
	The first step is to build a Graph with your data. Visit the example below to understand
	how to build a directed/undirected graph.

	\ref examples/graph.cpp

	\page GraphSketchTut Calculating GraphSketch
	After you have loaded your Graph you can calculate the graph sketch.

	\ref examples/sketch.cpp
	
	\page ReverseRankTut Calculating single node reverse rank
	How to calculate a single node reverse rank.
	\ref examples/reverse_rank.cpp
 */