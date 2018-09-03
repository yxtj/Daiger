CXX ?= g++
CC ?= gcc
MAKE ?= make
CPPFLAGS += -std=c++11
EXE = gen gen2 delta-gen delta-gen2 delta-gen-uw delta-gen-ce
COMMON = common.h common.o

DBGFLAGS = $(CPPFLAGS) -g
RELFLAGS = $(CPPFLAGS) -O2

#export CXX CC CFLAGS CPPFLAGS OPROFILE

all: release 

#%.exe: %.o common.o
#	$(CXX) $(CPPFLAGS) $^ -o $@

gen.exe: gen.o
	$(CXX) $(CPPFLAGS) $^ -o $@
	
gen2.exe: gen2.o
	$(CXX) $(CPPFLAGS) $^ -o $@

delta-gen.exe: delta-gen.o ${COMMON}
	$(CXX) $(CPPFLAGS) ${^:%.h=} -o $@

delta-gen2.exe: delta-gen2.o ${COMMON}
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


