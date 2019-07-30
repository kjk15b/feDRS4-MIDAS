# Kolby Kiesling

DRS_PATH=/home/daq/src/drs-5.0.5

WXLIBS        = $(shell wx-config-3.0 --libs)
WXFLAGS       = $(shell wx-config-3.0 --cxxflags)

all: drs_test


drs_test: drs_test.cpp $(DRS_PATH)/libdrs4.a
	g++ -std=c++11 $@.cpp -g -O -o $@ -lrt -lpthread -lcrypt -fpermissive -I. $(DRS_PATH)/libdrs4.a $(DRS_PATH)/libdrs4.a -lusb-1.0 $(WXFLAGS) $(WXLIBS)

clean:
	rm -f drs_test