
WRKDIR = %cd%

CC = gcc
CPP = g++
LD = g++
AR = ar

INC = -I. -I/usr/include -I../libdvbtenc/include -I../libdvbtenc/src
CPPFLAGS = -Wall -msse -msse2 -msse3 -Wextra -pedantic -std=gnu++11
RESINC = 
RCFLAGS = 
LIBDIR = -L../libdvbtenc/lib
LIB = -ldvbtenc -lfftw3f
LDFLAGS = $(LIBDIR) $(LIB)

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CPPFLAGS) -O0 -ggdb3 $(LDFLAGS)
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) #-s
OBJDIR_RELEASE = obj

%: %.cpp
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -o $@ $< 

all: libdvbtenc tools $(basename $(wildcard *.cpp))

libdvbtenc:
	cd ../libdvbtenc/ && $(MAKE) all

tools:
	$(CC) -o ./float_cmp ../tools/float_cmp.c

clean: clean_release

clean_release:
	rm -rf $(basename $(wildcard *.cpp))

.PHONY: clean clean_release

