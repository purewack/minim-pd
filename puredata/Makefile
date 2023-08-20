 # Makefile for minim

lib.name = minim

class.sources = src/bank~.cpp src/ticker.cpp src/mapper.cpp src/plonk~.cpp src/oled.cpp 

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

cflags = -std=c++17

define forDarwin
cflags += -stdlib=libc++ -mmacosx-version-min=10.9
endef
PDINCLUDEDIR = ./src
PDBINDIR = /c/Program Files/Pd/bin/

cflags += -std=c++0x
ldflags += -pthread

include Makefile.pdlibbuilder
