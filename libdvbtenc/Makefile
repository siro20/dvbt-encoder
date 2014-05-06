################################################################################
# libdvbtenc
#
# Quick Makefile
################################################################################

# Fetch the library version
include make/version.mk

LIB_NAME := libdvbtenc

BIN_DIR := bin
SRC_DIR := src
INC_DIR := include
LIB_DIR := lib
BIN_DIR := bin

CPP := g++
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:.cpp=.o)

# Craft the various file names associated with the library
LIB_SO_NAME := $(LIB_NAME).so.$(LIB_VER_MAJ)
LIB_NAME_FULL := $(LIB_SO_NAME).$(LIB_VER_MIN).$(LIB_VER_PAT)

SHARED_LIB := $(LIB_DIR)/$(LIB_NAME_FULL)
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME).a

# Makes LIB_VERSION macro definition available to code
LIB_VER_FLAG := -DLIB_VERSION=\"v$(LIB_VER_MAJ).$(LIB_VER_MIN).$(LIB_VER_PAT)\"

CPPFLAGS := -Wall -Wextra -Wno-unused-parameter \
		  -fPIC \
		  -std=c++11 -D_GNU_SOURCE $(LIB_VER_FLAG) \
		  -I$(INC_DIR) \
		  -msse -msse2 -msse3 -O0 -ggdb3 

LDFLAGS := -fPIC -lfftw3f_omp

# Default install prefix
INSTALL_PREFIX ?= /usr

all: shared static

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build static library
static: $(KERNEL_OBJ) $(LIB_DIR) $(STATIC_LIB)

$(STATIC_LIB): $(OBJ)
	$(AR) rcs $@ $(OBJ)

# Build shared library and all the version symlinks
shared: $(KERNEL_OBJ) $(LIB_DIR) $(SHARED_LIB)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@
        
$(SHARED_LIB): $(OBJ)
	$(CPP) -shared -Wl,-soname,$(LIB_SO_NAME) $(OBJ) $(LDFLAGS) -o $@
	@cd $(LIB_DIR); \
		ln -fs $(LIB_NAME_FULL) $(LIB_SO_NAME).$(LIB_VER_MIN); \
		ln -fs $(LIB_SO_NAME).$(LIB_VER_MIN) $(LIB_SO_NAME); \
		ln -fs $(LIB_SO_NAME) $(LIB_NAME).so

libdvbtenc.pc: shared
	@echo "prefix=${INSTALL_PREFIX}" > $@
	@echo "exec_prefix=$$""{prefix}" >> $@
	@echo "libdir=$$""{exec_prefix}/lib" >> $@
	@echo "includedir=$$""{exec_prefix}/include" >> $@
	@echo -e "\nName: libdvbtenc" >> $@
	@echo "Description: C library for baseband DVBT encoding" >> $@
	@echo "URL: http://das-labor.org" >> $@
	@echo "Version: ${LIB_VER}" >> $@
	@echo "Libs: -L$$""{libdir} -ldvbtenc" >> $@
	@echo "Cflags: -I$$""{includedir}"  >> $@

install: shared libdvbtenc.pc
	cp -P lib/* $(INSTALL_PREFIX)/lib64
	cp include/* $(INSTALL_PREFIX)/include
	if [ -d $(INSTALL_PREFIX)/share/pkgconfig ]; then \
		cp libdvbtenc.pc $(INSTALL_PREFIX)/share/pkgconfig/ ; \
	elif [ -d $(INSTALL_PREFIX)/lib/pkgconfig ]; then \
		cp libdvbtenc.pc $(INSTALL_PREFIX)/lib/pkgconfig/ ; \
	else \
		@echo "Unsure where to install libdvbtenc.pc ... aborting." ; \
		exit 1; \
	fi

uninstall:
	rm -f $(INSTALL_PREFIX)/lib/libdvbtenc*
	rm -f $(INSTALL_PREFIX)/include/libdvbtenc.h
	rm -f $(INSTALL_PREFIX)/lib/pkgconfig/libdvbtenc.pc
	rm -f $(INSTALL_PREFIX)/share/pkgconfig/libdvbtenc.pc

	
clean:
	rm -rf $(LIB_DIR)
	rm -rf $(BIN_DIR)
	rm -f $(OBJ)
	rm -f libdvbtenc.pc

.PHONY: clean doc
