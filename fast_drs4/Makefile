#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#
#  Contents:     Makefile for MIDAS example frontend and analyzer
#
#  $Id$
#
#####################################################################
#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

# get OS type from shell
OSTYPE = $(shell uname)

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#-----------------------------------------
# This is for Linux
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

ifeq ($(OSTYPE),linux)
OS_DIR = linux
OSFLAGS = -DOS_LINUX -Dextname
CFLAGS = -g -O2 -Wall
# add to compile in 32-bit mode
# OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

#-----------------------
# MacOSX/Darwin is just a funny Linux
#
ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),darwin)
OS_DIR = darwin
FF = cc
OSFLAGS = -DOS_LINUX -DOS_DARWIN -DHAVE_STRLCPY -fPIC -Wno-unused-function
LIBS = -lpthread -lz
NEED_STRLCPY=
NEED_RANLIB=1
NEED_SHLIB=
NEED_RPATH=

endif

#-----------------------------------------
# ROOT flags and libs
#
ifdef ROOTSYS
ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
# ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib64/root
ROOTLIBS   += -lThread
else
missroot:
	@echo "...";
	@echo "Missing definition of environment variable 'ROOTSYS' !";
	@echo "...";
endif
#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
INC_DIR   = $(MIDASSYS)/include
DRV_DIR   = $(MIDASSYS)/drivers
LIB_DIR   = $(MIDASSYS)/lib
SRC_DIR   = $(MIDASSYS)/src

#-------------------------------------------------------------------
# Extra includes, to make the DRS4 function...
#
WXLIBS        = $(shell wx-config-3.0 --libs)
WXFLAGS       = $(shell wx-config-3.0 --cxxflags)
DRSPATH	      = /home/daq/src/drs-5.0.5
#-------------------------------------------------------------------
# List of analyzer modules
#
MODULES   = adccalib.o adcsum.o scaler.o

#-------------------------------------------------------------------
# Frontend code name defaulted to frontend in this example.
# comment out the line and run your own frontend as follow:
# gmake UFE=my_frontend
#
UFE = frontend

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = g++ -DNEED_NO_EXTERN_C
CXX = g++
CFLAGS += -g -I$(INC_DIR) -I$(DRV_DIR)
LDFLAGS +=
LDDFLAGS=$(DRSPATH)/libdrs4.a $(MIDASSYS)/linux/lib/mfe.o -L$(MIDASSYS)/linux/lib -lmidas -lpthread -lutil -lrt -lz -lusb-1.0
CFLAGS+=$(DEBUGFLAGS) -Wall -Os -I$(MIDASSYS)/include -I$(MIDASSYS)/drivers/class -I$(MIDASSYS)/drivers/bus -fpermissive -std=c++11
CFLAGS+=$(WXLIBS) $(WXFLAGS)
CXXFLAGS=$(CFLAGS)


all:analyzer frontend


analyzer: $(LIB) $(LIB_DIR)/rmana.o analyzer.o $(MODULES)
	$(CXX) $(CFLAGS) -o $@ $(LIB_DIR)/rmana.o analyzer.o $(MODULES) \
	$(LIB) $(LDFLAGS) $(ROOTLIBS) $(LIBS)

%.o: %.cxx experim.h DRS.h averager.h $(DRSPATH)/libdrs4.a
	$(CXX) $(USERFLAGS) $(ROOTCFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

frontend: frontend.cxx  DRS.h averager.h
	g++ -o $@ $(CXXFLAGS) $^ $(LDDFLAGS)
	
clean::
	rm -f $(UFE) analyzer *.o *~ \#*

#end file
