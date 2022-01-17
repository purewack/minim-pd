 # Makefile for mylib

lib.name = motif

class.sources = ticker.cpp bank.cpp

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

define forDarwin
cflags = -stdlib=libc++ -mmacosx-version-min=10.9
endef
PDINCLUDEDIR = ./

include Makefile.pdlibbuilder