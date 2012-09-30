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

NET2_SRC = \
          Net.cpp \
          DataHandler.cpp \
          Transport.cpp \
          TcpTransport.cpp \
          UdpTransport.cpp \
          DataDescriptor.cpp \
          DataDispatcher.cpp \
          DataMessenger.cpp \
          DataMessengerFactory.cpp \
          RawDecoder.cpp \
          RawEncoder.cpp \
          Service.cpp \
          Nic.cpp \
          NicList.cpp \
          TimerTask.cpp \
          Resolver.cpp

NET2_INC = \
          $(SAI_ROOT)/net2/Net.h \
          $(SAI_ROOT)/net2/Transport.h \
          $(SAI_ROOT)/net2/DataHandler.h \
          $(SAI_ROOT)/net2/DataDescriptor.h \
          $(SAI_ROOT)/net2/DataDispatcher.h \
          $(SAI_ROOT)/net2/DataMessenger.h \
          $(SAI_ROOT)/net2/DataMessengerFactory.h \
          $(SAI_ROOT)/net2/RawDecoder.h \
          $(SAI_ROOT)/net2/RawEncoder.h \
          $(SAI_ROOT)/net2/Service.h \
          $(SAI_ROOT)/net2/Nic.h \
          $(SAI_ROOT)/net2/NicList.h \
          $(SAI_ROOT)/net2/TimerTask.h \
          $(SAI_ROOT)/net2/Resolver.h

NET2_OBJ1= $(NET2_SRC:.cpp=.o)
NET2_OBJ = $(addprefix net2/, $(NET2_OBJ1))
NET2_DEP = $(NET2_OBJ:.o=.d)
NET2_ASM = $(NET2_OBJ:.o=.asm)

SAI_OBJS += $(NET2_OBJ)
SAI_INC  += $(NET2_INC)

INC += -I$(BOOST_INC)

net2/%.o: $(SAI_ROOT)/net2/%.cpp
	$(CC) -o $@ $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $<

net2/%.d: $(SAI_ROOT)/net2/%.cpp
	@ set -e; rm -f $@; \
		$(CC) -M $(OPTS) $(DEF) $(INC) -I$(SAI_ROOT) -c $< > $@.d; \
		sed 's,\($*\)\.o[ :]*,net2/\1.o $@: ,g' < $@.d > $@; \
		rm -f $@.d

NET2: $(NET2_OBJ)

clean_net2:
	  @ find net2 -name '*.d' | xargs rm -f
	  @ find net2 -name '*.o' | xargs rm -f

include $(NET2_DEP)
