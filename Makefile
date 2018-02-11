CXX ?= g++
CC ?= gcc
CMAKE = cmake
TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
CMAKE_FLAGS = 
#OPROFILE = 1

export CXX CC CFLAGS CPPFLAGS OPROFILE

.PHONY: clean external

all: release 

release: 
	@mkdir -p bin/release
	@cd bin/release && $(CMAKE) $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Release $(TOP)/src
	@cd bin/release && $(MAKE)
#	@cp bin/release/examples/maiter .

debug: 
	@mkdir -p bin/debug
	@cd bin/debug && $(CMAKE) $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Debug $(TOP)/src
	@cd bin/debug  && $(MAKE)
#	@cp bin/debug/examples/maiter .

external: gflags glog

gflags: bin/glags/lib/libgflags.a
	@mkdir -p bin/gflags
	@cd external && tar xf gflags-2.2.0.tzr.gz
	@cd bin/gflags && cmake ../external/gflags-2.2.0
	@cd bin/gflags && $(MAKE)

glog: bin/glob/lib/libglog.a
	@mkdir -p bin/glog
	@cd external && tar xf glog-0.3.5.tzr.gz
	@cd bin/gflags && cmake ../external/glog-0.3.5
	@cd bin/gflags && $(MAKE)

clean:
	rm -rf bin/*
