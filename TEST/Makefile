# Kolby Kiesling
# 07 / 27 / 2019
# kjk15b@acu.edu
DRSDIR=/home/daq/src/drs-5.0.5
DEBUGFLAGS=-g -fpermissive
WXLIBS        = $(shell wx-config-3.0 --libs)
WXFLAGS       = $(shell wx-config-3.0 --cxxflags)
#DEBUGFLAGS=
CFLAGS=$(DEBUGFLAGS) -Wall -Os -I$(MIDASSYS)/include -I$(MIDASSYS)/drivers/class -I$(MIDASSYS)/drivers/bus -fpermissive -std=c++11 -I$(DRSDIR)
CXXFLAGS=$(CFLAGS)
LDFLAGS=$(MIDASSYS)/linux/lib/mfe.o  -L$(MIDASSYS)/linux/lib -lmidas -lpthread -lutil -lrt -lz $(WXLIBS)
DRSOBJ=DRS.o averager.o ConfigDialog.o DOFrame.o DOScreen.o DRSOsc.o MeasureDialog.o Measurement.o Osci.o InfoDialog.o DisplayDialog.o AboutDialog.o EPThread.o TriggerDialog.o rb.o  
CPP_OBJ=musbstd.o mxml.o strlcpy.o

all: feDRS4

rs232.o: $(MIDASSYS)/drivers/bus/rs232.cxx $(MIDASSYS)/drivers/bus/rs232.h
	g++ -c $(CFLAGS) $(MIDASSYS)/drivers/bus/rs232.cxx

multi.o: $(MIDASSYS)/drivers/class/multi.cxx $(MIDASSYS)/drivers/class/multi.h
	g++ -c $(CFLAGS) $(MIDASSYS)/drivers/class/multi.cxx

$(DRSOBJ): %.o: $(DRSDIR)/src/%.cpp $(DRSDIR)/include/%.h $(DRSDIR)/include/mxml.h $(DRSDIR)/include/DRS.h
	g++ $(CXXFLAGS) $(WXFLAGS) -c $<

$(CPP_OBJ): %.o: $(DRSDIR)/src/%.cpp $(DRSDIR)/include/%.h $(DRSDIR)/include/mxml.h $(DRSDIR)/include/DRS.h
	g++ $(CXXFLAGS) -c $<
	
dd_drs4.o: dd_drs4.cxx dd_drs4.h $(DRSDIR)/include/DRS.h
	g++ $(DRSOBJ) $(CPP_OBJ) $(CXXFLAGS) -c dd_drs4.cxx

feDRS4: feDRS4.cc rs232.o multi.o dd_drs4.o # something...
	g++ -o $@ $(CXXFLAGS) $^ $(LDFLAGS)
clean:
	rm -f *.o feDRS4
