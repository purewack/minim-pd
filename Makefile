 # Makefile for mylib

lib.name = motif

class.sources = ticker.cpp bank.cpp plonk~.cpp inter.cpp

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

cflags = -std=c++17

define forDarwin
cflags += -stdlib=libc++ -mmacosx-version-min=10.9
endef
PDINCLUDEDIR = ./
PDBINDIR = /mnt/c/Program Files/Purr Data/bin/

ldflags += -pthread

include Makefile.pdlibbuilder
