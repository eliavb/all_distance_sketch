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
	
	The following documentation is correct to the day it was written. case you have problems with my instruction
	see the packages website for additional information.

	### Build Instructions for root users###
	All packages expect SNAP can be installed by running the following command from the directory you downloaded 
	and unpacked the package

		>./configure
  		>make
  		>make install
  		>make check
	
	To install SNAP just run the following command from the unpacked directory:

		>make all
	
	Since SNAP build is local you need to indicate its location for this package build.
	You do it using the config.site file. See non root users and only place the details that are relevant to SNAP.

	### Build Instructions for non root users###
	
	The standard way to install packages when you don't have root privileges it to use the --prefix 
	option when running the ./configure 

		>./configure --prefix=<path>
	
	The above command will cause the package to be installed in the path given in the --prefix option.
	SNAP doesn't require any additional option and just type

		>make all 

	From the directory you unpacked the source.
	Since you installed the packages in you desired location you need to list them in a file that will allow
	the package to find them and link with them and this is where config.site comes into play.
	After you installed Gtest (in ${GTEST_DIR}), SNAP (in ${SNAP_DIR}) and PROTOBUF (in ${PROTO_DEST})
	create a file named config.site (I ommit Boost but if you need to install it just do the same as the others).

	The file should contain the location of SNAP and Gtest:

	#### config.site content ####


  		CPPFLAGS="${CPPFLAGS} -I${GTEST_DIR}/include/ -I{SNAP_DIR}/snap-adv/ \
  		-I{SNAP_DIR}/snap-core/ -I{SNAP_DIR}/snap-exp/ -I{SNAP_DIR}/glib-core/ -I${PROTO_DEST}/include/" 
  		LIBS="${LIBS} {SNAP_DIR}/snap-core/Snap.o ${GTEST_DIR}/libgtest.a -lpthread"
  		LDFLAGS="${LDFLAGS} -L${PROTO_DEST}/lib"
		PROTOC="${PROTO_DEST}/bin/protoc"
		PKG_CONFIG_PATH="${PROTO_DEST}/lib/pkgconfig/" 
	
	
	#### Example for a config.site ####
		

		CPPFLAGS="${CPPFLAGS} -I/work/eng/eliavb/googletest-release-1.7.0/include/ -I/work/eng/eliavb/Snap-2.3/snap-adv/ -I/work/eng/eliavb/Snap-2.3/snap-core/ -I/work/eng/eliavb/Snap-2.3/snap-exp/ -I/work/eng/eliavb/Snap-2.3/glib-core/ -I/work/eng/eliavb/protobuf_install/include/" 
		LIBS="${LIBS} /work/eng/eliavb/Snap-2.3/snap-core/Snap.o /work/eng/eliavb/googletest-release-1.7.0/libgtest.a -lpthread"
		LDFLAGS="${LDFLAGS} -L/work/eng/eliavb/protobuf_install/lib/"
		PROTOC="/work/eng/eliavb/protobuf_install/bin/protoc"

	
	Case you enounter:

		configure: error: Package requirements (protobuf >= 2.6.0) were not met:

		No package 'protobuf' found

		Consider adjusting the PKG_CONFIG_PATH environment variable if you
		installed software in a non-standard prefix.
	
	Try changing the package config env variable using the following command:

		export PKG_CONFIG_PATH='${PROTO_DEST}/lib/pkgconfig/'	
	
	Case you encounter:

		error while loading shared libraries: libprotobuf.so.9: cannot open shared object file: No such file or directory

	Try:
	
		export LD_LIBRARY_PATH="$LD_LIBRAY_PATH:${PROTO_DEST}/lib/"	

	#### CONFIG_SITE ####
	After the file was created we need to set an env variable named 
	CONFIG_SITE to point to the config.site:
  		
  		>export CONFIG_SITE=$CONFIG_SITE:<path>/config.site
	
	#### BUILD! non root users ####
	Suppose you placed the package in ${ADS_DIR} and want to install it in ${ADS_DEST}.

	Enter to the ${ADS_DIR} and run:


  		>./configure --prefix=${ADS_DEST}
  		>make
  		>make install
	
  	### Checking the install was a success non root users ###
  	Since this is not a root install we need to add the location of the proto lib
  		
  		>export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${PROTO_DEST}/lib
  		>make check

	#### BUILD! root users ####
	Suppose you placed the package in ${ADS_DIR}.

	Enter to the ${ADS_DIR} and run:


  		>./configure
  		>make
  		>make install
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
	You can run the algorithms as apps. When building the pakcage the apps will be located in the ${ADS_DEST}/bin.
	run --help to see the options that each such app can take.
	The application that calculates the all distance sketch:

		>./bin/sketch_app --help
		Usage: options_description [options]
		Allowed options:
		  --help                 produce help message
		  --K arg                K = 1/epsilon^2 sets the precision
		  --num_threads arg (=1) num_threads to use
		  --directed arg         is the graph directed
		  --graph_dir arg        Directory with the graph to calculate the sketch on
		  --output_file arg      output file path pattern, here the sketch Gpb will be saved with the 
		  						 prefix entered and suffix _[%d] 
		  						 (Gpb defined in src/proto/all_distance_sketch.proto)

	App for calculating the reverse ranks of a single node.

		>bin/reverse_rank_app --help
			Allowed options:
		  --help                 produce help message
		  --source_id arg        id of source node
		  --K arg                K = 1/epsilon^2 sets the precision
		  --num_threads arg (=1) num_threads to use
		  --directed arg         is the graph directed
		  --graph_dir arg        Directory with the graph to calculate the sketch on
		  --sketch_file arg      File prefix with the calculated sketch. The prefix should match what you entered in the sketch_app 
		  --output_file arg      output file path, here the sketch Gpb will be saved 
		                         (Gpb defined in src/proto/all_distance_sketch.proto)

	
	App for calculating the exact reverse rank greedy cover.


		>bin/t_skim_exact_app --help
			Allowed options:
		  --help                produce help message
		  --T arg               Defines the influence of a node. If pi_{us} < T then s 
		                        coveres u
		  --directed arg        is the graph directed
		  --graph_dir arg       Directory with the graph to calculate the sketch on
		  --output_file arg     output file path, here the cover will be saved in Gpb 
		                        format (Gpb defined in src/proto/cover.proto)
	
	
	App for calculating the approximate reverse rank greedy cover using TSkim.

		>bin/t_skim_reverse_rank_app --help
			Usage: options_description [options]
			Allowed options:
		  --help                           produce help message
		  --T arg                          Defines the influence of a node. If pi_{us} 
		                                   < T then s coveres u
		  --K arg                          K = 1/epsilon^2 sets the precision (Only relevant if you don't provide sketch_file option)
		  --num_threads arg (=1)           num_threads to use (Only relevant if you don't provide sketch_file option)
		  --min_influence_for_seed_set arg min influence to enter to the seed set
		  --directed arg                   is the graph directed
		  --graph_dir arg                  Directory with the graph to calculate the 
		                                   sketch on
		  --sketch_file arg                File prefix with the calculated sketch. The prefix should match what you entered in the sketch_app
		  --output_file arg                output file path, here the cover will be 
		                                   saved in Gpb format (Gpb defined in 
		                                   src/proto/cover.proto)

*/

/*!
	\page Overview Overview over the algorithms

	\section ADS All distance sketch

	\ref all_distance_sketch::GraphSketch

	\section RRank Reverse rank
	
	\ref all_distance_sketch::CalculateReverseRank

	\section TSkim TSkim influence maximization

	\ref all_distance_sketch::TSkimReverseRank

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

	\page TSkimTut Calculating TSkim influence maximization
	How to calculate TSkim influence maximization based on forward ranks.
	\ref examples/t_skim.cpp
 */
