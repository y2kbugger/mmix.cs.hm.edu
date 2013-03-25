#
#   Makefile for windos executables using MS Visual Studio
#

# you may want to adjust these variables

# where are the mmixware sources
MMIXDIR = ../../../mmixware/trunk

# where to install files
INSTALLDIR = ../../../../html

MMIXWAREEXE= mmix.exe mmmix.exe mmixal.exe mmotype.exe 

all: bin doc

bin:
	echo use visual studio to build the files
doc:
	echo no documents here

install: bin
	for file in $(MMIXWAREEXE); do \
	  cp Release/$$file $(INSTALLDIR)/exe ; \
	done

