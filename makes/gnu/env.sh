# SAI [ 8 Feb 2011 ]
export BOOST_INC=/home/athip/Dev/boost_1_46_1/BIN/include
export BOOST_LIB=/home/athip/Dev/boost_1_46_1/BIN/lib/libboost_system.a

MJR=`grep VERSION_MAJOR ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`
MNR=`grep VERSION_MINOR ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`
CNT=`grep VERSION_COUNT ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`

export VERSION_MAJOR=$MJR
export VERSION_MINOR=$MNR
export VERSION_COUNT=$CNT

