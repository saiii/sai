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

NET_SRC = Socket.cpp \
          Exception.cpp \
          Net.cpp \
          DataBus.cpp \
          DataDescriptor.cpp \
          ProtocolDecoder.cpp \
          ProtocolEncoder.cpp \
          DataChainable.cpp \
          DataDispatchable.cpp \
          DataBusChannel.cpp \
          DataBusState.cpp \
          TimerTask.cpp \
          DataOrderingManager.cpp

NET_INC = \
          $(SAI_ROOT)/net/Socket.h \
          $(SAI_ROOT)/net/Exception.h \
          $(SAI_ROOT)/net/Net.h

NET_OBJ1= $(NET_SRC:.cpp=.o)
NET_OBJ = $(addprefix net/, $(NET_OBJ1))
NET_DEP = $(NET_OBJ:.o=.d)
NET_ASM = $(NET_OBJ:.o=.asm)

SAI_OBJS += $(NET_OBJ)
SAI_INC  += $(NET_INC)

INC += -I$(BOOST_INC)

net/%.o: $(SAI_ROOT)/net/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

net/%.d: $(SAI_ROOT)/net/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,net/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

NET: $(NET_OBJ)

clean_net:
	  @ rm -f net/*.o net/*.d

include $(NET_DEP)
