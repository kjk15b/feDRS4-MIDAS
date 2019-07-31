# Kolby Kiesling
# 07 / 30 / 2019
# kjk15b@acu.edu

DRS_PATH=/home/daq/src/drs-5.0.5
WXLIBS        = $(shell wx-config-3.0 --libs)
WXFLAGS       = $(shell wx-config-3.0 --cxxflags)
DEBUGFLAGS=-g -fpermissive -O -lcrypt
CFLAGS=$(DEBUGFLAGS) -Wall -Os -I$(MIDASSYS)/include -I$(MIDASSYS)/drivers/class -I$(MIDASSYS)/drivers/bus -fpermissive -std=c++11
CFLAGS+=$(WXLIBS) $(WXFLAGS)
CXXFLAGS=$(CFLAGS)
LDFLAGS=$(MIDASSYS)/linux/lib/mfe.o  -L$(MIDASSYS)/linux/lib -lmidas -lpthread -lutil -lrt -lz -lusb-1.0


all: feDRS4

rs232.o: $(MIDASSYS)/drivers/bus/rs232.cxx $(MIDASSYS)/drivers/bus/rs232.h
	g++ -c $(CFLAGS) $(MIDASSYS)/drivers/bus/rs232.cxx

multi.o: $(MIDASSYS)/drivers/class/multi.cxx $(MIDASSYS)/drivers/class/multi.h
	g++ -c $(CFLAGS) $(MIDASSYS)/drivers/class/multi.cxx
	
dd_drs4.o: dd_drs4.cxx dd_drs4.h DRS.h averager.h $(DRS_PATH)/libdrs4.a
	g++ $(CXXFLAGS) -c dd_drs4.cxx $(DRS_PATH)/libdrs4.a -I/minimal_build 

feDRS4: feDRS4.cc DRS.h averager.h $(DRS_PATH)/libdrs4.a
	g++ -o $@ $(CXXFLAGS) $^ $(LDFLAGS) $(DRS_PATH)/libdrs4.a $(DRS_PATH)/libdrs4.a -I.

clean:
	rm -f feDRS4 *.o

