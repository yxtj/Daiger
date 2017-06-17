CXX ?= g++
CC ?= gcc
CMAKE = cmake
TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
CMAKE_FLAGS = 
#OPROFILE = 1

export CXX CC CFLAGS CPPFLAGS OPROFILE

.PHONY: clean

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
	
eclipse:
	#CMAKE_FLAGS = -G"Eclipse CDT4 - Unix Makefiles"
	@make debug CMAKE_FLAGS=-G"Eclipse CDT4 - Unix Makefiles"
	#$(MAKE) release CMAKE_FLAGS = -G"Eclipse CDT4 - Unix Makefiles"


clean:
	rm -rf bin/*
