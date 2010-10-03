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

AI_SRC = ml/clustering/Kmeans.cpp

AI_INC = $(SAI_ROOT)/ai/ml/clustering/Kmeans.h \
				 $(SAI_ROOT)/ai/ml/Configuration.h \
				 $(SAI_ROOT)/ai/Types.h

AI_OBJ1= $(AI_SRC:.cpp=.o)
AI_OBJ = $(addprefix ai/, $(AI_OBJ1))
AI_DEP = $(AI_OBJ:.o=.d)
AI_ASM = $(AI_OBJ:.o=.asm)

SAI_OBJS  += $(AI_OBJ)
SAI_INC   += $(AI_INC)

ai/ml/clustering/%.o: $(SAI_ROOT)/ai/ml/clustering/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

ai/ml/clustering/%.d: $(SAI_ROOT)/ai/ml/clustering/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,ai/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

ai/ml/%.o: $(SAI_ROOT)/ai/ml/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

ai/ml/%.d: $(SAI_ROOT)/ai/ml/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,ai/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

ai/%.o: $(SAI_ROOT)/ai/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

ai/%.d: $(SAI_ROOT)/ai/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,ai/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

AI: $(AI_OBJ)

clean_ai:
	  @ rm -f ai/*.o ai/*.d ai/ml/*.o ai/ml/*.d ai/ml/clustering/*.o ai/ml/clustering/*.d

include $(AI_DEP)
