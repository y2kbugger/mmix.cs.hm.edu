#
#   Makefile for windos executables using MS Visual Studio
#

# you may want to adjust these variables

# where are the mmixware sources
MMIXDIR = ../../../mmixware/trunk

# where to install files
INSTALLDIR = ../../../../html

MMIXWAREBIN= mmix mmmix mmixal mmotype 

all: bin doc

bin: mmoimg
doc:
	echo no documents here

install: bin
	for file in $(MMIXWAREBIN); do \
	  cp $$file.exe $(INSTALLDIR)/exe ; \
	done

# to make the directory ready for visual studio
ready:
	for file in ctangle cweave ctie; do \
	  cp $(INSTALLDIR)/exe/$$file.exe . ; \
	done
	cp $(MMIXDIR)/*.w .