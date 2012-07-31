##=============================================================================
## Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##	        
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##=============================================================================

TESTNET2_SRC = \
          Test_NicList.cpp \
          Test_TcpTransport.cpp \
          Test_UdpTransport.cpp \
          Test_RawEncoder.cpp \
          Test_RawDecoder.cpp \
          Test_DataMessenger.cpp \
          Test_Resolver.cpp 

TESTNET2_INC = 

TESTNET2_OBJ1= $(TESTNET2_SRC:.cpp=.o)
TESTNET2_OBJ = $(addprefix net2/, $(TESTNET2_OBJ1))
TESTNET2_EXE = $(TESTNET2_OBJ:.o=.exe)
TESTNET2_DEP = $(TESTNET2_OBJ:.o=.d)
TESTNET2_ASM = $(TESTNET2_OBJ:.o=.asm)

INC += -I$(BOOST_INC)

net2/%.o: $(SAI_ROOT)/net2/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

net2/%.d: $(SAI_ROOT)/net2/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,net2/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

net2/%.exe: net2/%.o libSai.a
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) $< libSai.a $(SAIEXT_LIB) $(BOOST_LIB) $(XERCES_LIB) $(CRYPTOPP_LIB) -lcurl -lpthread

TESTNET2: $(TESTNET2_OBJ) $(TESTNET2_EXE)

include $(TESTNET2_DEP)
