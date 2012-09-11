##=============================================================================
## Copyright (C) 2010 Athip Rooprayochsilp <athipr@gmail.com>
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

TEST_SRC = MathMatrix.cpp \
           Utils.cpp \
           KDTree.cpp \
           MathVector.cpp \
           MathEigenSolver.cpp \
           AiMlKmeans.cpp \
           AiMlFuzzyCmeans.cpp \
           MathUtils.cpp \
           NetTcp.cpp \
           NetUdp.cpp \
           Crypto.cpp \
           NetUdpDirect.cpp \
           DataBus.cpp

TEST_INC = 
SAI_LIB  = libSai.a 
LIBS    += $(BOOST_LIB)
LIBS    += $(CRYPTOPP_LIB)
LIBS    += $(OPGM_LIB)
LIBS    += -lpthread
LIBS    += -lrt

TEST_OBJ1= $(TEST_SRC:.cpp=.o)
TEST_OBJ = $(addprefix test/, $(TEST_OBJ1))
TEST_BIN = $(TEST_OBJ:.o=)
TEST_DEP = $(TEST_OBJ:.o=.d)
TEST_ASM = $(TEST_OBJ:.o=.asm)

test/%: test/%.o $(SAI_LIB)
	$(CC) -o $@ $(OPTS) $< libSai.a $(LIBS) $(BOOST_LIBS)

test/%.o: $(SAI_ROOT)/test/%.cpp $(SAI_LIB)
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

test/%.d: $(SAI_ROOT)/test/%.cpp $(SAI_LIB)
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,test/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

TEST: $(TEST_OBJ) $(TEST_BIN) TESTNET2

clean_test:
	  @ rm -f test/*.o test/*.d $(TEST_BIN)

include $(TEST_DEP)
