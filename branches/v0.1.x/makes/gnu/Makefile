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

CC   = g++
OPTS = -g -Wall -fPIC
INC  = -I.
DEF  =
LIBS =

SAI_INC  =
SAI_OBJS =

SAI_ROOT = ../../src

all: LIBS TEST

include math/Index.mk
include ai/Index.mk
include utils/Index.mk
include net/Index.mk
include test/Index.mk

LIBS: $(SAI_OBJS) libSai.a libSai.so

libSai.so: $(SAI_OBJS)
	$(CC) -fPIC -shared -o $@ $(SAI_OBJS)
	@ cp $@ $@.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_COUNT)

libSai.a: $(SAI_OBJS)
	ar rs $@ $(SAI_OBJS)

clean: clean_math clean_test clean_ai clean_utils clean_net
	  @ rm -Rf lib* *.o

count:
	  @ find . -name '*.*' | grep '\.\(cpp\|h\)$$' | xargs wc -l

