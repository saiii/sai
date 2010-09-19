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

MATH_SRC = Matrix.cpp

MATH_INC = $(SAI_ROOT)/math/Matrix.h \
					 $(SAI_ROOT)/math/Types.h

MATH_OBJ1= $(MATH_SRC:.cpp=.o)
MATH_OBJ = $(addprefix math/, $(MATH_OBJ1))
MATH_DEP = $(MATH_OBJ:.o=.d)
MATH_ASM = $(MATH_OBJ:.o=.asm)

SAI_OBJS  += $(MATH_OBJ)
SAI_INC   += $(MATH_INC)

math/%.o: $(SAI_ROOT)/math/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

math/%.d: $(SAI_ROOT)/math/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,math/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

MATH: $(MATH_OBJ)

clean_math:
	  @ rm -f math/*.o math/*.d

include $(MATH_DEP)
