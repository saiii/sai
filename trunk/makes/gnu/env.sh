# SAI [ 8 Feb 2011 ]
export BOOST_INC=/home/athip/dev/boost_1_50_0/BIN/include
export BOOST_LIB=/home/athip/dev/boost_1_50_0/BIN/lib/libboost_system.a
export BOOST_LIB="$BOOST_LIB /home/athip/dev/boost_1_50_0/BIN/lib/libboost_regex.a"
export CRYPTOPP_INC=/home/athip/dev/cryptopp/BIN/include/cryptopp
export CRYPTOPP_LIB=/home/athip/dev/cryptopp/BIN/lib/libcryptopp.a
export XERCES_INC=/home/athip/dev/xerces-c-3.1.1/BIN/include
export XERCES_LIB=/home/athip/dev/xerces-c-3.1.1/BIN/lib/libxerces-c.a

MJR=`grep VERSION_MAJOR ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`
MNR=`grep VERSION_MINOR ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`
CNT=`grep VERSION_COUNT ../../src/utils/Version.cpp | grep define | cut -d' ' -f 3 | cut -d\" -f 2`

echo export VERSION_MAJOR=$MJR
echo export VERSION_MINOR=$MNR
echo export VERSION_COUNT=$CNT
export VERSION_MAJOR=$MJR
export VERSION_MINOR=$MNR
export VERSION_COUNT=$CNT

