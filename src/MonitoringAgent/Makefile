CC=			g++
CFLAGS=			-std=c++0x -g -Wall -Wno-unused-function -O2 
WRAP_MALLOC=		-DUSE_MALLOC_WRAPPERS
DFLAGS=			-DHAVE_PTHREAD $(WRAP_MALLOC)

PROG=			MonitoringAgent

PROC_LIBS = $(shell find /lib/ -name "*libprocps*")
PROC_LIBS2 = $(shell find /lib/ -name "*libproc*")

ifneq (, $(findstring procps, $(PROC_LIBS)))
	LIBS=			-lprocps -lpthread
else ifneq (, $(findstring proc, $(PROC_LIBS2)))
	LIBS=			-lproc -lpthread
endif

#To check the compiler version and use C++11
empty:=
space:= $(empty) $(empty)

TEN_THOUSAND = 10000
HUNDRED = 100

GCCVERSIONSTRING = $(shell expr `$(CC) -dumpversion`)

GCCVERSION_FIRST = $(shell expr `echo $(GCCVERSIONSTRING)` | cut -f1 -d.)
GCCVERSION_FIRST = $(shell expr $(GCCVERSION_FIRST) \* $(TEN_THOUSAND))

GCCVERSION_SECOND = $(shell expr `echo $(GCCVERSIONSTRING)` | cut -f2 -d.)
GCCVERSION_SECOND = $(shell expr $(GCCVERSION_SECOND) \* $(HUNDRED))

GCCVERSION_THIRD = $(shell expr `echo $(GCCVERSIONSTRING)` | cut -f3 -d.)

GCCVERSION = $(shell expr $(GCCVERSION_FIRST) \+ $(GCCVERSION_SECOND) \+ $(GCCVERSION_THIRD))


.SUFFIXES:.cpp .o .cc

.cpp.o:
	$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)
	
MonitoringAgent: MonitoringAgent.o Network.o
	$(CC) $(CFLAGS) $(DFLAGS) $(AOBJS) MonitoringAgent.o Network.o -o $@  $(LIBS)
	
clean:
	rm $(PROG) *.o
