 # Makefile for mylib

lib.name = one

class.sources = one.cpp

# datafiles = help.pd README.txt LICENSE.txt

CXX = g++
CC = g++

cflags = -stdlib=libc++ -mmacosx-version-min=10.9

PDINCLUDEDIR = ./

include Makefile.pdlibbuilder