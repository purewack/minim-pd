 # Makefile for mylib

lib.name = motif

class.sources = bank.cpp ticker.cpp mapper.cpp plonk~.cpp 

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

define forDarwin
cflags = -stdlib=libc++ -mmacosx-version-min=10.9
endef
PDINCLUDEDIR = ./
PDBINDIR = /c/Program Files/Pd/bin/

cflags += -std=c++0x
ldflags += -pthread

include Makefile.pdlibbuilder