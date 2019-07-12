CXX ?= g++
CC ?= gcc
MAKE ?= make
CPPFLAGS += -std=c++11
EXE = gen delta-gen delta-gen-uw delta-gen-ce
COMMON = common.h common.o

DBGFLAGS = $(CPPFLAGS) -g
RELFLAGS = $(CPPFLAGS) -O2

#export CXX CC CFLAGS CPPFLAGS OPROFILE

all: release 

#%.exe: %.o common.o
#	$(CXX) $(CPPFLAGS) $^ -o $@

#common.o: ${COMMON}
#	$(CXX) $(CPPFLAGS) -c $^ -o $@

gen.exe: gen.o ${COMMON}
	$(CXX) $(CPPFLAGS) $^ -o $@

delta-gen.exe: delta-gen.o ${COMMON}
	$(CXX) $(CPPFLAGS) ${^:%.h=} -o $@
	
delta-gen-uw.exe: delta-gen-uw.o ${COMMON}
	$(CXX) $(CPPFLAGS) $^ -o $@
	
delta-gen-ce.exe: delta-gen-ce.o ${COMMON}
	$(CXX) $(CPPFLAGS) ${^:%.h=} -o $@

release: 
	$(MAKE) $(EXE:=.exe) CPPFLAGS="$(RELFLAGS)"

debug: 
	$(MAKE) $(EXE:=.exe) CPPFLAGS="$(DBGFLAGS)"

clean:
	rm -rf *.o
	rm -rf *.exe


