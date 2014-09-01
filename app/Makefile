
WRKDIR = %cd%

CC = gcc
CPP = g++
LD = g++
AR = ar

INC = -I. -I/usr/include -I../libdvbtenc/include -I../libdvbtenc/src
CPPFLAGS = -Wall -march=native -Wextra -ggdb3 -ffast-math -march=native -pedantic -std=gnu++11 -O3
RESINC = 
RCFLAGS = 
LIB = -lfftw3f -lpthread
LDFLAGS = $(LIBDIR) $(LIB)

%: %.cpp
	$(CPP) $(CPPFLAGS) $(INC) $(LDFLAGS) -o $@ $< ../libdvbtenc/lib/libdvbtenc.a 

all: libdvbtenc $(basename $(wildcard *.cpp))

libdvbtenc:
	cd ../libdvbtenc/ && $(MAKE) all

clean: clean_release

clean_release:
	rm -rf $(basename $(wildcard *.cpp))

.PHONY: clean clean_release

