##=============================================================================
## Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
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

UTILS_SRC = BinarySearch.cpp \
            BubbleSort.cpp \
            KDTreeSort.cpp \
            LinearSearch.cpp \
            List.cpp \
            Version.cpp \
            Crypto.cpp \
            Logger.cpp \
            ThreadPool.cpp \
            XmlReader.cpp

UTILS_INC = \
          $(SAI_ROOT)/utils/BinarySearch.h \
          $(SAI_ROOT)/utils/BubbleSort.h \
          $(SAI_ROOT)/utils/KDTreeSort.h \
          $(SAI_ROOT)/utils/LinearSearch.h \
          $(SAI_ROOT)/utils/List.h \
          $(SAI_ROOT)/utils/Searcher.h \
          $(SAI_ROOT)/utils/Sorter.h \
          $(SAI_ROOT)/utils/Types.h \
          $(SAI_ROOT)/utils/ThreadPool.h \
          $(SAI_ROOT)/utils/XmlReader.h

UTILS_OBJ1= $(UTILS_SRC:.cpp=.o)
UTILS_OBJ = $(addprefix utils/, $(UTILS_OBJ1))
UTILS_DEP = $(UTILS_OBJ:.o=.d)
UTILS_ASM = $(UTILS_OBJ:.o=.asm)

SAI_OBJS  += $(UTILS_OBJ)
SAI_INC   += $(UTILS_INC)

INC += -I$(CRYPTOPP_INC)
INC += -I$(XERCES_INC)

utils/%.o: $(SAI_ROOT)/utils/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

utils/%.d: $(SAI_ROOT)/utils/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,utils/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

UTILS: $(UTILS_OBJ)

clean_utils:
	  @ rm -f utils/*.o utils/*.d

include $(UTILS_DEP)
