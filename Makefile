SHELL=/bin/bash

PLATFORM := $(shell uname)

ifeq ($(PLATFORM),Darwin)
	CPP=/usr/local/Cellar/gcc/5.2.0/bin/g++-5
	CC=/usr/local/Cellar/gcc/5.2.0/bin/gcc-5
else
	CPP=g++
	CC=gcc
endif

CFLAGS=-O3 -std=c99 -Wall
CXXFLAGS=-O3 -std=c++11 -Wall

.DEFAULT_GOAL := rv8

OBJS=v8-extended-util.o zsocket.o


v8-extended-util.o: v8-extended-util.cpp include/v8-extended-util.h
	$(CPP) $(CXXFLAGS) -c v8-extended-util.cpp -I .


zsocket.o: zsocket.cpp include/zsocket.h
	$(CPP) $(CXXFLAGS) -c zsocket.cpp -I .


rv8: rv8.cpp include/rv8.h ${OBJS}
ifeq ($(PLATFORM),Darwin)
		$(CPP) $(CXXFLAGS) rv8.cpp -o rv8 -I . -L/usr/local/Cellar/boost/1.58.0/lib -L/usr/local/Cellar/libsodium/1.0.3/lib lib/$(PLATFORM)/libzmq.a lib/$(PLATFORM)/{tools/gyp/libv8_{base,libbase,nosnapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -ldl -lboost_system -lboost_filesystem -lsodium -pthread ${OBJS}
else
		$(CPP) $(CXXFLAGS) rv8.cpp -o rv8 -I . lib/$(PLATFORM)/libzmq.a -Wl,--start-group lib/$(PLATFORM)/{tools/gyp/libv8_{base,libbase,nosnapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -lboost_system -lboost_filesystem -lsodium -pthread ${OBJS}
endif


all: rv8


.PHONY: clean

clean:
	rm -rf *.o
	rm -f rv8

