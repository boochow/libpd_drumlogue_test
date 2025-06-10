##############################################################################
# Project Configuration
#

PROJECT := libpd_test
PROJECT_TYPE := synth

.DEFAULT_GOAL := all

# libpd
LIBPD := libpd-arm
LIBPD_LIB := $(LIBPD)/libs
LIBPD_INC := $(LIBPD)/inc

$(LIBPD): $(LIBPD_LIB) $(LIBPD_INC)


$(LIBPD_LIB): 
	mkdir $(LIBPD) ; \
	cd $(LIBPD) ; \
	CC=../../tools/drumlogue-toolchain-amd64/arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-gcc cmake -DLIBPD_SHARED=OFF -DLIBPD_STATIC=ON -DPD_UTILS=OFF -DCMAKE_BUILD_TYPE=Release ../libpd ; \
	make

$(LIBPD_INC):
	mkdir -p $(LIBPD)/inc
	cp libpd/libpd_wrapper/z_libpd.h libpd/pure-data/src/m_pd.h $(LIBPD)/inc/

##############################################################################
# Sources
#

# C sources
CSRC = header.c

# C++ sources
CXXSRC = unit.cc

# List ASM source files here
ASMSRC = 

ASMXSRC = 

##############################################################################
# Include Paths
#

UINCDIR  = $(LIBPD_INC)

##############################################################################
# Library Paths
#

ULIBDIR = $(LIBPD_LIB)

##############################################################################
# Libraries
#

ULIBS  = -lm
ULIBS += -lc
ULIBS += -lpd

##############################################################################
# Macros
#

UDEFS = 

