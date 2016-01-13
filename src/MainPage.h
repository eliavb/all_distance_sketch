/*! \mainpage All distance sketch
 
	\section Background
 
	This package was designed and implemented by Eliav Buchnik as part of a M.Sc in Computer Science at  <a href="https://english.tau.ac.il/"> 
	Tel Aviv University </a>, Israel under Prof. <a href="http://www.cohenwang.com/edith/">Edith Cohen</a>.
 	
	\section intro_sec Introduction
	Collection of randomized algorithm based on the all distance sketch (http://arxiv.org/abs/1306.3284). The algorithms are compatible to Linux based platforms.

	Package contains the following algorithms:
	1. Bottom-k sketch (http://arxiv.org/abs/1306.3284)
	2. Reverse rank (http://arxiv.org/abs/1506.02386)
	3. TSkim algorithm based on reverse rank (http://arxiv.org/pdf/1506.02386v1.pdf)
	
	\section Prerequisites
	The package is designed to have fairly minimal requirements to build
	and use with your projects, but there are some. Currently, we support
	Linux based paltforms.

	### Linux Requirements ###

	These are the base requirements to build and use Google Test from a source
	package (as described below):
	  1. GNU-compatible Make
	  2. GCC compiler (version >= 4.8.3)
	  3. SNAP (https://snap.stanford.edu/snap/ version >= 2.3)
	  4. Boost (http://www.boost.org/ version >= 1.58)
	  5. Gtest (https://github.com/google/googletest)

	\section install_sec Installation
	
	### Build Instructions for root users###
	After installing all the other packages as root.

		>./configure
  		>make
  		>make install
  		>make check

	### Build Instructions for non root users###

	After you installed Gtest (in ${GTEST_DIR}) and SNAP (in ${SNAP_DIR}) 
	create a file named config.site (I ommit Boost but if you need to install it just do the same as the others).

	The file should contain the location of SNAP and Gtest:

	#### config.site content ####


  		CPPFLAGS="${CPPFLAGS} -I${GTEST_DIR}/include/ -I{SNAP_DIR}/snap-adv/ \
  		-I{SNAP_DIR}/snap-core/ -I{SNAP_DIR}/snap-exp/ -I{SNAP_DIR}/glib-core/" 
  		LIBS="${LIBS} {SNAP_DIR}/snap-core/Snap.o ${GTEST_DIR}/libgtest.a -lpthread" 

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
  		>make check
	
	\section Package
	## Using the package as library ##
	To use the package in your code you must include ${ADS_DEST}/include/all_distance_sketch.h
	and link with ${ADS_DEST}/lib/liball_distance_sketch.a

	Compiling test.cpp:


		>g++ --std=c++11  test.cpp -I${SNAP_DEST}/snap-adv/ -I${SNAP_DEST}/snap-core/ -I${SNAP_DEST}/snap-exp/ -I${SNAP_DEST}/glib-core/  ${ADS_DEST}/lib/liball_distance_sketch.a ${SNAP_DEST}/snap-core/Snap.o  -o test
	
	## Run algorithms as binary ##
	TODO
 */