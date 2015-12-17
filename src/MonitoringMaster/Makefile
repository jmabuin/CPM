CC=			g++
CFLAGS=			-std=c++0x -g -Wall -Wno-unused-function -O2 
WRAP_MALLOC=		-DUSE_MALLOC_WRAPPERS
DFLAGS=			-DHAVE_PTHREAD $(WRAP_MALLOC)
PROG=			MonitoringMaster

.SUFFIXES:.cpp .o .cc

.cpp.o:
	$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)
	
MonitoringMaster: MonitoringMaster.o Network.o
	$(CC) $(CFLAGS) $(DFLAGS) $(AOBJS) MonitoringMaster.o Network.o -o $@
	
clean:
	rm $(PROG) *.o
