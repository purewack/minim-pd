 # Makefile for mylib

lib.name = one

class.sources = one.cpp

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

define forDarwin
cflags = -stdlib=libc++ -mmacosx-version-min=10.9
endef
PDINCLUDEDIR = ./

include Makefile.pdlibbuilder